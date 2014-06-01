#ifndef UTIL_CONCURRENT_SKIPLIST_H
#define UTIL_CONCURRENT_SKIPLIST_H

// Implementation of a concurrent skiplist following the algorithm described
// in the book: "The Art of Multiprocessor Programming, Revised Print"
// http://www.amazon.com/books/dp/0123973376
// and in the algorithm described in: "Practical lock-freedom"
// http://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-579.pdf
//
// Notes:
// As nodes cannot be freed directly, they are saved in a list until the
// list reaches 'kThreshold' nodes. At this point the oldest node will be
// deleted.

#include <stdlib.h>
#include <new>
#include "util/minus.h"
#include "util/concurrent/atomic/markable_ptr.h"
#include "util/concurrent/atomic/atomic.h"
#include "util/concurrent/locks/spinlock.h" // For the list of deleted nodes.

// Thread Local Storage.
static __thread struct drand48_data random_data = {{0xc0}};

namespace util {
	namespace concurrent {
		template<typename _Key, typename _Compare = util::minus<_Key> >
		class skiplist {
			public:
				// Constructor.
				skiplist();
				skiplist(const _Compare& cmp);

				// Destructor.
				~skiplist();

				// Initialize.
				bool init();

				// Insert.
				bool insert(const _Key& k);

				// Erase.
				bool erase(const _Key& k);

				// Contains.
				bool contains(const _Key& k) const;

				class iterator {
					friend class skiplist;

					public:
						const _Key& key() const;

					private:
						_Key k;
				};

				// Begin.
				bool begin(iterator& it) const;

				// End.
				bool end(iterator& it) const;

				// Previous.
				bool previous(iterator& it) const;

				// Next.
				bool next(iterator& it) const;

				// Seek.
				bool seek(const _Key& k, iterator& it) const;

			private:
				// [Probability] p = 0.25
				// [Maximum number of elements] N = 4294967296
				// L(n) = log1/p(n)
				// kMaxLevel = L(N) = log4(4294967296) = 16
				static const int kMaxLevel = 16;

				struct node {
					// Key.
					const _Key key;

					// Level.
					int level;

					// Next deleted node.
					node* next_deleted;

					// Previous deleted node.
					node* prev_deleted;

					concurrent::atomic::markable_ptr<node> next[1];

					// Constructor.
					node(int height);
					node(const _Key& k, int height);
				};

				node* _M_header;

				int _M_level_hint;

				_Compare _M_compare;

				struct nodelist {
					static const int kThreshold = 32 * 1024;

					node* head;
					node* tail;

					int count;

					concurrent::locks::spinlock lock;

					// Insert.
					void insert(node* n);
				};

				nodelist _M_free_nodes;

				// Find.
				bool find(const _Key& k, node** preds, node** succs);
				bool find(const _Key& k, const node** preds, const node** succs) const;

				// Make node.
				node* make_node(int height);
				node* make_node(const _Key& k, int height);

				// Allocate node.
				node* allocate_node(int height);

				// Delete node.
				void delete_node(node* n);

				// Random level.
				int random_level();
		};

		template<typename _Key, typename _Compare>
		inline skiplist<_Key, _Compare>::skiplist()
		: _M_header(NULL),
		_M_level_hint(1),
		_M_compare()
		{
			_M_free_nodes.head = NULL;
			_M_free_nodes.tail = NULL;
			_M_free_nodes.count = 0;
		}

		template<typename _Key, typename _Compare>
		inline skiplist<_Key, _Compare>::skiplist(const _Compare& cmp)
		: _M_header(NULL),
		_M_level_hint(1),
		_M_compare(cmp)
		{
			_M_free_nodes.head = NULL;
			_M_free_nodes.tail = NULL;
			_M_free_nodes.count = 0;
		}

		template<typename _Key, typename _Compare>
		skiplist<_Key, _Compare>::~skiplist()
		{
			node* n = _M_header;
			while (n) {
				node* next = n->next[0].get();
				delete_node(n);
				n = next;
			}

			n = _M_free_nodes.head;
			while (n) {
				node* next = n->next_deleted;
				delete_node(n);
				n = next;
			}
		}

