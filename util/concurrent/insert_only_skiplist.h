#ifndef UTIL_CONCURRENT_INSERT_ONLY_SKIPLIST_H
#define UTIL_CONCURRENT_INSERT_ONLY_SKIPLIST_H

// Implementation of a concurrent skiplist following the algorithm described
// in the book: "The Art of Multiprocessor Programming, Revised Print"
// http://www.amazon.com/books/dp/0123973376
// and in the algorithm described in: "Practical lock-freedom"
// http://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-579.pdf

#include <stdlib.h>
#include <new>
#include "util/minus.h"
#include "util/concurrent/atomic/pointer.h"
#include "util/concurrent/atomic/atomic.h"

// Thread Local Storage.
static __thread struct drand48_data random_data = {{0xc0}};

namespace util {
	namespace concurrent {
		template<typename _Key, typename _Compare = util::minus<_Key> >
		class insert_only_skiplist {
			public:
				// Constructor.
				insert_only_skiplist();
				insert_only_skiplist(const _Compare& cmp);

				// Destructor.
				~insert_only_skiplist();

				// Initialize.
				bool init();

				// Insert.
				bool insert(const _Key& k);

				// Contains.
				bool contains(const _Key& k) const;

				// Find key.
				bool find(const _Key& k, _Key& fullkey) const;

				class iterator {
					friend class insert_only_skiplist;

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

					atomic::pointer<node> next[1];

					// Constructor.
					node(int height);
					node(const _Key& k, int height);
				};

				node* _M_header;

				int _M_level_hint;

				_Compare _M_compare;

				// Find.
				bool find(const _Key& k, node** preds, node** succs);
				bool find(const _Key& k, const node** preds, const node** succs) const;
				const node* find(const _Key& k) const;

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
		inline insert_only_skiplist<_Key, _Compare>::insert_only_skiplist()
		: _M_header(NULL),
		_M_level_hint(1),
		_M_compare()
		{
		}

		template<typename _Key, typename _Compare>
		inline insert_only_skiplist<_Key, _Compare>::insert_only_skiplist(const _Compare& cmp)
		: _M_header(NULL),
		_M_level_hint(1),
		_M_compare(cmp)
		{
		}

		template<typename _Key, typename _Compare>
		insert_only_skiplist<_Key, _Compare>::~insert_only_skiplist()
		{
			node* n = _M_header;
			while (n) {
				node* next = n->next[0].get();
				delete_node(n);
				n = next;
			}
		}

		template<typename _Key, typename _Compare>
		bool insert_only_skiplist<_Key, _Compare>::init()
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
		bool insert_only_skiplist<_Key, _Compare>::insert(const _Key& k)
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
				if (preds[0]->next[0].compare_and_swap(succs[0], new_node)) {
					break;
				}
			} while (true);

			for (int i = 1; i < level; i++) {
				do {
					if (preds[i]->next[i].compare_and_swap(succs[i], new_node)) {
						break;
					}

					find(k, preds, succs);

					// Set successor pointers.
					for (int j = i; j < level; j++) {
						new_node->next[j] = succs[j];
					}
				} while (true);
			}

			return true;
		}

		template<typename _Key, typename _Compare>
		inline bool insert_only_skiplist<_Key, _Compare>::contains(const _Key& k) const
		{
			return (find(k) != NULL);
		}

		template<typename _Key, typename _Compare>
		inline bool insert_only_skiplist<_Key, _Compare>::find(const _Key& k, _Key& fullkey) const
		{
			const node* x;
			if ((x = find(k)) == NULL) {
				return false;
			}

			fullkey = x->key;

			return true;
		}

		template<typename _Key, typename _Compare>
		inline const _Key& insert_only_skiplist<_Key, _Compare>::iterator::key() const
		{
			return k;
		}

		template<typename _Key, typename _Compare>
		inline bool insert_only_skiplist<_Key, _Compare>::begin(iterator& it) const
		{
			const node* curr;
			if ((curr = _M_header->next[0].get()) == NULL) {
				return false;
			}

			it.k = curr->key;

			return true;
		}

