#ifndef UTIL_CONCURRENT_LOCKS_MUTEX_H
#define UTIL_CONCURRENT_LOCKS_MUTEX_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

namespace util {
	namespace concurrent {
		namespace locks {
			class mutex {
				public:
					// Constructor.
					mutex();

					// Destructor.
					~mutex();

					// Lock.
					bool lock();

					// Try lock.
					bool try_lock();

					// Unlock.
					bool unlock();

				private:
					pthread_mutex_t _M_mutex;
			};

			inline mutex::mutex()
			{
				int ret;
				if ((ret = pthread_mutex_init(&_M_mutex, NULL)) != 0) {
					fprintf(stderr, "pthread_mutex_init failed (%s).\n", strerror(ret));
					abort();
				}
			}

			inline mutex::~mutex()
			{
				pthread_mutex_destroy(&_M_mutex);
			}

			inline bool mutex::lock()
			{
				return (pthread_mutex_lock(&_M_mutex) == 0);
			}

			inline bool mutex::try_lock()
			{
				return (pthread_mutex_trylock(&_M_mutex) == 0);
			}

			inline bool mutex::unlock()
			{
				return (pthread_mutex_unlock(&_M_mutex) == 0);
			}

			class scoped_mutex {
				public:
					// Constructor.
					scoped_mutex(mutex& m);

					// Destructor.
					~scoped_mutex();

				private:
					mutex& _M_mutex;
			};

			inline scoped_mutex::scoped_mutex(mutex& m)
			: _M_mutex(m)
			{
				_M_mutex.lock();
			}

			inline scoped_mutex::~scoped_mutex()
			{
				_M_mutex.unlock();
			}
		}
	}
}

#endif // UTIL_CONCURRENT_LOCKS_MUTEX_H
