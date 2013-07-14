#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "util/varint.h"
#include "string/buffer.h"

int main()
{
	string::buffer buf;
	for (uint32_t i = 0; i < 9999999; i++) {
		buf.reset();
		if (!util::varint::encode(i, buf)) {
			fprintf(stderr, "Couldn't encode %u.\n", i);
			return -1;
		}

		uint32_t res;
		size_t numlen;
		if (!util::varint::decode(buf.data(), buf.count(), res, numlen)) {
			fprintf(stderr, "Couldn't decode %u.\n", i);
			return -1;
		}

		if ((res != i) || (numlen != util::varint::length(i)) || (numlen != buf.count())) {
			fprintf(stderr, "Error while encoding/decoding.\n");
			return -1;
		}
	}

	for (uint64_t i = 0; i < 9999999; i++) {
		buf.reset();
		if (!util::varint::encode(i, buf)) {
			fprintf(stderr, "Couldn't encode %u.\n", i);
			return -1;
		}

		uint64_t res;
		size_t numlen;
		if (!util::varint::decode(buf.data(), buf.count(), res, numlen)) {
			fprintf(stderr, "Couldn't decode %u.\n", i);
			return -1;
		}

		if ((res != i) || (numlen != util::varint::length(i)) || (numlen != buf.count())) {
			fprintf(stderr, "Error while encoding/decoding.\n");
			return -1;
		}
	}

	return 0;
}
