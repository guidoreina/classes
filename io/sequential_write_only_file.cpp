#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "io/sequential_write_only_file.h"

bool io::sequential_write_only_file::open(const char* filename)
{
	if ((_M_fd = ::open(filename, O_CREAT | O_WRONLY, 0644)) < 0) {
		return false;
	}

	off_t offset;
	if ((offset = lseek(_M_fd, 0, SEEK_END)) < 0) {
		return false;
	}

	_M_filesize = offset;
	_M_offset = offset;

	return true;
}

bool io::sequential_write_only_file::close()
{
	if (_M_filesize != _M_offset) {
		if (ftruncate(_M_fd, _M_offset) < 0) {
			return false;
		}

		_M_filesize = _M_offset;
	}

	if (::close(_M_fd) < 0) {
		return false;
	}

	_M_fd = -1;

	return true;
}

bool io::sequential_write_only_file::write(const void* buf, size_t count)
{
	// If we have to increment the size of the file...
	uint64_t filesize;
	if ((filesize = _M_offset + count) > _M_filesize) {
		do {
			_M_filesize += kFileIncrement;
		} while (filesize > _M_filesize);

		if (ftruncate(_M_fd, _M_filesize) < 0) {
			return false;
		}
	}

	const char* b = reinterpret_cast<const char*>(buf);

	while (count > 0) {
		ssize_t ret;
		if ((ret = ::write(_M_fd, b, count)) < 0) {
			if ((errno != EINTR) && (errno != EAGAIN)) {
				return false;
			}
		} else if (ret > 0) {
			b += ret;
			count -= ret;

			_M_offset += ret;
		}
	}

	return true;
}
