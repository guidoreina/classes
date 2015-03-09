#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include "util/min_priority_queue.h"

static const size_t kMaxKeys = 1000;

static bool test_with_integers();
static bool test_with_strings();

int main()
{
  printf("Testing with integers...\n");
  test_with_integers();

  printf("\nTesting with strings...\n");
  test_with_strings();

  return 0;
}

bool test_with_integers()
{
  util::min_priority_queue<int> pq;

  // Insert keys.
  for (size_t i = 0; i < kMaxKeys; i++) {
    int n = rand();
    if (!pq.push(n)) {
      fprintf(stderr, "Couldn't insert %d.\n", n);
      return false;
    }
  }

  printf("# elements = %lu.\n", pq.size());
  printf("Empty? %s.\n", pq.empty() ? "yes" : "no");

  int last = 0;
  while (!pq.empty()) {
    const int* n = pq.top();
    if (last > *n) {
      fprintf(stderr,
              "Wrong order (current element: %d, previous: %d).\n",
              *n,
              last);

      return false;
    }

    last = *n;

    pq.pop();
  }

  printf("# elements = %lu.\n", pq.size());
  printf("Empty? %s.\n", pq.empty() ? "yes" : "no");

  return true;
}

bool test_with_strings()
{
  util::min_priority_queue<std::string> pq;

  // Insert keys.
  for (size_t i = 0; i < kMaxKeys; i++) {
    char s[64];
    snprintf(s, sizeof(s), "%012d", rand());

    if (!pq.push(s)) {
      fprintf(stderr, "Couldn't insert %s.\n", s);
      return false;
    }
  }

  printf("# elements = %lu.\n", pq.size());
  printf("Empty? %s.\n", pq.empty() ? "yes" : "no");

  int last = 0;
  while (!pq.empty()) {
    const std::string* s = pq.top();
    int n = atoi(s->c_str());

    if (last > n) {
      fprintf(stderr,
              "Wrong order (current element: %d, previous: %d).\n",
              n,
              last);

      return false;
    }

    last = n;

    pq.pop();
  }

  printf("# elements = %lu.\n", pq.size());
  printf("Empty? %s.\n", pq.empty() ? "yes" : "no");

  // Insert keys.
  for (size_t i = 0; i < kMaxKeys; i++) {
    char s[64];
    snprintf(s, sizeof(s), "%012d", rand());

    if (!pq.push(s)) {
      fprintf(stderr, "Couldn't insert %s.\n", s);
      return false;
    }
  }

  pq.clear();

  return true;
}
