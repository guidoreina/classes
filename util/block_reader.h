#ifndef UTIL_BLOCK_READER_H
#define UTIL_BLOCK_READER_H

#include "util/block.h"
#include "string/buffer.h"

namespace util {
	class block_reader {
		public:
			static const size_t kBufferSize = 4 * 1024;

			// Constructor.
			block_reader();

			// Get next block.
			enum error {
				kNoMemory,
				kBlockTooBig,
				kReadError,
				kEndOfFile,
				kTimeout
			};

			bool next(block& block, string::buffer& buf, int timeout);
			bool next(block& block, string::buffer& buf, int timeout, error& err);

		protected:
			unsigned char _M_buf[kBufferSize];
			size_t _M_end;
			size_t _M_block;

			virtual ssize_t read(void* buf, size_t count, int timeout, error& err) = 0;
	};

	inline block_reader::block_reader()
		: _M_end(0),
		  _M_block(0)
	{
	}

	inline bool block_reader::next(block& block, string::buffer& buf, int timeout)
	{
		error err;
		return next(block, buf, timeout, err);
	}
}

#endif // UTIL_BLOCK_READER_H
