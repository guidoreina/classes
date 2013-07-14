#include <stdlib.h>
#include "util/varint.h"

bool util::varint::encode(uint64_t n, string::buffer& buf, size_t& len)
{
	if (!buf.allocate(9)) {
		return false;
	}

	char* data = buf.end();
	size_t l = 1;

	while (n >= 0x80) {
		*data++ = (n & 0x7f) | 0x80;
		n >>= 7;
		l++;
	}

	*data = n;

	buf.increment_count(l);
	len = l;

	return true;
}

bool util::varint::decode(const char* s, size_t len, uint32_t& n, size_t& numlen)
{
	const char* end = s + len;

	uint32_t res = 0;
	size_t l = 0;

	while (s < end) {
		uint8_t c = *(reinterpret_cast<const uint8_t*>(s++));

		if (c < 0x80) {
			res |= (c << (l * 7));
			n = res;
			numlen = ++l;

			return true;
		}

		res |= ((c & 0x7f) << (l * 7));

		// Varint too long?
		if (++l == 5) {
			return false;
		}
	}

	return false;
}

bool util::varint::decode(const char* s, size_t len, uint64_t& n, size_t& numlen)
{
	const char* end = s + len;

	uint64_t res = 0;
	size_t l = 0;

	while (s < end) {
		uint8_t c = *(reinterpret_cast<const uint8_t*>(s++));

		if (c < 0x80) {
			res |= (c << (l * 7));
			n = res;
			numlen = ++l;

			return true;
		}

		res |= ((c & 0x7f) << (l * 7));

		// Varint too long?
		if (++l == 9) {
			return false;
		}
	}

	return false;
}
