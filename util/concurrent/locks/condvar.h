#ifndef UTIL_CONCURRENT_LOCKS_CONDVAR_H
#define UTIL_CONCURRENT_LOCKS_CONDVAR_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

namespace util {
  namespace concurrent {
    namespace locks {
      class condvar {
        public:
          // Constructor.
          condvar();

          // Destructor.
          ~condvar();

          // Lock.
          bool lock();

          // Notify one thread.
          bool notify();

          // Notify all threads.
          bool notify _all();

          // Wait.
          bool wait();
          bool wait(unsigned long usec);

        private:
          pthread_cond_t _M_cond;
          pthread_mutex_t _M_mutex;
      };

      inline condvar::condvar()
      {
        int ret;
        if ((ret = pthread_cond_init(&_M_cond, NULL)) != 0) {
          fprintf(stderr, "pthread_cond_init() failed (%s).\n", strerror(ret));
          abort();
        }

        if ((ret = pthread_mutex_init(&_M_mutex, NULL)) != 0) {
          fprintf(stderr, "pthread_mutex_init() failed (%s).\n", strerror(ret));

          pthread_cond_destroy(&_M_cond);
          abort();
        }
      }

      inline condvar::~condvar()
      {
        pthread_cond_destroy(&_M_cond);
        pthread_mutex_destroy(&_M_mutex);
      }

      inline bool condvar::lock()
      {
        return (pthread_mutex_lock(&_M_mutex) == 0);
      }

      inline bool condvar::notify()
      {
        return (pthread_cond_signal(&_M_cond) == 0);
      }

      inline bool condvar::notify_all()
      {
        return (pthread_cond_broadcast(&_M_cond) == 0);
      }

      inline bool condvar::wait()
      {
        return (pthread_cond_wait(&_M_cond, &_M_mutex) == 0);
      }

      inline bool condvar::wait(unsigned long usec)
      {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        long tv_nsec = ts.tv_nsec + ((usec % 1000000) * 1000);

        ts.tv_sec += ((usec / 1000000) + (tv_nsec % 1000000000));
        ts.tv_nsec = tv_nsec % 1000000000;

        return (pthread_cond_timedwait(&_M_cond, &_M_mutex, &ts) == 0);
      }
    }
  }
}

#endif // UTIL_CONCURRENT_LOCKS_CONDVAR_H
