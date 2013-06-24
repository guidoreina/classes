#ifndef UTIL_CONCURRENT_ATOMIC_POINTER_H
#define UTIL_CONCURRENT_ATOMIC_POINTER_H

#include "util/concurrent/atomic/atomic.h"

namespace util {
	namespace concurrent {
		namespace atomic {
			template<typename _T>
			class pointer {
				public:
					// Constructor.
					pointer();
					pointer(_T* ptr);
					pointer(const pointer& ptr);

					// Assignment operator.
					pointer& operator=(_T* ptr);
					pointer& operator=(const pointer& ptr);

					// Get pointer.
					_T* get() const;

					// Set pointer.
					void set(_T* ptr);

					// Pointer operator.
					_T* operator->() const;

					// Dereference operator.
					_T& operator*() const;

					// Compare and swap.
					bool compare_and_swap(_T* oldval, _T* newval);

				private:
					_T* _M_ptr;
			};

			template<typename _T>
			inline pointer<_T>::pointer()
			{
			}

			template<typename _T>
			inline pointer<_T>::pointer(_T* ptr)
			: _M_ptr(ptr)
			{
			}

			template<typename _T>
			inline pointer<_T>::pointer(const pointer& ptr)
			: _M_ptr(ptr._M_ptr)
			{
			}

			template<typename _T>
			inline pointer<_T>& pointer<_T>::operator=(_T* ptr)
			{
				_M_ptr = ptr;
				return *this;
			}

			template<typename _T>
			inline pointer<_T>& pointer<_T>::operator=(const pointer& ptr)
			{
				_M_ptr = ptr._M_ptr;
				return *this;
			}

			template<typename _T>
			inline _T* pointer<_T>::get() const
			{
				return _M_ptr;
			}

			template<typename _T>
			inline void pointer<_T>::set(_T* ptr)
			{
				_M_ptr = ptr;
			}

			template<typename _T>
			inline _T* pointer<_T>::operator->() const
			{
				return _M_ptr;
			}

			template<typename _T>
			inline _T& pointer<_T>::operator*() const
			{
				return *_M_ptr;
			}

			template<typename _T>
			inline bool pointer<_T>::compare_and_swap(_T* oldval, _T* newval)
			{
				return util::concurrent::atomic::bool_compare_and_swap<_T*>(&_M_ptr, oldval, newval);
			}
		}
	}
}

#endif // UTIL_CONCURRENT_ATOMIC_POINTER_H
