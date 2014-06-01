#include <stdlib.h>
#include <stdio.h>
#include "util/fifo.h"

static const size_t kMaxElements = 1000000;

int main()
{
  util::fifo<size_t> fifo;

  for (unsigned i = 0; i < 2; i++) {
    // Insert elements.
    for (size_t j = 0; j < kMaxElements; j++) {
      if (!fifo.push(j)) {
        fprintf(stderr, "Error inserting %lu.\n", j);
        return -1;
      }
    }

    // Check last element.
    const size_t* elem;
    if ((elem = fifo.back()) == NULL) {
      fprintf(stderr, "Couldn't get back of the queue.\n");
      return -1;
    }

    if (*elem != kMaxElements - 1) {
      fprintf(stderr,
              "Wrong element %lu, expected %lu.\n",
              *elem,
              kMaxElements - 1);

      return -1;
    }

    size_t count = 0;
    while ((elem = fifo.front()) != NULL) {
      if (*elem != count) {
        fprintf(stderr, "Wrong element %lu, expected %lu.\n", *elem, count);
        return -1;
      }

      fifo.pop();
      count++;
    }

    if (count != kMaxElements) {
      fprintf(stderr,
              "Popped %lu elements, expected %lu.\n",
              count,
              kMaxElements);

      return -1;
    }

    if (fifo.count() != 0) {
      fprintf(stderr,
              "Invalid number of elements %lu, expected %d.\n",
              fifo.count(),
              0);

      return -1;
    }

    if (!fifo.empty()) {
      fprintf(stderr, "Queue should be empty.\n");
      return -1;
    }
  }

  return 0;
}
