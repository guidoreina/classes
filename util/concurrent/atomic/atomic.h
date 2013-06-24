#ifndef UTIL_CONCURRENT_ATOMIC_ATOMIC_H
#define UTIL_CONCURRENT_ATOMIC_ATOMIC_H

namespace util {
	namespace concurrent {
		namespace atomic {
			static inline void memory_barrier()
			{
				asm volatile("" : : : "memory");
			}

			template<typename _T>
			static inline bool bool_compare_and_swap(_T* ptr, _T oldval, _T newval)
			{
				return __sync_bool_compare_and_swap(ptr, oldval, newval);
			}

			template<typename _T>
			static inline _T val_compare_and_swap(_T* ptr, _T oldval, _T newval)
			{
				return __sync_val_compare_and_swap(ptr, oldval, newval);
			}

			template<typename _T>
			static inline _T add(_T* ptr, _T val)
			{
				return __sync_fetch_and_add(ptr, val);
			}

			template<typename _T>
			static inline _T sub(_T* ptr, _T val)
			{
				return __sync_fetch_and_sub(ptr, val);
			}
		}
	}
}

#endif // UTIL_CONCURRENT_ATOMIC_ATOMIC_H
