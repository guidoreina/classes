#include <stdlib.h>
#include "util/block_reader.h"
#include "macros/macros.h"

bool util::block_reader::next(block& block, string::buffer& buf, int timeout, error& err)
{
	// While we don't have a full header...
	size_t count;
	while ((count = _M_end - _M_block) < block::kMinSize) {
		if (_M_block > 0) {
			if (count > 0) {
				memmove(_M_buf, _M_buf + _M_block, count);
				_M_end = count;
			} else {
				_M_end = 0;
			}

			_M_block = 0;
		}

		ssize_t ret;
		if ((ret = read(_M_buf + _M_end, sizeof(_M_buf) - _M_end, timeout, err)) <= 0) {
			return false;
		}

		_M_end += ret;
	}

	blocklen_t block_size = util::block(_M_buf + _M_block).size();

	// If the block is too big...
	if (block_size > block::kMaxSize) {
		err = kBlockTooBig;
		return false;
	}

	// If we have a full block...
	if (block_size <= count) {
		block = util::block(_M_buf + _M_block);

		_M_block += block_size;
		return true;
	}

	size_t left = block_size - count;

	unsigned char* begin;

	// If the block doesn't fit in the buffer...
	if (block_size > sizeof(_M_buf)) {
		if (!buf.allocate(block_size)) {
			err = kNoMemory;
			return false;
		}

		begin = reinterpret_cast<unsigned char*>(buf.end());

		memcpy(begin, _M_buf + _M_block, count);
		unsigned char* end = begin + count;

		// Read.
		ssize_t ret;
		size_t n;
		do {
			if ((ret = read(_M_buf, sizeof(_M_buf), timeout, err)) <= 0) {
				return false;
			}

			n = MIN(left, static_cast<size_t>(ret));

			memcpy(end, _M_buf, n);
			end += n;
		} while ((left -= n) > 0);

		_M_block = n;
		_M_end = ret;

		buf.increment_count(block_size);
	} else {
		if (_M_block > 0) {
			memmove(_M_buf, _M_buf + _M_block, count);
			_M_end = count;
		}

		begin = _M_buf;

		// Read.
		ssize_t ret;
		do {
			if ((ret = read(_M_buf + _M_end, sizeof(_M_buf) - _M_end, timeout, err)) <= 0) {
				return false;
			}

			_M_end += ret;
		} while ((left -= MIN(left, static_cast<size_t>(ret))) > 0);

		_M_block = block_size;
	}

	block = util::block(begin);

	return true;
}
