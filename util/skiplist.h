#ifndef UTIL_SKIPLIST_H
#define UTIL_SKIPLIST_H

// Implementation of the skiplist invented by William Pugh:
// ftp://ftp.cs.umd.edu/pub/skipLists/skiplists.pdf
// ftp://ftp.cs.umd.edu/pub/skipLists/skipLists.c
//
// Limitations:
// This implementation is not thread-safe.

#include <stdlib.h>
#include <new>
#include "util/difference.h"

namespace util {
	template<typename _Key, typename _Compare = util::difference<_Key> >
	class skiplist {
		private:
			struct node {
				// Key.
				const _Key key;

				// Level.
				int level;

				// Previous node.
				node* prev;

				// Forward nodes.
				node* forward[1];

				// Constructor.
				node(int height);
				node(const _Key& k, int height);
			};

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
			bool erase(const _Key& k, _Key& fullkey);

			// Contains.
			bool contains(const _Key& k) const;

			// Find key.
			bool find(const _Key& k, _Key& fullkey) const;

			// Clear.
			void clear();

			class iterator {
				friend class skiplist;

				public:
					const _Key& key() const;

				private:
					const node* n;
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

			static const int kBitsInRandom = 31;

			node* _M_header;
			node* _M_tail;

			int _M_level;

			int _M_random;
			int _M_randoms_left;

			_Compare _M_compare;

			// Find.
			const node* find(const _Key& k) const;

			// Make node.
			node* make_node(int height);
			node* make_node(const _Key& k, int height);

			// Allocate node.
			node* allocate_node(int height);

			// Unlink.
			node* unlink(const _Key& k);

			// Delete node.
			void delete_node(node* n);

			// Random level.
			int random_level();
	};

	template<typename _Key, typename _Compare>
	inline skiplist<_Key, _Compare>::skiplist()
	: _M_header(NULL),
	_M_level(1),
	_M_random(random()),
	_M_randoms_left(kBitsInRandom / 2),
	_M_compare()
	{
	}

	template<typename _Key, typename _Compare>
	inline skiplist<_Key, _Compare>::skiplist(const _Compare& cmp)
	: _M_header(NULL),
	_M_level(1),
	_M_random(random()),
	_M_randoms_left(kBitsInRandom / 2),
	_M_compare(cmp)
	{
	}

	template<typename _Key, typename _Compare>
	skiplist<_Key, _Compare>::~skiplist()
	{
		node* n = _M_header;
		while (n) {
			node* next = n->forward[0];
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
			_M_header->forward[i] = NULL;
		}

		_M_tail = _M_header;

		return true;
	}

	template<typename _Key, typename _Compare>
	bool skiplist<_Key, _Compare>::insert(const _Key& k)
	{
		int ret = -1;

		node* update[kMaxLevel];
		node* x = _M_header;
		for (int i = _M_level - 1; i >= 0; i--) {
			node* next;
			while (((next = x->forward[i]) != NULL) && ((ret = _M_compare(next->key, k)) < 0)) {
				x = next;
			}

			update[i] = x;
		}

		if (ret == 0) {
			// Already inserted.
			return false;
		}

		int level = random_level();

		// Create node.
		if ((x = make_node(k, level)) == NULL) {
			return false;
		}

		if (level > _M_level) {
			for (int i = _M_level; i < level; i++) {
				update[i] = _M_header;
			}

			_M_level = level;
		}

		for (int i = 0; i < level; i++) {
			x->forward[i] = update[i]->forward[i];
			update[i]->forward[i] = x;
		}

		// Set previous pointer.
		x->prev = update[0];

		if (x->forward[0]) {
			x->forward[0]->prev = x;
		} else {
			_M_tail = x;
		}

		return true;
	}

	template<typename _Key, typename _Compare>
	bool skiplist<_Key, _Compare>::erase(const _Key& k)
	{
		node* x;
		if ((x = unlink(k)) == NULL) {
			return false;
		}

		delete_node(x);

		return true;
	}

	template<typename _Key, typename _Compare>
	bool skiplist<_Key, _Compare>::erase(const _Key& k, _Key& fullkey)
	{
		node* x;
		if ((x = unlink(k)) == NULL) {
			return false;
		}

		fullkey = x->key;

		delete_node(x);

		return true;
	}

