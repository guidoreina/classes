#include <stdlib.h>
#include "util/number.h"
#include "macros/macros.h"

size_t util::number::length(off_t number)
{
  size_t len = 0;

  // If the number is negative...
  if (number < 0) {
    number *= -1;
    len++;
  }

  do {
    len++;
  } while ((number /= 10) > 0);

  return len;
}

util::number::parse_result util::number::parse(const void* buf,
                                               size_t len,
                                               int64_t& n,
                                               int64_t min,
                                               int64_t max)
{
  if (len == 0) {
    return kParseError;
  }

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buf);
  const uint8_t* end = ptr + len;

  off_t sign;

  if (*ptr == '-') {
    if (len == 1) {
      return kParseError;
    }

    sign = -1;

    ptr++;
  } else if (*ptr == '+') {
    if (len == 1) {
      return kParseError;
    }

    sign = 1;

    ptr++;
  } else {
    sign = 1;
  }

  n = 0;

  while (ptr < end) {
    uint8_t c = *ptr++;
    if (!IS_DIGIT(c)) {
      return kParseError;
    }

    int64_t tmp = (n * 10) + (c - '0');

    // Overflow?
    if (tmp < n) {
      return kParseError;
    }

    n = tmp;
  }

  n *= sign;

  if (n < min) {
    return kParseUnderflow;
  } else if (n > max) {
    return kParseOverflow;
  }

  return kParseSucceeded;
}

util::number::parse_result util::number::parse(const void* buf,
                                               size_t len,
                                               uint64_t& n,
                                               uint64_t min,
                                               uint64_t max)
{
  if (len == 0) {
    return kParseError;
  }

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buf);
  const uint8_t* end = ptr + len;

  n = 0;

  while (ptr < end) {
    uint8_t c = *ptr++;
    if (!IS_DIGIT(c)) {
      return kParseError;
    }

    uint64_t tmp = (n * 10) + (c - '0');

    // Overflow?
    if (tmp < n) {
      return kParseError;
    }

    n = tmp;
  }

  if (n < min) {
    return kParseUnderflow;
  } else if (n > max) {
    return kParseOverflow;
  }

  return kParseSucceeded;
}
