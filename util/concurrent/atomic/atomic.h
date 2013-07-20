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

			template<typename _T>
			static inline _T acquire_load(const _T* ptr)
			{
#if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7))
				return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
#else
				__sync_synchronize();
				_T res = *ptr;
				__sync_synchronize();

				return res;
#endif
			}

			template<typename _T>
			static inline void release_store(_T* ptr, _T val)
			{
#if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 7))
				__atomic_store_n(ptr, val, __ATOMIC_RELEASE);
#else
				__sync_synchronize();
				*ptr = val;
				__sync_synchronize();
#endif
			}
		}
	}
}

#endif // UTIL_CONCURRENT_ATOMIC_ATOMIC_H
