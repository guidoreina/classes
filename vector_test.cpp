#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include "util/vector.h"

static const size_t kMaxElements = 1000;

static bool test_with_integers();
static bool test_with_strings();

int main()
{
  printf("Testing with integers...\n");
  test_with_integers();

  printf("Testing with strings...\n");
  test_with_strings();

  return 0;
}

bool test_with_integers()
{
  util::vector<size_t> v;

  // Insert elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    if (!v.push_back(i)) {
      fprintf(stderr, "Error inserting %lu.\n", i);
      return false;
    }
  }

  // Check elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    const size_t* n = v.at(i);
    if (*n != i) {
      fprintf(stderr, "Invalid number %lu, expected %lu.\n", *n, i);
      return false;
    }
  }

  printf("# elements = %lu.\n", v.size());
  printf("Empty? %s.\n", v.empty() ? "yes" : "no");

  // Clear vector.
  v.clear();

  // Insert elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    size_t n = i;
    if (!v.emplace_back(util::move(n))) {
      fprintf(stderr, "Error inserting %lu.\n", i);
      return false;
    }
  }

  // Check elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    const size_t* n = v.at(i);
    if (*n != i) {
      fprintf(stderr, "Invalid number %lu, expected %lu.\n", *n, i);
      return false;
    }
  }

  // Replace elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    size_t n = i + 1;
    if (!v.emplace_at(i, util::move(n))) {
      fprintf(stderr, "Error replacing %lu with %lu.\n", i, i + 1);
      return false;
    }
  }

  // Check elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    const size_t* n = v.at(i);
    if (*n != i + 1) {
      fprintf(stderr, "Invalid number %lu, expected %lu.\n", *n, i + 1);
      return false;
    }
  }

  // Remove all elements.
  size_t i = 0;
  while (!v.empty()) {
    v.pop_back();
    i++;
  }

  if (i != kMaxElements) {
    fprintf(stderr, "Erased %lu elements, expected %lu.\n", i, kMaxElements);
    return false;
  }

  return true;
}

bool test_with_strings()
{
  util::vector<std::string> v;

  // Insert elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    char s[64];
    snprintf(s, sizeof(s), "%012lu", i);

    if (!v.push_back(s)) {
      fprintf(stderr, "Error inserting %s.\n", s);
      return false;
    }
  }

  // Check elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    const std::string* s = v.at(i);
    size_t n = atoi(s->c_str());

    if (n != i) {
      fprintf(stderr, "Invalid number %lu, expected %lu.\n", n, i);
      return false;
    }
  }

  printf("# elements = %lu.\n", v.size());
  printf("Empty? %s.\n", v.empty() ? "yes" : "no");

  // Clear vector.
  v.clear();

  // Insert elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    char s[64];
    snprintf(s, sizeof(s), "%012lu", i);
    std::string str(s);

    if (!v.emplace_back(util::move(str))) {
      fprintf(stderr, "Error inserting %s.\n", s);
      return false;
    }
  }

  // Check elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    const std::string* s = v.at(i);
    size_t n = atoi(s->c_str());

    if (n != i) {
      fprintf(stderr, "Invalid number %lu, expected %lu.\n", n, i);
      return false;
    }
  }

  // Replace elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    char s[64];
    snprintf(s, sizeof(s), "%012lu", i + 1);
    std::string str(s);

    if (!v.emplace_at(i, util::move(str))) {
      fprintf(stderr, "Error replacing %lu with %lu.\n", i, i + 1);
      return false;
    }
  }

  // Check elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    const std::string* s = v.at(i);
    size_t n = atoi(s->c_str());

    if (n != i + 1) {
      fprintf(stderr, "Invalid number %lu, expected %lu.\n", n, i + 1);
      return false;
    }
  }

  // Remove all elements.
  size_t i = 0;
  while (!v.empty()) {
    v.pop_back();
    i++;
  }

  if (i != kMaxElements) {
    fprintf(stderr, "Erased %lu elements, expected %lu.\n", i, kMaxElements);
    return false;
  }

  // Insert elements.
  for (size_t i = 0; i < kMaxElements; i++) {
    char s[64];
    snprintf(s, sizeof(s), "%012lu", i);
    std::string str(s);

    if (!v.emplace_back(util::move(str))) {
      fprintf(stderr, "Error inserting %s.\n", s);
      return false;
    }
  }

  util::vector<std::string> v2;
  v2 = util::move(v);

  printf("[v] # elements = %lu.\n", v.size());
  printf("[v] Empty? %s.\n", v.empty() ? "yes" : "no");

  printf("[v2] # elements = %lu.\n", v2.size());
  printf("[v2] Empty? %s.\n", v2.empty() ? "yes" : "no");

  return true;
}
