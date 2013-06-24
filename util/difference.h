#ifndef UTIL_DIFFERENCE_H
#define UTIL_DIFFERENCE_H

namespace util {
	template<typename _T>
	struct difference {
		int operator()(const _T& x, const _T& y) const;
	};

	template<typename _T>
	inline int difference<_T>::operator()(const _T& x, const _T& y) const
	{
		return x - y;
	}
}

#endif // UTIL_DIFFERENCE_H
