#ifndef UTIL_MINUS_H
#define UTIL_MINUS_H

namespace util {
  template<typename _T>
  struct minus {
    int operator()(const _T& x, const _T& y) const;
  };

  template<typename _T>
  inline int minus<_T>::operator()(const _T& x, const _T& y) const
  {
    return x - y;
  }
}

#endif // UTIL_MINUS_H
