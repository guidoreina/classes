#ifndef FILE_H
#define FILE_H

#include <sys/types.h>
#include <fcntl.h>
#include <sys/uio.h>
#include "string/buffer.h"

namespace fs {
	class file {
		public:
			// Constructor.
			file();
			file(int fd);

			// Open file.
			bool open(const char* pathname, int flags);
			bool open(const char* pathname, int flags, mode_t mode);

			// Close file.
			bool close();

			// Read.
			ssize_t read(void* buf, size_t count);

			// Read at a given offset.
			ssize_t pread(void* buf, size_t count, off_t offset);

			// Read into multiple buffers.
			ssize_t readv(const struct iovec* iov, unsigned iovcnt);

			// Read file.
			static bool read_all(const char* pathname, string::buffer& buf, off_t max = 1024 * 1024);

			// Write.
			ssize_t write(const void* buf, size_t count);

			// Write at a given offset.
			ssize_t pwrite(const void* buf, size_t count, off_t offset);

			// Write from multiple buffers.
			ssize_t writev(const struct iovec* iov, unsigned iovcnt);

			// Seek.
			off_t seek(off_t offset, int whence);

			// Get offset.
			off_t offset() const;

			// Truncate file.
			bool truncate(off_t length);

			// Get file descriptor.
			int fd() const;

			// Set file descriptor.
			void fd(int descriptor);

		protected:
			int _M_fd;
	};

	inline file::file()
	{
	}

	inline file::file(int fd)
	{
		_M_fd = fd;
	}

	inline int file::fd() const
	{
		return _M_fd;
	}

	inline void file::fd(int descriptor)
	{
		_M_fd = descriptor;
	}
}

#endif // FILE_H
