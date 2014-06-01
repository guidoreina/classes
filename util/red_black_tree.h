#ifndef UTIL_RED_BLACK_TREE_H
#define UTIL_RED_BLACK_TREE_H

// Implementation of the Red-Black-Tree algorithm of the book:
// "Introduction to Algorithms", by Cormen, Leiserson, Rivest and Stein.

#include <stdlib.h>
#include <stdint.h>
#include <memory>
#include "util/minus.h"

namespace util {
  template<typename _Key,
           typename _Value,
           typename _Compare = util::minus<_Key> >
  class red_black_tree {
    private:
      typedef uint8_t color_t;

      struct node {
        _Key key;
        _Value value;

        color_t color;

        node* left;
        node* right;
        node* p;

        // Constructor.
        node();
        node(const _Key& k, const _Value& v);
      };

    public:
      class iterator {
        friend class red_black_tree;

        public:
          _Key* first;
          _Value* second;

        private:
          node* _M_node;
      };

      class const_iterator {
        friend class red_black_tree;

        public:
          const _Key* first;
          const _Value* second;

        private:
          const node* _M_node;
      };

      // Constructor.
      red_black_tree();
      red_black_tree(const _Compare& comp);
      red_black_tree(red_black_tree&& other);

      // Destructor.
      ~red_black_tree();

      // Clear.
      void clear();

      // Get number of elements.
      size_t count() const;

      // Insert.
      bool insert(const _Key& key, const _Value& value);

      // Erase.
      bool erase(iterator& it);
      bool erase(const _Key& key);

      // Find.
      bool find(const _Key& key, iterator& it);
      bool find(const _Key& key, const_iterator& it) const;

      // Begin.
      bool begin(iterator& it);
      bool begin(const_iterator& it) const;

      // End.
      bool end(iterator& it);
      bool end(const_iterator& it) const;

      // Previous.
      bool prev(iterator& it);
      bool prev(const_iterator& it) const;

      // Next.
      bool next(iterator& it);
      bool next(const_iterator& it) const;

    private:
      enum {
        kRed = 0,
        kBlack = 1
      };

      static node _M_nil;

      node* _M_root;

#if HAVE_FREE_LIST
      node* _M_free;
#endif // HAVE_FREE_LIST

      size_t _M_count;

      _Compare _M_comp;

      // Search.
      node* search(const _Key& key);
      const node* search(const _Key& key) const;

      // Minimum.
      node* minimum(node* x);
      const node* minimum(const node* x) const;

      // Maximum.
      node* maximum(node* x);
      const node* maximum(const node* x) const;

      // Predecessor.
      node* predecessor(node* x);
      const node* predecessor(const node* x) const;

      // Successor.
      node* successor(node* x);
      const node* successor(const node* x) const;

      // Left rotate.
      void left_rotate(node* x);

      // Right rotate.
      void right_rotate(node* x);

      // Insert fixup.
      void insert_fixup(node* z);

      // Transplant.
      void transplant(node* u, node* v);

      // Delete fixup.
      void delete_fixup(node* x);

      // Create node.
      node* create(const _Key& key, const _Value& value);

#if HAVE_FREE_LIST
      // Add node to free list.
      void add_free_list(node* node);
#endif // HAVE_FREE_LIST

      // Erase subtree.
      void erase_subtree(node* node);

      // Get nil.
      static node* nil();

      // Disable copy constructor and assignment operator.
      red_black_tree(const red_black_tree&) = delete;
      red_black_tree& operator=(const red_black_tree&) = delete;
  };

  template<typename _Key, typename _Value, typename _Compare>
  typename red_black_tree<_Key, _Value, _Compare>::node
  red_black_tree<_Key, _Value, _Compare>::_M_nil;

