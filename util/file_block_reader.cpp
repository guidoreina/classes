#include <stdlib.h>
#include "util/file_block_reader.h"

ssize_t util::file_block_reader::read(void* buf, size_t count, int timeout, error& err)
{
	ssize_t ret;
	switch ((ret = _M_file.read(buf, count))) {
		case -1:
			err = kReadError;
			break;
		case 0:
			err = kEndOfFile;
			break;
		default:
			;
	}

	return ret;
}
