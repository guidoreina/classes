#ifndef UTIL_VECTOR_H
#define UTIL_VECTOR_H

#include <stdlib.h>
#include <new>
#include "util/move.h"

namespace util {
  template<typename _Tp>
  class vector {
    public:
      typedef _Tp value_type;

      // Constructor.
      vector();
      vector(vector&& other);

      // Destructor.
      ~vector();

      // Move assignment operator.
      vector& operator=(vector&& other);

      // Swap content.
      void swap(vector& other);

      // Free vector.
      void free();

      // Clear vector.
      void clear();

      // Empty?
      bool empty() const;

      // Get the number of elements.
      size_t size() const;

      // Get size of allocated storage capacity.
      size_t capacity() const;

      // Reserve.
      bool reserve(size_t n);

      // Add element at the end.
      bool push_back(const value_type& x);

      // Construct and insert element at the end.
      bool emplace_back(value_type&& x);

      // Delete last element.
      bool pop_back();

      // Access element.
      const value_type* at(size_t n) const;
      bool at(size_t n, const value_type& x);

      // Construct and insert element at position.
      bool emplace_at(size_t n, value_type&& x);

    private:
      static const size_t kInitialSize = 32;

      value_type* _M_values;
      size_t _M_size;
      size_t _M_used;

      // Allocate.
      bool allocate(size_t n = 1);

      // Disable copy constructor and assignment operator.
      vector(const vector&);
      vector& operator=(const vector&);
  };

  template<typename _Tp>
  inline vector<_Tp>::vector()
    : _M_values(NULL),
      _M_size(0),
      _M_used(0)
  {
  }

  template<typename _Tp>
  inline vector<_Tp>::vector(vector&& other)
    : _M_values(other._M_values),
      _M_size(other._M_size),
      _M_used(other._M_used)
  {
    other._M_values = NULL;
    other._M_size = 0;
    other._M_used = 0;
  }

  template<typename _Tp>
  inline vector<_Tp>::~vector()
  {
    free();
  }

  template<typename _Tp>
  vector<_Tp>& vector<_Tp>::operator=(vector&& other)
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
  inline void vector<_Tp>::swap(vector& other)
  {
    util::swap(_M_values, other._M_values);
    util::swap(_M_size, other._M_size);
    util::swap(_M_used, other._M_used);
  }

  template<typename _Tp>
  void vector<_Tp>::free()
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
  inline void vector<_Tp>::clear()
  {
    // Invoke the destructors.
    for (size_t i = 0; i < _M_used; i++) {
      _M_values[i].value_type::~value_type();
    }

    _M_used = 0;
  }

  template<typename _Tp>
  inline bool vector<_Tp>::empty() const
  {
    return (_M_used == 0);
  }

  template<typename _Tp>
  inline size_t vector<_Tp>::size() const
  {
    return _M_used;
  }

  template<typename _Tp>
  inline size_t vector<_Tp>::capacity() const
  {
    return _M_size;
  }

  template<typename _Tp>
  bool vector<_Tp>::reserve(size_t n)
  {
    if (n <= _M_size) {
      return true;
    }

    size_t size = (_M_size == 0) ? kInitialSize : _M_size;

    while (size < n) {
      // Overflow?
      size_t tmp;
      if ((tmp = size * 2) < size) {
        return false;
      }

      size = tmp;
    }

    value_type* values;
    if ((values = reinterpret_cast<value_type*>(
                    realloc(_M_values, size * sizeof(value_type))
                  )) == NULL) {
      return false;
    }

    _M_values = values;
    _M_size = size;

    return true;
  }

  template<typename _Tp>
  inline bool vector<_Tp>::push_back(const value_type& x)
  {
    if (!allocate()) {
      return false;
    }

    new (&_M_values[_M_used++]) value_type(x);

    return true;
  }

  template<typename _Tp>
  inline bool vector<_Tp>::emplace_back(value_type&& x)
  {
    if (!allocate()) {
      return false;
    }

    new (&_M_values[_M_used++]) value_type(util::move(x));

    return true;
  }

  template<typename _Tp>
  inline bool vector<_Tp>::pop_back()
  {
    if (_M_used == 0) {
      return false;
    }

    // Invoke the destructor.
    _M_values[--_M_used].value_type::~value_type();

    return true;
  }

  template<typename _Tp>
  inline const typename vector<_Tp>::value_type* vector<_Tp>::at(size_t n) const
  {
    if (n >= _M_used) {
      return NULL;
    }

    return &_M_values[n];
  }

  template<typename _Tp>
  inline bool vector<_Tp>::at(size_t n, const value_type& x)
  {
    if (n >= _M_used) {
      return false;
    }

    // Invoke the destructor.
    _M_values[n].value_type::~value_type();

    new (&_M_values[n]) value_type(x);

    return true;
  }

  template<typename _Tp>
  inline bool vector<_Tp>::emplace_at(size_t n, value_type&& x)
  {
    if (n >= _M_used) {
      return false;
    }

    // Invoke the destructor.
    _M_values[n].value_type::~value_type();

    new (&_M_values[n]) value_type(util::move(x));

    return true;
  }

  template<typename _Tp>
  inline bool vector<_Tp>::allocate(size_t n)
  {
    return reserve(_M_used + n);
  }
}

#endif // UTIL_VECTOR_H
