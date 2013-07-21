#ifndef IO_WRITE_ONLY_FILE_H
#define IO_WRITE_ONLY_FILE_H

#include <stdint.h>

namespace io {
	class write_only_file {
		public:
			// Open.
			virtual bool open(const char* filename) = 0;

			// Close.
			virtual bool close() = 0;

			// Write.
			virtual bool write(const void* buf, size_t count) = 0;

		protected:
			int _M_fd;
			uint64_t _M_offset;
	};
}

#endif // IO_WRITE_ONLY_FILE_H