	template<typename _Key, typename _Compare>
	inline bool skiplist<_Key, _Compare>::contains(const _Key& k) const
	{
		return (find(k) != NULL);
	}

	template<typename _Key, typename _Compare>
	inline bool skiplist<_Key, _Compare>::find(const _Key& k, _Key& fullkey) const
	{
		const node* x;
		if ((x = find(k)) == NULL) {
			return false;
		}

		fullkey = x->key;

		return true;
	}

	template<typename _Key, typename _Compare>
	void skiplist<_Key, _Compare>::clear()
	{
		node* n = _M_header->forward[0];
		while (n) {
			node* next = n->forward[0];
			delete_node(n);
			n = next;
		}

		// Initialize forward pointers to NULL.
		for (int i = 0; i < kMaxLevel; i++) {
			_M_header->forward[i] = NULL;
		}

		_M_tail = _M_header;
	}

	template<typename _Key, typename _Compare>
	inline const _Key& skiplist<_Key, _Compare>::iterator::key() const
	{
		return n->key;
	}

	template<typename _Key, typename _Compare>
	inline bool skiplist<_Key, _Compare>::begin(iterator& it) const
	{
		// If the list is empty...
		if (!_M_header->forward[0]) {
			return false;
		}

		it.n = _M_header->forward[0];

		return true;
	}

	template<typename _Key, typename _Compare>
	inline bool skiplist<_Key, _Compare>::end(iterator& it) const
	{
		// If the list is empty...
		if (!_M_header->forward[0]) {
			return false;
		}

		it.n = _M_tail;

		return true;
	}

	template<typename _Key, typename _Compare>
	inline bool skiplist<_Key, _Compare>::previous(iterator& it) const
	{
		if (it.n->prev == _M_header) {
			return false;
		}

		it.n = it.n->prev;

		return true;
	}

	template<typename _Key, typename _Compare>
	inline bool skiplist<_Key, _Compare>::next(iterator& it) const
	{
		if (!it.n->forward[0]) {
			return false;
		}

		it.n = it.n->forward[0];

		return true;
	}

	template<typename _Key, typename _Compare>
	inline bool skiplist<_Key, _Compare>::seek(const _Key& k, iterator& it) const
	{
		return ((it.n = find(k)) != NULL);
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
	const struct skiplist<_Key, _Compare>::node* skiplist<_Key, _Compare>::find(const _Key& k) const
	{
		const node* x = _M_header;
		for (int i = _M_level - 1; i >= 0; i--) {
			const node* next;
			while ((next = x->forward[i]) != NULL) {
				int ret;
				if ((ret = _M_compare(next->key, k)) < 0) {
					x = next;
				} else if (ret == 0) {
					return next;
				} else {
					break;
				}
			}
		}

		return NULL;
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
		return reinterpret_cast<node*>(malloc(sizeof(node) + ((height - 1) * sizeof(node*))));
	}

	template<typename _Key, typename _Compare>
	struct skiplist<_Key, _Compare>::node* skiplist<_Key, _Compare>::unlink(const _Key& k)
	{
		int ret = -1;

		node* update[kMaxLevel];
		node* x = _M_header;
		for (int i = _M_level - 1; i >= 0; i--) {
			node* next;
			while (((next = x->forward[i]) != NULL) && ((ret = _M_compare(next->key, k)) < 0)) {
				x = next;
			}

			update[i] = x;
		}

		if (ret != 0) {
			// Not found.
			return NULL;
		}

		x = x->forward[0];

		for (int i = 0; i < _M_level; i++) {
			if (update[i]->forward[i] != x) {
				break;
			}

			update[i]->forward[i] = x->forward[i];
		}

		if (update[0]->forward[0]) {
			update[0]->forward[0]->prev = update[0];
		} else {
			_M_tail = update[0];
		}

		while ((_M_level > 1) && (!_M_header->forward[_M_level - 1])) {
			_M_level--;
		}

		return x;
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
		int level = 1;
		int b;

		do {
			if ((b = _M_random & 3) == 0) {
				level++;
			}

			_M_random >>= 2;

			if (--_M_randoms_left == 0) {
				_M_random = random();
				_M_randoms_left = kBitsInRandom / 2;
			}
		} while (b == 0);

		return (level > kMaxLevel) ? kMaxLevel : level;
	}
}

#endif // UTIL_SKIPLIST_H
