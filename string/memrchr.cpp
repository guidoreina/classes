#include <stdlib.h>
#include <string.h>
#include "string/memrchr.h"

const void* string::memrchr(const void* s, int c, size_t n)
{
#if HAVE_MEMRCHR
	return ::memrchr(s, c, n);
#else
	if (n == 0) {
		return NULL;
	}

	const char* start = reinterpret_cast<const char*>(s);
	const char* end = start + n - 1;

	while (end >= start) {
		if (*end == c) {
			return end;
		}

		end--;
	}

	return NULL;
#endif
}
