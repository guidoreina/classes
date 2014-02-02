#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <endian.h>
#include <stdio.h>
#include "util/file_block_reader.h"
#include "fs/file.h"
#include "string/buffer.h"

int main()
{
	static const char* kFilename = "blocks.bin";

	fs::file f;
	if (!f.open(kFilename, O_CREAT | O_TRUNC | O_RDWR, 0644)) {
		fprintf(stderr, "Couldn't open file %s for reading/writing.\n", kFilename);
		return -1;
	}

	static const size_t kMinSize = util::block::kMinSize;
	static const size_t kMaxSize = 16 * 1024;
	static const size_t kNumberBlocks = 8192;

	unsigned char buf[kMaxSize];

	for (size_t i = kMinSize; i <= kMaxSize; i++) {
		if ((i % 100) == 0) {
			printf("Block size: %lu.\n", i);
		}

		// Write blocks.
		for (size_t j = 0; j < kNumberBlocks; j++) {
			// Prepare block.
			switch (sizeof(util::blocklen_t)) {
				case 1:
					*buf = i;
					break;
				case 2:
					{
						uint16_t n;
						n = htobe16(i);
						memcpy(buf, &n, sizeof(uint16_t));
					}

					break;
				case 4:
					{
						uint32_t n;
						n = htobe32(i);
						memcpy(buf, &n, sizeof(uint32_t));
					}

					break;
				case 8:
					{
						uint64_t n;
						n = htobe64(i);
						memcpy(buf, &n, sizeof(uint64_t));
					}

					break;
				default:
					fprintf(stderr, "Invalid blocklen_t size (%lu).\n", sizeof(util::blocklen_t));

					f.close();
					unlink(kFilename);

					return -1;
			}

			for (size_t k = util::block::kMinSize; k < i; k++) {
				buf[k] = static_cast<unsigned char>(j % 255);
			}

			// Write block.
			if (!f.write(buf, i)) {
				fprintf(stderr, "Error writing record of %lu bytes.\n", i);

				f.close();
				unlink(kFilename);

				return -1;
			}
		}

		// Rewind file.
		f.seek(0, SEEK_SET);

		// Read blocks.
		util::file_block_reader block_reader(f);
		string::buffer b;

		for (size_t j = 0; j < kNumberBlocks; j++) {
			// Get next block.
			util::block block;
			b.reset();
			if (!block_reader.next(block, b, -1)) {
				fprintf(stderr, "Couldn't read block of %lu bytes.\n", i);

				f.close();
				unlink(kFilename);

				return -1;
			}

			if (i <= util::block_reader::kBufferSize) {
				if (b.count() > 0) {
					fprintf(stderr, "Buffer contains data (%lu bytes), it should be empty.\n", b.count());

					f.close();
					unlink(kFilename);

					return -1;
				}
			} else {
				if (b.count() != i) {
					fprintf(stderr, "Buffer length %lu doesn't match block length: %lu.\n", b.count(), i);

					f.close();
					unlink(kFilename);

					return -1;
				}
			}

			// Check block.
			if (block.size() != i) {
				fprintf(stderr, "Block has wrong size (%u, expected: %lu).\n", block.size(), i);

				f.close();
				unlink(kFilename);

				return -1;
			}

			util::blocklen_t len;
			const unsigned char* data = block.data(len);
			for (size_t k = 0; k < len; k++) {
				if (data[k] != static_cast<unsigned char>(j % 255)) {
					fprintf(stderr, "Invalid data.\n");

					f.close();
					unlink(kFilename);

					return -1;
				}
			}
		}

		if (!f.truncate(0)) {
			fprintf(stderr, "Couldn't truncate file %s.\n", kFilename);

			f.close();
			unlink(kFilename);

			return -1;
		}

		// Rewind file.
		f.seek(0, SEEK_SET);
	}

	f.close();
	unlink(kFilename);

	return 0;
}
