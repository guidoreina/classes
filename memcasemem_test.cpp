#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "string/memcasemem.h"

int main()
{
	const char* s = "This is a simple test.";

	if (!string::memcasemem(s, strlen(s), "SIMPLE", 6)) {
		fprintf(stderr, "Substring not found.\n");
		return -1;
	}

	return 0;
}