		template<typename _Key, typename _Compare>
		bool skiplist<_Key, _Compare>::init()
		{
			// Create header.
			if ((_M_header = make_node(kMaxLevel)) == NULL) {
				return false;
			}

			// Initialize forward pointers to NULL.
			for (int i = 0; i < kMaxLevel; i++) {
				_M_header->next[i] = NULL;
			}

			return true;
		}

		template<typename _Key, typename _Compare>
		bool skiplist<_Key, _Compare>::insert(const _Key& k)
		{
			int level = random_level();

			node* preds[kMaxLevel];
			node* succs[kMaxLevel];

			node* new_node = NULL;

			do {
				if (find(k, preds, succs)) {
					// Already inserted.
					if (new_node) {
						delete_node(new_node);
					}

					return false;
				}

				// Create node.
				if ((!new_node) && ((new_node = make_node(k, level)) == NULL)) {
					return false;
				}

				// Set successor pointers.
				for (int i = 0; i < level; i++) {
					new_node->next[i] = succs[i];
				}

				// Node is considered inserted when it is linked at level 0.
				if (preds[0]->next[0].compare_and_swap(succs[0], new_node, false, false)) {
					break;
				}
			} while (true);

			for (int i = 1; i < level; i++) {
				do {
					// If 'new_node' has been marked as deleted...
					concurrent::atomic::markable_ptr<node> next = new_node->next[i];
					if (next.marked()) {
						find(k, preds, succs);
						return true;
					}

					if (next.get() != succs[i]) {
						bool oldmark;
						if ((!new_node->next[i].compare_and_swap(next.get(), succs[i], false, false, oldmark)) && (oldmark)) {
							// 'new_node' has been marked as deleted...
							find(k, preds, succs);
							return true;
						}
					}

					if (preds[i]->next[i].compare_and_swap(succs[i], new_node, false, false)) {
						break;
					}

					find(k, preds, succs);
				} while (true);
			}

			// If 'new_node' has been marked as deleted...
			if (new_node->next[level - 1].marked()) {
				find(k, preds, succs);
			}

			return true;
		}

		template<typename _Key, typename _Compare>
		bool skiplist<_Key, _Compare>::erase(const _Key& k)
		{
			node* preds[kMaxLevel];
			node* succs[kMaxLevel];
			if (!find(k, preds, succs)) {
				// Not found.
				return false;
			}

			node* node_to_delete = succs[0];

			for (int level = node_to_delete->level - 1; level >= 0; level--) {
				do {
					bool oldmark;
					node* succ = node_to_delete->next[level].get();

					// Try to mark the node as deleted.
					if (!node_to_delete->next[level].compare_and_swap(succ, succ, false, true, oldmark)) {
						// If the node has been already marked as deleted...
						if (oldmark) {
							if (level == 0) {
								// Another thread has just deleted the same node.
								return false;
							}

							// Some other thread has marked the node as deleted.
							break;
						}
					} else {
						// Node has been marked as deleted.
						break;
					}
				} while (true);
			}

			find(k, preds, succs);

			// Add node to the list of free nodes.
			_M_free_nodes.insert(node_to_delete);

			return true;
		}

		template<typename _Key, typename _Compare>
		bool skiplist<_Key, _Compare>::contains(const _Key& k) const
		{
			const node* pred = _M_header;
			for (int level = _M_level_hint - 1; level >= 0; level--) {
				bool marked;
				const node* curr = pred->next[level].get(marked);

				// If 'pred' has just been marked as deleted...
				if (marked) {
					return contains(k);
				}

				while (curr) {
					// While 'curr' has been marked as deleted...
					const node* succ = curr->next[level].get(marked);
					while (marked) {
						if ((curr = succ) == NULL) {
							break;
						}

						succ = curr->next[level].get(marked);
					}

					if (curr) {
						int ret;
						if ((ret = _M_compare(curr->key, k)) < 0) {
							pred = curr;
							curr = succ;
						} else if (ret == 0) {
							return true;
						} else {
							break;
						}
					} else {
						break;
					}
				}
			}

			return false;
		}

