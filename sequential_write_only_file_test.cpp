#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "io/sequential_write_only_file.h"

int main()
{
	static const char* kFilename = "test.dat";

	// Remove file (if exists).
	unlink(kFilename);

	// Open file for writing.
	io::sequential_write_only_file file;
	if (!file.open(kFilename)) {
		fprintf(stderr, "Couldn't open file %s for writing.\n", kFilename);
		return -1;
	}

	// Initialize buffer.
	char buf[8 * 1024];
	memset(buf, '0', sizeof(buf));

	// Write file.
	static const uint64_t kFilesize = 1ULL << 31;
	uint64_t filesize = 0;
	do {
		if (!file.write(buf, sizeof(buf))) {
			fprintf(stderr, "Error writing to file %s.\n", kFilename);
			return -1;
		}

		filesize += sizeof(buf);
	} while (filesize < kFilesize);

	return 0;
}
