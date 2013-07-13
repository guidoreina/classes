#ifndef UTIL_CONCURRENT_LOCKS_SPINLOCK_H
#define UTIL_CONCURRENT_LOCKS_SPINLOCK_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

namespace util {
	namespace concurrent {
		namespace locks {
			class spinlock {
				public:
					// Constructor.
					spinlock();

					// Destructor.
					~spinlock();

					// Lock.
					bool lock();

					// Try lock.
					bool try_lock();

					// Unlock.
					bool unlock();

				private:
					pthread_spinlock_t _M_spinlock;
			};

			inline spinlock::spinlock()
			{
				int ret;
				if ((ret = pthread_spin_init(&_M_spinlock, 0)) != 0) {
					fprintf(stderr, "pthread_spin_init failed (%s).\n", strerror(ret));
					abort();
				}
			}

			inline spinlock::~spinlock()
			{
				pthread_spin_destroy(&_M_spinlock);
			}

			inline bool spinlock::lock()
			{
				return (pthread_spin_lock(&_M_spinlock) == 0);
			}

			inline bool spinlock::try_lock()
			{
				return (pthread_spin_trylock(&_M_spinlock) == 0);
			}

			inline bool spinlock::unlock()
			{
				return (pthread_spin_unlock(&_M_spinlock) == 0);
			}

			class scoped_spinlock {
				public:
					// Constructor.
					scoped_spinlock(spinlock& sl);

					// Destructor.
					~scoped_spinlock();

				private:
					spinlock& _M_spinlock;
			};

			inline scoped_spinlock::scoped_spinlock(spinlock& sl)
			: _M_spinlock(sl)
			{
				_M_spinlock.lock();
			}

			inline scoped_spinlock::~scoped_spinlock()
			{
				_M_spinlock.unlock();
			}
		}
	}
}

#endif // UTIL_CONCURRENT_LOCKS_SPINLOCK_H
