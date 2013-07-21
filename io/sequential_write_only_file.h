#ifndef IO_SEQUENTIAL_WRITE_ONLY_FILE_H
#define IO_SEQUENTIAL_WRITE_ONLY_FILE_H

#include "io/write_only_file.h"

namespace io {
	class sequential_write_only_file : public write_only_file {
		public:
			// Constructor.
			sequential_write_only_file();

			// Destructor.
			~sequential_write_only_file();

			// Open.
			bool open(const char* filename);

			// Close.
			bool close();

			// Write.
			bool write(const void* buf, size_t count);

		protected:
			static const uint64_t kFileIncrement = 4 * 1024 * 1024;

			uint64_t _M_filesize;
	};

	inline sequential_write_only_file::sequential_write_only_file()
	{
		_M_fd = -1;
	}

	inline sequential_write_only_file::~sequential_write_only_file()
	{
		if (_M_fd != -1) {
			close();
		}
	}
}

#endif // IO_SEQUENTIAL_WRITE_ONLY_FILE_H
