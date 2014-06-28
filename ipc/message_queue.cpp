#include <sys/select.h>
#include <errno.h>
#include "ipc/message_queue.h"

ssize_t ipc::message_queue::recv(void* buf, size_t count, int timeout)
{
  ssize_t ret;
  if (timeout == 0) {
    while (((ret = mq_receive(_M_qd,
                              reinterpret_cast<char*>(buf),
                              count,
                              NULL)) < 0) && (errno == EINTR));
  } else {
    do {
      if (!wait_readable(timeout)) {
        return -1;
      }
    } while ((((ret = mq_receive(_M_qd,
                                 reinterpret_cast<char*>(buf),
                                 count,
                                 NULL)) < 0) &&
             ((errno == EINTR) || (errno == EAGAIN))));
  }

  return ret;
}

bool ipc::message_queue::send(const void* buf,
                              size_t count,
                              unsigned priority,
                              int timeout)
{
  int ret;
  if (timeout == 0) {
    while (((ret = mq_send(_M_qd,
                           reinterpret_cast<const char*>(buf),
                           count,
                           priority)) < 0) && (errno == EINTR));
  } else {
    do {
      if (!wait_writable(timeout)) {
        return false;
      }
    } while ((((ret = mq_send(_M_qd,
                              reinterpret_cast<const char*>(buf),
                              count,
                              priority)) < 0) &&
             ((errno == EINTR) || (errno == EAGAIN))));
  }

  return (ret == 0);
}

bool ipc::message_queue::wait_readable(int timeout)
{
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(_M_qd, &rfds);

  if (timeout < 0) {
    return (select(_M_qd + 1, &rfds, NULL, NULL, NULL) > 0);
  } else {
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    return (select(_M_qd + 1, &rfds, NULL, NULL, &tv) > 0);
  }
}

bool ipc::message_queue::wait_writable(int timeout)
{
  fd_set wfds;
  FD_ZERO(&wfds);
  FD_SET(_M_qd, &wfds);

  if (timeout < 0) {
    return (select(_M_qd + 1, NULL, &wfds, NULL, NULL) > 0);
  } else {
    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    return (select(_M_qd + 1, NULL, &wfds, NULL, &tv) > 0);
  }
}
