#ifndef UTIL_VARINT_H
#define UTIL_VARINT_H

// Varint:
// https://developers.google.com/protocol-buffers/docs/encoding#varints

#include <stdint.h>
#include "string/buffer.h"

namespace util {
	class varint {
		public:
			// Encode.
			static bool encode(uint64_t n, string::buffer& buf);
			static bool encode(uint64_t n, string::buffer& buf, size_t& len);

			// Decode.
			static bool decode(const char* s, size_t len, uint32_t& n);
			static bool decode(const char* s, size_t len, uint32_t& n, size_t& numlen);

			static bool decode(const char* s, size_t len, uint64_t& n);
			static bool decode(const char* s, size_t len, uint64_t& n, size_t& numlen);

			// Length.
			static size_t length(uint64_t n);
	};

	inline bool varint::encode(uint64_t n, string::buffer& buf)
	{
		size_t len;
		return encode(n, buf, len);
	}

	inline bool varint::decode(const char* s, size_t len, uint32_t& n)
	{
		size_t numlen;
		return decode(s, len, n, numlen);
	}

	inline bool varint::decode(const char* s, size_t len, uint64_t& n)
	{
		size_t numlen;
		return decode(s, len, n, numlen);
	}

	inline size_t varint::length(uint64_t n)
	{
		size_t l = 1;
		while (n >= 0x80) {
			n >>= 7;
			l++;
		}

		return l;
	}
}

#endif // UTIL_VARINT_H
