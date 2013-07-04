#include <stdlib.h>
#include <stdio.h>
#include "string/buffer.h"

int main()
{
	string::buffer buf;
	if (!buf.append("Test", 4)) {
		fprintf(stderr, "Couldn't insert string.\n");
		return -1;
	}

	if (!buf.append('s')) {
		fprintf(stderr, "Couldn't insert character.\n");
		return -1;
	}

	printf("%.*s\n", buf.count(), buf.data());

	return 0;
}
