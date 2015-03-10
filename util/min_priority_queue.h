#ifndef UTIL_MIN_PRIORITY_QUEUE_H
#define UTIL_MIN_PRIORITY_QUEUE_H

#include <stdlib.h>
#include <string.h>
#include <new>
#include "util/move.h"

namespace util {
  template<typename _Tp>
  class min_priority_queue {
    public:
      typedef _Tp value_type;

      // Constructor.
      min_priority_queue();
      min_priority_queue(min_priority_queue&& other);

      // Destructor.
      ~min_priority_queue();

      // Move assignment operator.
      min_priority_queue& operator=(min_priority_queue&& other);

      // Swap content.
      void swap(min_priority_queue& other);

      // Free priority queue.
      void free();

      // Clear priority queue.
      void clear();

      // Empty?
      bool empty() const;

      // Get the number of elements.
      size_t size() const;

      // Insert element.
      bool push(const value_type& x);
      bool push(value_type&& x);

      // Return top element.
      const value_type* top() const;

      // Remove top element.
      bool pop();

    private:
      static const size_t kInitialSize = 32;

      value_type* _M_values;
      size_t _M_size;
      size_t _M_used;

      // Build min-heap.
      void build_min_heap();

      // Min-heapify.
      void min_heapify(size_t i);

      // Allocate.
      bool allocate();

      // Disable copy constructor and assignment operator.
      min_priority_queue(const min_priority_queue&);
      min_priority_queue& operator=(const min_priority_queue&);
  };

  #define PARENT(i) (((i) - 1) >> 1)
  #define LEFT(i)   (((i) << 1) + 1)
  #define RIGHT(i)  (((i) << 1) + 2)

  template<typename _Tp>
  inline min_priority_queue<_Tp>::min_priority_queue()
    : _M_values(NULL),
      _M_size(0),
      _M_used(0)
  {
  }

  template<typename _Tp>
  inline min_priority_queue<_Tp>::min_priority_queue(min_priority_queue&& other)
    : _M_values(other._M_values),
      _M_size(other._M_size),
      _M_used(other._M_used)
  {
    other._M_values = NULL;
    other._M_size = 0;
    other._M_used = 0;
  }

  template<typename _Tp>
  inline min_priority_queue<_Tp>::~min_priority_queue()
  {
    free();
  }

  template<typename _Tp>
  min_priority_queue<_Tp>&
  min_priority_queue<_Tp>::operator=(min_priority_queue&& other)
  {
    // http://scottmeyers.blogspot.de/2014/06/the-drawbacks-of-implementing-move.html
    // It is better not to swap the objects.

    if (_M_values) {
      // Invoke the destructors.
      for (size_t i = 0; i < _M_used; i++) {
        _M_values[i].value_type::~value_type();
      }

      ::free(_M_values);
    }

    _M_values = other._M_values;
    _M_size = other._M_size;
    _M_used = other._M_used;

    other._M_values = NULL;
    other._M_size = 0;
    other._M_used = 0;

    return *this;
  }

  template<typename _Tp>
  inline void min_priority_queue<_Tp>::swap(min_priority_queue& other)
  {
    util::swap(_M_values, other._M_values);
    util::swap(_M_size, other._M_size);
    util::swap(_M_used, other,_M_used);
  }

  template<typename _Tp>
  void min_priority_queue<_Tp>::free()
  {
    if (_M_values) {
      // Invoke the destructors.
      for (size_t i = 0; i < _M_used; i++) {
        _M_values[i].value_type::~value_type();
      }

      ::free(_M_values);
      _M_values = NULL;
    }

    _M_size = 0;
    _M_used = 0;
  }

  template<typename _Tp>
  inline void min_priority_queue<_Tp>::clear()
  {
    // Invoke the destructors.
    for (size_t i = 0; i < _M_used; i++) {
      _M_values[i].value_type::~value_type();
    }

    _M_used = 0;
  }

  template<typename _Tp>
  inline bool min_priority_queue<_Tp>::empty() const
  {
    return (_M_used == 0);
  }

  template<typename _Tp>
  inline size_t min_priority_queue<_Tp>::size() const
  {
    return _M_used;
  }

  template<typename _Tp>
  bool min_priority_queue<_Tp>::push(const value_type& x)
  {
    if (!allocate()) {
      return false;
    }

    new (&_M_values[_M_used]) value_type(x);

    size_t i = _M_used++;

    while (i > 0) {
      size_t parent = PARENT(i);
      if (_M_values[i] < _M_values[parent]) {
        util::swap(_M_values[i], _M_values[parent]);
        i = parent;
      } else {
        break;
      }
    }

    return true;
  }

  template<typename _Tp>
  bool min_priority_queue<_Tp>::push(value_type&& x)
  {
    if (!allocate()) {
      return false;
    }

    new (&_M_values[_M_used]) value_type(util::move(x));

    size_t i = _M_used++;

    while (i > 0) {
      size_t parent = PARENT(i);
      if (_M_values[i] < _M_values[parent]) {
        util::swap(_M_values[i], _M_values[parent]);
        i = parent;
      } else {
        break;
      }
    }

    return true;
  }

  template<typename _Tp>
  inline const typename min_priority_queue<_Tp>::value_type*
  min_priority_queue<_Tp>::top() const
  {
    return (_M_used != 0) ? &_M_values[0] : NULL;
  }

  template<typename _Tp>
  bool min_priority_queue<_Tp>::pop()
  {
    // If the priority queue is empty...
    if (_M_used == 0) {
      return false;
    }

    // Invoke the destructor.
    _M_values[0].value_type::~value_type();

    // Decrement number of elements.
    _M_used--;

    if (_M_used > 0) {
      memcpy(&_M_values[0], &_M_values[_M_used], sizeof(value_type));

      if (_M_used > 1) {
        min_heapify(0);
      }
    }

    return true;
  }

  template<typename _Tp>
  inline void min_priority_queue<_Tp>::build_min_heap()
  {
    for (ssize_t i = (_M_used / 2) - 1; i >= 0; i--) {
      min_heapify(i);
    }
  }

  template<typename _Tp>
  void min_priority_queue<_Tp>::min_heapify(size_t i)
  {
    size_t l = LEFT(i);
    size_t r = RIGHT(i);

    size_t smallest;
    if ((l < _M_used) && (_M_values[l] < _M_values[i])) {
      smallest = l;
    } else {
      smallest = i;
    }

    if ((r < _M_used) && (_M_values[r] < _M_values[smallest])) {
      smallest = r;
    }

    if (smallest != i) {
      util::swap(_M_values[i], _M_values[smallest]);
      min_heapify(smallest);
    }
  }

  template<typename _Tp>
  bool min_priority_queue<_Tp>::allocate()
  {
    if (_M_used == _M_size) {
      size_t size = (_M_size == 0) ? kInitialSize : (_M_size * 2);

      value_type* values;
      if ((values = reinterpret_cast<value_type*>(
                      realloc(_M_values, size * sizeof(value_type))
                    )) == NULL) {
        return false;
      }

      _M_values = values;
      _M_size = size;
    }

    return true;
  }

  #undef PARENT
  #undef LEFT
  #undef RIGHT
}

#endif // UTIL_MIN_PRIORITY_QUEUE_H