  template<typename _Key, typename _Value, typename _Compare>
  inline red_black_tree<_Key, _Value, _Compare>::node::node()
    : color(kBlack)
  {
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline red_black_tree<_Key, _Value, _Compare>::node::node(const _Key& k,
                                                            const _Value& v)
    : key(k),
      value(v)
  {
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline
  red_black_tree<_Key, _Value, _Compare>::red_black_tree()
    : _M_root(nil()),

#if HAVE_FREE_LIST
      _M_free(NULL),
#endif // HAVE_FREE_LIST

      _M_count(0),
      _M_comp()
  {
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline
  red_black_tree<_Key, _Value, _Compare>::red_black_tree(const _Compare& comp)
    : _M_root(nil()),

#if HAVE_FREE_LIST
      _M_free(NULL),
#endif // HAVE_FREE_LIST

      _M_count(0),
      _M_comp(comp)
  {
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline
  red_black_tree<_Key, _Value, _Compare>::red_black_tree(red_black_tree&& other)
  {
    _M_root = other._M_root;

#if HAVE_FREE_LIST
    _M_free = other._M_free;
#endif // HAVE_FREE_LIST

    _M_count = other._M_count;
    _M_comp = other._M_comp;

    other._M_root = nil();
    other._M_free = NULL;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline red_black_tree<_Key, _Value, _Compare>::~red_black_tree()
  {
    clear();
  }

  template<typename _Key, typename _Value, typename _Compare>
  void red_black_tree<_Key, _Value, _Compare>::clear()
  {
    if (_M_root != nil()) {
      erase_subtree(_M_root);
      _M_root = nil();
    }

#if HAVE_FREE_LIST
    while (_M_free) {
      node* next = _M_free->left;

      delete _M_free;

      _M_free = next;
    }
#endif // HAVE_FREE_LIST

    _M_count = 0;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline size_t red_black_tree<_Key, _Value, _Compare>::count() const
  {
    return _M_count;
  }

  template<typename _Key, typename _Value, typename _Compare>
  bool red_black_tree<_Key, _Value, _Compare>::insert(const _Key& key,
                                                      const _Value& value)
  {
    node* z;
    if ((z = create(key, value)) == NULL) {
      return false;
    }

    node* y = nil();
    node* x = _M_root;

    while (x != nil()) {
      y = x;
      if (_M_comp(z->key, x->key) < 0) {
        x = x->left;
      } else {
        x = x->right;
      }
    }

    z->p = y;

    if (y == nil()) {
      _M_root = z;
    } else if (_M_comp(z->key, y->key) < 0) {
      y->left = z;
    } else {
      y->right = z;
    }

    z->left = nil();
    z->right = nil();
    z->color = kRed;

    insert_fixup(z);

    _M_count++;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  bool red_black_tree<_Key, _Value, _Compare>::erase(iterator& it)
  {
    node* z = it._M_node;
    it._M_node = successor(z);

    node* x;
    node* y = z;

    color_t ycolor = y->color;

    if (z->left == nil()) {
      x = z->right;
      transplant(z, z->right);
    } else if (z->right == nil()) {
      x = z->left;
      transplant(z, z->left);
    } else {
      y = minimum(z->right);

      ycolor = y->color;

      x = y->right;

      if (y->p == z) {
        x->p = y;
      } else {
        transplant(y, y->right);
        y->right = z->right;
        y->right->p = y;
      }

      transplant(z, y);

      y->left = z->left;
      y->left->p = y;
      y->color = z->color;
    }

    if (ycolor == kBlack) {
      delete_fixup(x);
    }

    _M_count--;

#if HAVE_FREE_LIST
    // Call destructor.
    z->~node();

    add_free_list(z);
#else
    delete z;
#endif

    if (it._M_node == nil()) {
      return false;
    }

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool red_black_tree<_Key, _Value, _Compare>::erase(const _Key& key)
  {
    iterator it;
    if ((it._M_node = search(key)) == nil()) {
      return false;
    }

    erase(it);

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool red_black_tree<_Key, _Value, _Compare>::find(const _Key& key,
                                                           iterator& it)
  {
    if ((it._M_node = search(key)) == nil()) {
      return false;
    }

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool
  red_black_tree<_Key, _Value, _Compare>::find(const _Key& key,
                                               const_iterator& it) const
  {
    if ((it._M_node = search(key)) == nil()) {
      return false;
    }

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool red_black_tree<_Key, _Value, _Compare>::begin(iterator& it)
  {
    if (_M_root == nil()) {
      return false;
    }

    it._M_node = minimum(_M_root);

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool
  red_black_tree<_Key, _Value, _Compare>::begin(const_iterator& it) const
  {
    if (_M_root == nil()) {
      return false;
    }

    it._M_node = minimum(_M_root);

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool red_black_tree<_Key, _Value, _Compare>::end(iterator& it)
  {
    if (_M_root == nil()) {
      return false;
    }

    it._M_node = maximum(_M_root);

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool
  red_black_tree<_Key, _Value, _Compare>::end(const_iterator& it) const
  {
    if (_M_root == nil()) {
      return false;
    }

    it._M_node = maximum(_M_root);

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool red_black_tree<_Key, _Value, _Compare>::prev(iterator& it)
  {
    if ((it._M_node = predecessor(it._M_node)) == nil()) {
      return false;
    }

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool
  red_black_tree<_Key, _Value, _Compare>::prev(const_iterator& it) const
  {
    if ((it._M_node = predecessor(it._M_node)) == nil()) {
      return false;
    }

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool red_black_tree<_Key, _Value, _Compare>::next(iterator& it)
  {
    if ((it._M_node = successor(it._M_node)) == nil()) {
      return false;
    }

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline bool
  red_black_tree<_Key, _Value, _Compare>::next(const_iterator& it) const
  {
    if ((it._M_node = successor(it._M_node)) == nil()) {
      return false;
    }

    it.first = &it._M_node->key;
    it.second = &it._M_node->value;

    return true;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::search(const _Key& key)
  {
    return const_cast<red_black_tree<_Key, _Value, _Compare>::node*>(
             const_cast<const red_black_tree<_Key, _Value, _Compare>&>(
               *this
             ).search(key)
           );
  }

  template<typename _Key, typename _Value, typename _Compare>
  const typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::search(const _Key& key) const
  {
    const node* x = _M_root;
    while (x != nil()) {
      int ret;
      if ((ret = _M_comp(key, x->key)) < 0) {
        x = x->left;
      } else if (ret == 0) {
        return x;
      } else {
        x = x->right;
      }
    }

    return nil();
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::minimum(node* x)
  {
    return const_cast<red_black_tree<_Key, _Value, _Compare>::node*>(
             const_cast<const red_black_tree<_Key, _Value, _Compare>&>(
               *this
             ).minimum(x)
           );
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline const typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::minimum(const node* x) const
  {
    while (x->left != nil()) {
      x = x->left;
    }

    return x;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::maximum(node* x)
  {
    return const_cast<red_black_tree<_Key, _Value, _Compare>::node*>(
             const_cast<const red_black_tree<_Key, _Value, _Compare>&>(
               *this
             ).maximum(x)
           );
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline const typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::maximum(const node* x) const
  {
    while (x->right != nil()) {
      x = x->right;
    }

    return x;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::predecessor(node* x)
  {
    return const_cast<red_black_tree<_Key, _Value, _Compare>::node*>(
             const_cast<const red_black_tree<_Key, _Value, _Compare>&>(
               *this
             ).predecessor(x)
           );
  }

  template<typename _Key, typename _Value, typename _Compare>
  const typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::predecessor(const node* x) const
  {
    if (x->left != nil()) {
      return maximum(x->left);
    }

    const node* y = x->p;
    while ((y != nil()) && (x == y->left)) {
      x = y;
      y = y->p;
    }

    return y;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::successor(node* x)
  {
    return const_cast<red_black_tree<_Key, _Value, _Compare>::node*>(
             const_cast<const red_black_tree<_Key, _Value, _Compare>&>(
               *this
             ).successor(x)
           );
  }

  template<typename _Key, typename _Value, typename _Compare>
  const typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::successor(const node* x) const
  {
    if (x->right != nil()) {
      return minimum(x->right);
    }

    const node* y = x->p;
    while ((y != nil()) && (x == y->right)) {
      x = y;
      y = y->p;
    }

    return y;
  }

  template<typename _Key, typename _Value, typename _Compare>
  void red_black_tree<_Key, _Value, _Compare>::left_rotate(node* x)
  {
    node* y = x->right;
    x->right = y->left;

    if (y->left != nil()) {
      y->left->p = x;
    }

    y->p = x->p;

    if (x->p == nil()) {
      _M_root = y;
    } else if (x == x->p->left) {
      x->p->left = y;
    } else {
      x->p->right = y;
    }

    y->left = x;
    x->p = y;
  }

  template<typename _Key, typename _Value, typename _Compare>
  void red_black_tree<_Key, _Value, _Compare>::right_rotate(node* x)
  {
    node* y = x->left;
    x->left = y->right;

    if (y->right != nil()) {
      y->right->p = x;
    }

    y->p = x->p;

    if (x->p == nil()) {
      _M_root = y;
    } else if (x == x->p->right) {
      x->p->right = y;
    } else {
      x->p->left = y;
    }

    y->right = x;
    x->p = y;
  }

  template<typename _Key, typename _Value, typename _Compare>
  void red_black_tree<_Key, _Value, _Compare>::insert_fixup(node* z)
  {
    while (z->p->color == kRed) {
      if (z->p == z->p->p->left) {
        node* y = z->p->p->right;
        if (y->color == kRed) {
          z->p->color = kBlack;
          y->color = kBlack;
          z->p->p->color = kRed;
          z = z->p->p;
        } else {
          if (z == z->p->right) {
            z = z->p;

            left_rotate(z);
          }

          z->p->color = kBlack;
          z->p->p->color = kRed;

          right_rotate(z->p->p);
        }
      } else {
        node* y = z->p->p->left;
        if (y->color == kRed) {
          z->p->color = kBlack;
          y->color = kBlack;
          z->p->p->color = kRed;
          z = z->p->p;
        } else {
          if (z == z->p->left) {
            z = z->p;

            right_rotate(z);
          }

          z->p->color = kBlack;
          z->p->p->color = kRed;

          left_rotate(z->p->p);
        }
      }
    }

    _M_root->color = kBlack;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline void red_black_tree<_Key, _Value, _Compare>::transplant(node* u,
                                                                 node* v)
  {
    if (u->p == nil()) {
      _M_root = v;
    } else if (u == u->p->left) {
      u->p->left = v;
    } else {
      u->p->right = v;
    }

    v->p = u->p;
  }

  template<typename _Key, typename _Value, typename _Compare>
  void red_black_tree<_Key, _Value, _Compare>::delete_fixup(node* x)
  {
    while ((x != _M_root) && (x->color == kBlack)) {
      if (x == x->p->left) {
        node* w = x->p->right;
        if (w->color == kRed) {
          w->color = kBlack;
          x->p->color = kRed;
          left_rotate(x->p);
          w = x->p->right;
        }

        if ((w->left->color == kBlack) && (w->right->color == kBlack)) {
          w->color = kRed;
          x = x->p;
        } else {
          if (w->right->color == kBlack) {
            w->left->color = kBlack;
            w->color = kRed;
            right_rotate(w);
            w = x->p->right;
          }

          w->color = x->p->color;
          x->p->color = kBlack;
          w->right->color = kBlack;
          left_rotate(x->p);
          x = _M_root;
        }
      } else {
        node* w = x->p->left;
        if (w->color == kRed) {
          w->color = kBlack;
          x->p->color = kRed;
          right_rotate(x->p);
          w = x->p->left;
        }

        if ((w->right->color == kBlack) && (w->left->color == kBlack)) {
          w->color = kRed;
          x = x->p;
        } else {
          if (w->left->color == kBlack) {
            w->right->color = kBlack;
            w->color = kRed;
            left_rotate(w);
            w = x->p->left;
          }

          w->color = x->p->color;
          x->p->color = kBlack;
          w->left->color = kBlack;
          right_rotate(x->p);
          x = _M_root;
        }
      }
    }

    x->color = kBlack;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::create(const _Key& key,
                                                 const _Value& value)
  {
#if HAVE_FREE_LIST
    if (!_M_free) {
      return new (std::nothrow) node(key, value);
    }

    node* n = _M_free;
    _M_free = _M_free->left;

    return new (n) node(key, value);
#else
    return new (std::nothrow) node(key, value);
#endif
  }

#if HAVE_FREE_LIST
  template<typename _Key, typename _Value, typename _Compare>
  inline void red_black_tree<_Key, _Value, _Compare>::add_free_list(node* node)
  {
    node->left = _M_free;
    _M_free = node;
  }
#endif // HAVE_FREE_LIST

  template<typename _Key, typename _Value, typename _Compare>
  void red_black_tree<_Key, _Value, _Compare>::erase_subtree(node* node)
  {
    if (node->left != nil()) {
      erase_subtree(node->left);
    }

    if (node->right != nil()) {
      erase_subtree(node->right);
    }

    delete node;
  }

  template<typename _Key, typename _Value, typename _Compare>
  inline typename red_black_tree<_Key, _Value, _Compare>::node*
  red_black_tree<_Key, _Value, _Compare>::nil()
  {
    return &_M_nil;
  }
}

#endif // UTIL_RED_BLACK_TREE_H
