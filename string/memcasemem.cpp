#include <stdlib.h>
#include "string/memcasemem.h"

void* string::memcasemem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen)
{
	if (needlelen == 0) {
		return const_cast<void*>(haystack);
	}

	if (haystacklen < needlelen) {
		return NULL;
	}

	const char* end = reinterpret_cast<const char*>(haystack) + haystacklen - needlelen;
	const char* n = reinterpret_cast<const char*>(needle);

	for (const char* ptr = reinterpret_cast<const char*>(haystack); ptr <= end; ptr++) {
		size_t i;
		for (i = 0; i < needlelen; i++) {
			unsigned char c1 = ptr[i];
			c1 = ((c1 >= 'A') && (c1 <= 'Z')) ? (c1 | 0x20) : c1;

			unsigned char c2 = n[i];
			c2 = ((c2 >= 'A') && (c2 <= 'Z')) ? (c2 | 0x20) : c2;

			if (c1 != c2) {
				break;
			}
		}

		if (i == needlelen) {
			return reinterpret_cast<void*>(const_cast<char*>(ptr));
		}
	}

	return NULL;
}
