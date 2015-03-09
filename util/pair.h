#ifndef UTIL_PAIR_H
#define UTIL_PAIR_H

#include "util/move.h"

namespace util {
  template<typename _T1, typename _T2>
  struct pair {
    typedef _T1 first_type;
    typedef _T2 second_type;

    _T1 first;
    _T2 second;

    // Constructor.
    pair();
    pair(const _T1& a, const _T2& b);
    pair(_T1&& a, _T2&& b);
  };

  inline pair::pair()
    : first(),
      second()
  {
  }

  inline pair::pair(const _T1& a, const _T2& b)
    : first(a),
      second(b)
  {
  }

  inline pair::pair(_T1&& a, _T2&& b)
  {
    first = util::move(a);
    second = util::move(b);
  }
}

#endif // UTIL_PAIR_H