		template<typename _Key, typename _Compare>
		inline const _Key& skiplist<_Key, _Compare>::iterator::key() const
		{
			return k;
		}

		template<typename _Key, typename _Compare>
		bool skiplist<_Key, _Compare>::begin(iterator& it) const
		{
			const node* pred = _M_header;
			const node* curr;
			if ((curr = pred->next[0].get()) == NULL) {
				return false;
			}

			// While 'curr' has been marked as deleted...
			bool marked;
			const node* succ = curr->next[0].get(marked);
			while (marked) {
				if ((curr = succ) == NULL) {
					return false;
				}

				succ = curr->next[0].get(marked);
			}

			it.k = curr->key;

			return true;
		}

		template<typename _Key, typename _Compare>
		bool skiplist<_Key, _Compare>::end(iterator& it) const
		{
			const node* pred = _M_header;
			for (int level = _M_level_hint - 1; level >= 0; level--) {
				bool marked;
				const node* curr = pred->next[level].get(marked);

				// If 'pred' has just been marked as deleted...
				if (marked) {
					return end(it);
				}

				while (curr) {
					// While 'curr' has been marked as deleted...
					const node* succ = curr->next[level].get(marked);
					while (marked) {
						if ((curr = succ) == NULL) {
							break;
						}

						succ = curr->next[level].get(marked);
					}

					if (curr) {
						pred = curr;
						curr = succ;
					} else {
						break;
					}
				}
			}

			// If the list is empty...
			if (pred == _M_header) {
				return false;
			}

			it.k = pred->key;

			return true;
		}

		template<typename _Key, typename _Compare>
		inline bool skiplist<_Key, _Compare>::previous(iterator& it) const
		{
			const node* preds[kMaxLevel];
			const node* succs[kMaxLevel];

			find(it.k, preds, succs);

			if (preds[0] == _M_header) {
				return false;
			}

			it.k = preds[0]->key;

			return true;
		}

		template<typename _Key, typename _Compare>
		bool skiplist<_Key, _Compare>::next(iterator& it) const
		{
			const node* preds[kMaxLevel];
			const node* succs[kMaxLevel];
			const node* curr;

			if (!find(it.k, preds, succs)) {
				if ((curr = succs[0]) == NULL) {
					return false;
				}
			} else {
				if (((curr = succs[0]) == NULL) || ((curr = curr->next[0].get()) == NULL)) {
					return false;
				}

				// While 'curr' has been marked as deleted...
				bool marked;
				const node* succ = curr->next[0].get(marked);
				while (marked) {
					if ((curr = succ) == NULL) {
						return false;
					}

					succ = curr->next[0].get(marked);
				}
			}

			it.k = curr->key;

			return true;
		}

		template<typename _Key, typename _Compare>
		bool skiplist<_Key, _Compare>::seek(const _Key& k, iterator& it) const
		{
			const node* preds[kMaxLevel];
			const node* succs[kMaxLevel];
			const node* succ;

			find(k, preds, succs);

			if ((succ = succs[0]) == NULL) {
				return false;
			}

			it.k = succ->key;

			return true;
		}

		template<typename _Key, typename _Compare>
		inline skiplist<_Key, _Compare>::node::node(int height)
		: key(),
		level(height)
		{
		}

		template<typename _Key, typename _Compare>
		inline skiplist<_Key, _Compare>::node::node(const _Key& k, int height)
		: key(k),
		level(height)
		{
		}

		template<typename _Key, typename _Compare>
		void skiplist<_Key, _Compare>::nodelist::insert(node* n)
		{
			lock.lock();

			n->prev_deleted = NULL;
			n->next_deleted = head;

			if (!tail) {
				tail = n;
				count++;
			} else {
				head->prev_deleted = n;

				if (count == kThreshold) {
					node* last = tail;
					tail = tail->prev_deleted;
					tail->next_deleted = NULL;

					// Call the destructor.
					last->~node();

					// Free the memory.
					free(last);
				} else {
					count++;
				}
			}

			head = n;

			lock.unlock();
		}

