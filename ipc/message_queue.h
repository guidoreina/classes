#ifndef IPC_MESSAGE_QUEUE_H
#define IPC_MESSAGE_QUEUE_H

#include <fcntl.h>
#include <stdio.h>
#include <mqueue.h>
#include <limits.h>

namespace ipc {
  class message_queue {
    public:
      // Constructor.
      message_queue(bool unlink_in_destructor = false);

      // Destructor.
      ~message_queue();

      // Open.
      bool open(const char* name);
      bool open(const char* name, unsigned maxmsgs, size_t msgsize);

      // Close.
      bool close();

      // Unlink message queue.
      bool unlink();

      // Receive message.
      ssize_t recv(void* buf,
                   size_t count,
                   int timeout = -1); // Timeout in milliseconds.

      // Send message.
      bool send(const void* buf,
                size_t count,
                int timeout = -1); // Timeout in milliseconds.

      bool send(const void* buf,
                size_t count,
                unsigned priority,
                int timeout = -1); // Timeout in milliseconds.

      // Wait readable.
      bool wait_readable(int timeout); // Timeout in milliseconds.

      // Wait writable.
      bool wait_writable(int timeout); // Timeout in milliseconds.

      // Get message queue descriptor.
      mqd_t qd() const;

    private:
      char _M_name[NAME_MAX + 1];
      mqd_t _M_qd;

      bool _M_unlink_in_destructor;
  };

  inline message_queue::message_queue(bool unlink_in_destructor)
    : _M_qd(-1),
      _M_unlink_in_destructor(unlink_in_destructor)
  {
    *_M_name = 0;
  }

  inline message_queue::~message_queue()
  {
    close();

    if ((_M_unlink_in_destructor) && (*_M_name)) {
      unlink();
    }
  }

  inline bool message_queue::open(const char* name)
  {
    snprintf(_M_name, sizeof(_M_name), "/%s", name);
    return ((_M_qd = mq_open(_M_name,
                             O_CREAT | O_RDWR | O_NONBLOCK,
                             0644,
                             NULL)) != -1);
  }

  inline bool message_queue::open(const char* name,
                                  unsigned maxmsgs,
                                  size_t msgsize)
  {
    snprintf(_M_name, sizeof(_M_name), "/%s", name);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = maxmsgs;
    attr.mq_msgsize = msgsize;
    attr.mq_curmsgs = 0;

    return ((_M_qd = mq_open(_M_name,
                             O_CREAT | O_RDWR | O_NONBLOCK,
                             0644,
                             &attr)) != -1);
  }

  inline bool message_queue::close()
  {
    if (_M_qd != -1) {
      if (mq_close(_M_qd) < 0) {
        return false;
      }

      _M_qd = -1;
    }

    return true;
  }

  inline bool message_queue::unlink()
  {
    return (mq_unlink(_M_name) == 0);
  }

  inline bool message_queue::send(const void* buf, size_t count, int timeout)
  {
    return send(buf, count, 0, timeout);
  }

  inline mqd_t message_queue::qd() const
  {
    return _M_qd;
  }
}

#endif // IPC_MESSAGE_QUEUE_H
