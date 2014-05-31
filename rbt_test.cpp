// Compile with:
// g++ -g -o rbt_test rbt_test.cpp -I . -std=c++11 -Wall --pedantic -DHAVE_FREE_LIST

#include <stdlib.h>
#include <stdio.h>
#include "util/red_black_tree.h"

static const size_t kMaxElements = 1000000;

static bool valid(const util::red_black_tree<size_t, size_t>& rbt);

int main()
{
  util::red_black_tree<size_t, size_t> rbt;

  // Insert elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    if (!rbt.insert(i, i + 1)) {
      fprintf(stderr, "Error inserting in red-black-tree %lu.\n", i);
      return -1;
    }
  }

  // If the red-black-tree is not valid...
  if (!valid(rbt)) {
    return -1;
  }

  // Erase elements.
  for (size_t i = kMaxElements; i > 0; i--) {
    if (!rbt.erase(i - 1)) {
      fprintf(stderr, "Error erasing %lu from red-black-tree.\n", i - 1);
      return -1;
    }
  }

  // Check number of elements.
  if (rbt.count() != 0) {
    fprintf(stderr,
            "Invalid number of elements %lu, expected %d.\n",
            rbt.count(),
            0);

    return -1;
  }

  {
    util::red_black_tree<size_t, size_t>::const_iterator it;
    if (rbt.begin(it)) {
      fprintf(stderr, "Could get first element, not expected.\n");
      return -1;
    }
  }

  // Insert elements.
  for (size_t i = kMaxElements; i > 0; i--) {
    if (!rbt.insert(i - 1, i)) {
      fprintf(stderr, "Error inserting in red-black-tree %lu.\n", i - 1);
      return -1;
    }
  }

  // If the red-black-tree is not valid...
  if (!valid(rbt)) {
    return -1;
  }

  // Erase elements.
  util::red_black_tree<size_t, size_t>::iterator it;
  if (!rbt.begin(it)) {
    fprintf(stderr, "Couldn't get first element.\n");
    return -1;
  }

  size_t i = 0;
  while (rbt.erase(it)) {
    i++;
  }

  if (++i != kMaxElements) {
    fprintf(stderr, "Deleted %lu elements, expected %lu.\n", i, kMaxElements);
    return -1;
  }

  if (rbt.count() != 0) {
    fprintf(stderr,
            "Invalid number of elements %lu, expected %d.\n",
            rbt.count(),
            0);

    return -1;
  }

  return 0;
}

bool valid(const util::red_black_tree<size_t, size_t>& rbt)
{
  // Find elements.
  util::red_black_tree<size_t, size_t>::const_iterator it;
  for (size_t i = 0; i < kMaxElements; i++) {
    if (!rbt.find(i, it)) {
      fprintf(stderr, "Element %lu not found.\n", i);
      return false;
    }

    if (*it.first != i) {
      fprintf(stderr, "Wrong key %lu, expected %lu.\n", *it.first, i);
      return false;
    }

    if (*it.second != i + 1) {
      fprintf(stderr, "Wrong value %lu, expected %lu.\n", *it.second, i + 1);
      return false;
    }
  }

  // Forward iteration.
  if (!rbt.begin(it)) {
    fprintf(stderr, "Couldn't get first element.\n");
    return false;
  }

  size_t i = 0;
  do {
    if (*it.first != i) {
      fprintf(stderr, "Wrong key %lu, expected %lu.\n", *it.first, i);
      return false;
    }

    if (*it.second != i + 1) {
      fprintf(stderr, "Wrong value %lu, expected %lu.\n", *it.second, i + 1);
      return false;
    }

    i++;
  } while (rbt.next(it));

  if (i != kMaxElements) {
    fprintf(stderr, "Found only %lu elements, expected %lu.\n", i, kMaxElements);
    return false;
  }

  // Reverse iteration.
  if (!rbt.end(it)) {
    fprintf(stderr, "Couldn't get last element.\n");
    return false;
  }

  i = kMaxElements;
  do {
    if (*it.first != i - 1) {
      fprintf(stderr, "Wrong key %lu, expected %lu.\n", *it.first, i - 1);
      return false;
    }

    if (*it.second != i) {
      fprintf(stderr, "Wrong value %lu, expected %lu.\n", *it.second, i);
      return false;
    }

    i--;
  } while (rbt.prev(it));

  if (i != 0) {
    fprintf(stderr,
            "Found only %lu elements, expected %lu.\n",
            kMaxElements - i,
            kMaxElements);

    return false;
  }

  // Check number of elements.
  if (rbt.count() != kMaxElements) {
    fprintf(stderr,
            "Invalid number of elements %lu, expected %lu.\n",
            rbt.count(),
            kMaxElements);

    return false;
  }

  return true;
}
