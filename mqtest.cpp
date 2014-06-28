#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "ipc/message_queue.h"

static void usage(const char* program);
static void signal_handler(int nsignal);

static const unsigned kMaxMessages = 10;
static const size_t kMaxMessageSize = 128;
static const unsigned kDefaultTimeout = -1; // Blocking.
static const unsigned kNumberMessages = 10;

static bool running = false;

int main(int argc, const char** argv)
{
  // Check arguments.
  if ((argc < 3) || (argc > 4)) {
    usage(argv[0]);
    return -1;
  }

  bool sender;
  if (strcasecmp(argv[2], "sender") == 0) {
    sender = true;
  } else if (strcasecmp(argv[2], "receiver") == 0) {
    sender = false;
  } else {
    usage(argv[0]);
    return -1;
  }

  int timeout;
  if (argc == 4) {
    timeout = atoi(argv[3]);
  } else {
    timeout = kDefaultTimeout;
  }

  // Open/create message queue.
  ipc::message_queue mq(!sender);
  if (!mq.open(argv[1], kMaxMessages, kMaxMessageSize)) {
    fprintf(stderr, "Couldn't open queue (%s).\n", argv[1]);
    return -1;
  }

  pid_t pid = getpid();

  // Install signal handlers.
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = signal_handler;
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGTERM, &act, NULL);

  running = true;

  if (sender) {
    // Send messages.
    for (unsigned i = 0; (running) && (i < kNumberMessages); i++) {
      char msg[64];
      size_t len = snprintf(msg, sizeof(msg), "msg-%u-%04u", pid, i);

      // Send message.
      if (!mq.send(msg, len, timeout)) {
        fprintf(stderr, "Error sending message [%s].\n", msg);
        return -1;
      }

      printf("Sent message: [%s].\n", msg);
    }
  } else {
    // Receive messages.
    do {
      // Receive message.
      char msg[kMaxMessageSize + 1];
      ssize_t ret;
      if ((ret = mq.recv(msg, sizeof(msg), timeout)) <= 0) {
        break;
      }

      printf("Received message: [%.*s].\n", static_cast<int>(ret), msg);
    } while (running);
  }

  return 0;
}

void usage(const char* program)
{
  fprintf(stderr,
          "Usage: %s <queue-name> \"sender\" | \"receiver\" "
          "[<timeout-in-milliseconds>]\n",
          program);
}

void signal_handler(int nsignal)
{
  fprintf(stderr, "Signal received.\n");
  running = false;
}
