#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "util/number.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

int main()
{
  const char* str = "123";
  uint32_t uint32;
  util::number::parse_result res;
  if ((res = util::number::parse(str,
                                 strlen(str),
                                 uint32)) != util::number::kParseSucceeded) {
    fprintf(stderr, "Error parsing %s.\n", str);
    return -1;
  }

  printf("Number: %u.\n", uint32);

  str = "-123";
  int32_t int32;
  if ((res = util::number::parse(str,
                                 strlen(str),
                                 int32)) != util::number::kParseSucceeded) {
    fprintf(stderr, "Error parsing %s.\n", str);
    return -1;
  }

  printf("Number: %d.\n", int32);

  off_t numbers[] = {
    0,
    -1,
    -9,
    -19,
    -99,
    -100,
    1,
    9,
    19,
    20,
    99,
    100
  };

  for (size_t i = 0; i < ARRAY_SIZE(numbers); i++) {
    printf("length(%lld) = %lu.\n",
           numbers[i],
           util::number::length(numbers[i]));
  }

  return 0;
}
