#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "string/memrchr.h"

int main()
{
	const char* s = "This is a simple test.";

	if (!string::memrchr(s, 'h', strlen(s))) {
		fprintf(stderr, "Character not found.\n");
		return -1;
	}

	return 0;
}
