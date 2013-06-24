#ifndef UTIL_CONCURRENT_ATOMIC_MARKABLE_PTR_H
#define UTIL_CONCURRENT_ATOMIC_MARKABLE_PTR_H

#include <stdint.h>
#include "util/concurrent/atomic/atomic.h"

namespace util {
	namespace concurrent {
		namespace atomic {
			template<typename _T>
			class markable_ptr {
				public:
					// Constructor.
					markable_ptr();
					markable_ptr(_T* ptr);
					markable_ptr(const markable_ptr& ptr);

					// Assignment operator.
					markable_ptr& operator=(_T* ptr);
					markable_ptr& operator=(const markable_ptr& ptr);

					// Get raw pointer.
					_T* get() const;

					// Get raw pointer and mark.
					_T* get(bool& marked) const;

					// Set raw pointer and mark.
					void set(_T* ptr, bool mark);

					// Pointer operator.
					_T* operator->() const;

					// Dereference operator.
					_T& operator*() const;

					// Mark.
					void mark();

					// Unmark.
					void unmark();

					// Marked?
					bool marked() const;

					// Compare and swap.
					bool compare_and_swap(_T* expected_ptr, _T* newptr, bool expected_mark, bool newmark);
					bool compare_and_swap(_T* expected_ptr, _T* newptr, bool expected_mark, bool newmark, bool& oldmark);

				private:
					_T* _M_ptr;
			};

			template<typename _T>
			inline markable_ptr<_T>::markable_ptr()
			{
			}

			template<typename _T>
			inline markable_ptr<_T>::markable_ptr(_T* ptr)
			: _M_ptr(ptr)
			{
			}

			template<typename _T>
			inline markable_ptr<_T>::markable_ptr(const markable_ptr& ptr)
			: _M_ptr(ptr._M_ptr)
			{
			}

			template<typename _T>
			inline markable_ptr<_T>& markable_ptr<_T>::operator=(_T* ptr)
			{
				_M_ptr = ptr;
				return *this;
			}

			template<typename _T>
			inline markable_ptr<_T>& markable_ptr<_T>::operator=(const markable_ptr& ptr)
			{
				_M_ptr = ptr._M_ptr;
				return *this;
			}

			template<typename _T>
			inline _T* markable_ptr<_T>::get() const
			{
				return reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(_M_ptr) & ~0x1);
			}

			template<typename _T>
			inline _T* markable_ptr<_T>::get(bool& marked) const
			{
				_T* ptr = _M_ptr;
				marked = ((reinterpret_cast<uintptr_t>(ptr) & 0x1) != 0);
				return reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(ptr) & ~0x1);
			}

			template<typename _T>
			inline void markable_ptr<_T>::set(_T* ptr, bool mark)
			{
				_M_ptr = reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(ptr) | (!!mark));
			}

			template<typename _T>
			inline _T* markable_ptr<_T>::operator->() const
			{
				return reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(_M_ptr) & ~0x1);
			}

			template<typename _T>
			inline _T& markable_ptr<_T>::operator*() const
			{
				return *(reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(_M_ptr) & ~0x1));
			}

			template<typename _T>
			inline void markable_ptr<_T>::mark()
			{
				_M_ptr = reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(_M_ptr) | 0x1);
			}

			template<typename _T>
			inline void markable_ptr<_T>::unmark()
			{
				_M_ptr = reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(_M_ptr) & ~0x1);
			}

			template<typename _T>
			inline bool markable_ptr<_T>::marked() const
			{
				return ((reinterpret_cast<uintptr_t>(_M_ptr) & 0x1) != 0);
			}

			template<typename _T>
			inline bool markable_ptr<_T>::compare_and_swap(_T* expected_ptr, _T* newptr, bool expected_mark, bool newmark)
			{
				_T* oldval = reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(expected_ptr) | (!!expected_mark));
				_T* newval = reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(newptr) | (!!newmark));
				return util::concurrent::atomic::bool_compare_and_swap<_T*>(&_M_ptr, oldval, newval);
			}

			template<typename _T>
			inline bool markable_ptr<_T>::compare_and_swap(_T* expected_ptr, _T* newptr, bool expected_mark, bool newmark, bool& oldmark)
			{
				_T* oldval = reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(expected_ptr) | (!!expected_mark));
				_T* newval = reinterpret_cast<_T*>(reinterpret_cast<uintptr_t>(newptr) | (!!newmark));

				_T* ptr;
				if ((ptr = util::concurrent::atomic::val_compare_and_swap<_T*>(&_M_ptr, oldval, newval)) == oldval) {
					return true;
				}

				oldmark = ((reinterpret_cast<uintptr_t>(ptr) & 0x1) != 0);

				return false;
			}
		}
	}
}

#endif // UTIL_CONCURRENT_ATOMIC_MARKABLE_PTR_H
