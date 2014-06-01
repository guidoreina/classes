#ifndef UTIL_FIFO_H
#define UTIL_FIFO_H

#include <stdlib.h>
#include <memory>
#include "util/move.h"

namespace util {
  template<typename _T>
  class fifo {
    public:
      // Constructor.
      fifo();
      fifo(fifo&& other);

      // Destructor.
      ~fifo();

      // Clear.
      void clear();

      // Empty?
      bool empty() const;

      // Get number of elements.
      size_t count() const;

      // Push element.
      bool push(const _T& x);
      bool push(_T&& x);

      // Pop element.
      bool pop();

      // Front element.
      _T* front();
      const _T* front() const;

      // Back element.
      _T* back();
      const _T* back() const;

    private:
      struct node {
        _T elem;
        node* next;

        // Constructor.
        node(const _T& x);
        node(_T&& x);
      };

      node* _M_head;
      node* _M_tail;

#if HAVE_FREE_LIST
      node* _M_free;
#endif // HAVE_FREE_LIST

      size_t _M_count;

      // Create node.
      node* create(const _T& x);
      node* create(_T&& x);

      // Push.
      void push(node* node);

#if HAVE_FREE_LIST
      // Add node to free list.
      void add_free_list(node* node);
#endif // HAVE_FREE_LIST

      // Disable copy constructor and assignment operator.
      fifo(const fifo&) = delete;
      fifo& operator=(const fifo&) = delete;
  };

  template<typename _T>
  inline fifo<_T>::fifo()
    : _M_head(NULL),
      _M_tail(NULL),

#if HAVE_FREE_LIST
      _M_free(NULL),
#endif // HAVE_FREE_LIST

      _M_count(0)
  {
  }

  template<typename _T>
  inline fifo<_T>::fifo(fifo&& other)
  {
    _M_head = other._M_head;
    _M_tail = other._M_tail;

#if HAVE_FREE_LIST
    _M_free = other._M_free;
#endif // HAVE_FREE_LIST

    _M_count = other._M_count;

    other._M_head = NULL;
    other._M_tail = NULL;

#if HAVE_FREE_LIST
    other._M_free = NULL;
#endif // HAVE_FREE_LIST
  }

  template<typename _T>
  inline fifo<_T>::~fifo()
  {
    clear();
  }

  template<typename _T>
  void fifo<_T>::clear()
  {
    while (_M_head) {
      node* next = _M_head->next;

      delete _M_head;

      _M_head = next;
    }

    _M_tail = NULL;

#if HAVE_FREE_LIST
    while (_M_free) {
      node* next = _M_free->next;

      delete _M_free;

      _M_free = next;
    }
#endif // HAVE_FREE_LIST

    _M_count = 0;
  }

  template<typename _T>
  inline bool fifo<_T>::empty() const
  {
    return (_M_head == NULL);
  }

  template<typename _T>
  inline size_t fifo<_T>::count() const
  {
    return _M_count;
  }

  template<typename _T>
  inline bool fifo<_T>::push(const _T& x)
  {
    node* node;
    if ((node = create(x)) == NULL) {
      return false;
    }

    push(node);

    return true;
  }

  template<typename _T>
  inline bool fifo<_T>::push(_T&& x)
  {
    node* node;
    if ((node = create(x)) == NULL) {
      return false;
    }

    push(node);

    return true;
  }

  template<typename _T>
  bool fifo<_T>::pop()
  {
    // If the list is empty...
    if (!_M_head) {
      return false;
    }

    // Save head.
    node* node = _M_head;

    // Advance head.
    if ((_M_head = _M_head->next) == NULL) {
      _M_tail = NULL;
    }

#if HAVE_FREE_LIST
    // Call destructor.
    node->~node();

    add_free_list(node);
#else
    delete node;
#endif

    _M_count--;

    return true;
  }

  template<typename _T>
  inline _T* fifo<_T>::front()
  {
    return _M_head ? &_M_head->elem : NULL;
  }

  template<typename _T>
  inline const _T* fifo<_T>::front() const
  {
    return _M_head ? &_M_head->elem : NULL;
  }

  template<typename _T>
  inline _T* fifo<_T>::back()
  {
    return _M_tail ? &_M_tail->elem : NULL;
  }

  template<typename _T>
  inline const _T* fifo<_T>::back() const
  {
    return _M_tail ? &_M_tail->elem : NULL;
  }

  template<typename _T>
  inline fifo<_T>::node::node(const _T& x)
    : elem(x)
  {
  }

  template<typename _T>
  inline fifo<_T>::node::node(_T&& x)
    : elem(util::move(x))
  {
  }

  template<typename _T>
  inline typename fifo<_T>::node* fifo<_T>::create(const _T& x)
  {
#if HAVE_FREE_LIST
    if (!_M_free) {
      return new (std::nothrow) node(x);
    }

    node* n = _M_free;
    _M_free = _M_free->next;

    return new (n) node(x);
#else
    return new (std::nothrow) node(x);
#endif
  }

  template<typename _T>
  inline typename fifo<_T>::node* fifo<_T>::create(_T&& x)
  {
#if HAVE_FREE_LIST
    if (!_M_free) {
      return new (std::nothrow) node(x);
    }

    node* n = _M_free;
    _M_free = _M_free->next;

    return new (n) node(x);
#else
    return new (std::nothrow) node(x);
#endif
  }

  template<typename _T>
  void fifo<_T>::push(node* node)
  {
    node->next = NULL;

    if (!_M_head) {
      _M_head = node;
    } else {
      _M_tail->next = node;
    }

    _M_tail = node;

    _M_count++;
  }

#if HAVE_FREE_LIST
  template<typename _T>
  inline void fifo<_T>::add_free_list(node* node)
  {
    node->next = _M_free;
    _M_free = node;
  }
#endif // HAVE_FREE_LIST
}

#endif // UTIL_FIFO_H