		template<typename _Key, typename _Compare>
		bool insert_only_skiplist<_Key, _Compare>::end(iterator& it) const
		{
			const node* pred = _M_header;
			for (int level = _M_level_hint - 1; level >= 0; level--) {
				const node* curr = pred->next[level].get();
				while (curr) {
					const node* succ = curr->next[level].get();
					pred = curr;
					curr = succ;
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
		inline bool insert_only_skiplist<_Key, _Compare>::previous(iterator& it) const
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
		bool insert_only_skiplist<_Key, _Compare>::next(iterator& it) const
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
			}

			it.k = curr->key;

			return true;
		}

		template<typename _Key, typename _Compare>
		bool insert_only_skiplist<_Key, _Compare>::seek(const _Key& k, iterator& it) const
		{
			const node* preds[kMaxLevel];
			const node* succs[kMaxLevel];

			find(k, preds, succs);

			const node* succ;
			if ((succ = succs[0]) == NULL) {
				return false;
			}

			it.k = succ->key;

			return true;
		}

		template<typename _Key, typename _Compare>
		inline insert_only_skiplist<_Key, _Compare>::node::node(int height)
		: key(),
		level(height)
		{
		}

		template<typename _Key, typename _Compare>
		inline insert_only_skiplist<_Key, _Compare>::node::node(const _Key& k, int height)
		: key(k),
		level(height)
		{
		}

		template<typename _Key, typename _Compare>
		bool insert_only_skiplist<_Key, _Compare>::find(const _Key& k, node** preds, node** succs)
		{
			int ret = -1;

			node* pred = _M_header;
			for (int level = _M_level_hint - 1; level >= 0; level--) {
				node* curr = pred->next[level].get();
				while (curr) {
					if ((ret = _M_compare(curr->key, k)) < 0) {
						pred = curr;
						curr = curr->next[level].get();
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
		bool insert_only_skiplist<_Key, _Compare>::find(const _Key& k, const node** preds, const node** succs) const
		{
			int ret = -1;

			const node* pred = _M_header;
			for (int level = _M_level_hint - 1; level >= 0; level--) {
				const node* curr = pred->next[level].get();
				while (curr) {
					if ((ret = _M_compare(curr->key, k)) < 0) {
						pred = curr;
						curr = curr->next[level].get();
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
		const struct insert_only_skiplist<_Key, _Compare>::node* insert_only_skiplist<_Key, _Compare>::find(const _Key& k) const
		{
			const node* pred = _M_header;
			for (int level = _M_level_hint - 1; level >= 0; level--) {
				const node* curr = pred->next[level].get();
				while (curr) {
					int ret;
					if ((ret = _M_compare(curr->key, k)) < 0) {
						pred = curr;
						curr = curr->next[level].get();
					} else if (ret == 0) {
						return curr;
					} else {
						break;
					}
				}
			}

			return NULL;
		}

		template<typename _Key, typename _Compare>
		inline struct insert_only_skiplist<_Key, _Compare>::node* insert_only_skiplist<_Key, _Compare>::make_node(int height)
		{
			node* n;
			if ((n = allocate_node(height)) == NULL) {
				return NULL;
			}

			return new (n) node(height);
		}

		template<typename _Key, typename _Compare>
		inline struct insert_only_skiplist<_Key, _Compare>::node* insert_only_skiplist<_Key, _Compare>::make_node(const _Key& k, int height)
		{
			node* n;
			if ((n = allocate_node(height)) == NULL) {
				return NULL;
			}

			return new (n) node(k, height);
		}

		template<typename _Key, typename _Compare>
		inline struct insert_only_skiplist<_Key, _Compare>::node* insert_only_skiplist<_Key, _Compare>::allocate_node(int height)
		{
			return reinterpret_cast<node*>(malloc(sizeof(node) + ((height - 1) * sizeof(atomic::pointer<node>))));
		}

		template<typename _Key, typename _Compare>
		inline void insert_only_skiplist<_Key, _Compare>::delete_node(node* n)
		{
			// Call the destructor.
			n->~node();

			// Free the memory.
			free(n);
		}

		template<typename _Key, typename _Compare>
		int insert_only_skiplist<_Key, _Compare>::random_level()
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
			} while (!atomic::bool_compare_and_swap(&_M_level_hint, level_hint, level));

			return level;
		}
	}
}

#endif // UTIL_CONCURRENT_INSERT_ONLY_SKIPLIST_H