		template<typename _Key, typename _Compare>
		bool skiplist<_Key, _Compare>::find(const _Key& k, node** preds, node** succs)
		{
			int ret = -1;

			node* pred = _M_header;
			for (int level = _M_level_hint - 1; level >= 0; level--) {
				bool marked;
				node* curr = pred->next[level].get(marked);

				// If 'pred' has just been marked as deleted...
				if (marked) {
					return find(k, preds, succs);
				}

				while (curr) {
					// While 'curr' has been marked as deleted...
					node* succ = curr->next[level].get(marked);
					while (marked) {
						if (!pred->next[level].compare_and_swap(curr, succ, false, false)) {
							// Retry.
							return find(k, preds, succs);
						}

						if ((curr = succ) == NULL) {
							break;
						}

						succ = curr->next[level].get(marked);
					}

					if ((curr) && ((ret = _M_compare(curr->key, k)) < 0)) {
						pred = curr;
						curr = succ;
					} else {
						break;
					}
				}

				preds[level] = pred;
				succs[level] = curr;
			}

			return (ret == 0);
		}

		template<typename _Key, typename _Compare>
		bool skiplist<_Key, _Compare>::find(const _Key& k, const node** preds, const node** succs) const
		{
			int ret = -1;

			const node* pred = _M_header;
			for (int level = _M_level_hint - 1; level >= 0; level--) {
				bool marked;
				const node* curr = pred->next[level].get(marked);

				// If 'pred' has just been marked as deleted...
				if (marked) {
					return find(k, preds, succs);
				}

				while (curr) {
					// While 'curr' has been marked as deleted...
					const node* succ = curr->next[level].get(marked);
					while (marked) {
						if ((curr = succ) == NULL) {
							break;
						}

						succ = curr->next[level].get(marked);
					}

					if ((curr) && ((ret = _M_compare(curr->key, k)) < 0)) {
						pred = curr;
						curr = succ;
					} else {
						break;
					}
				}

				preds[level] = pred;
				succs[level] = curr;
			}

			return (ret == 0);
		}

		template<typename _Key, typename _Compare>
		inline struct skiplist<_Key, _Compare>::node* skiplist<_Key, _Compare>::make_node(int height)
		{
			node* n;
			if ((n = allocate_node(height)) == NULL) {
				return NULL;
			}

			return new (n) node(height);
		}

		template<typename _Key, typename _Compare>
		inline struct skiplist<_Key, _Compare>::node* skiplist<_Key, _Compare>::make_node(const _Key& k, int height)
		{
			node* n;
			if ((n = allocate_node(height)) == NULL) {
				return NULL;
			}

			return new (n) node(k, height);
		}

		template<typename _Key, typename _Compare>
		inline struct skiplist<_Key, _Compare>::node* skiplist<_Key, _Compare>::allocate_node(int height)
		{
			return reinterpret_cast<node*>(malloc(sizeof(node) + ((height - 1) * sizeof(concurrent::atomic::markable_ptr<node>))));
		}

		template<typename _Key, typename _Compare>
		inline void skiplist<_Key, _Compare>::delete_node(node* n)
		{
			// Call the destructor.
			n->~node();

			// Free the memory.
			free(n);
		}

		template<typename _Key, typename _Compare>
		int skiplist<_Key, _Compare>::random_level()
		{
			static const unsigned FRACTION_P = 4;

			int level = 1;
			long rand;

			do {
				lrand48_r(&random_data, &rand);
				if ((rand % FRACTION_P) == 0) {
					level++;
				} else {
					break;
				}
			} while (level < kMaxLevel);

			// Increment level hint if required.
			int level_hint;
			do {
				if ((level_hint = _M_level_hint) >= level) {
					break;
				}
			} while (!concurrent::atomic::bool_compare_and_swap(&_M_level_hint, level_hint, level));

			return level;
		}
	}
}

#endif // UTIL_CONCURRENT_SKIPLIST_H
