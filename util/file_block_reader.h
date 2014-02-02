#ifndef UTIL_FILE_BLOCK_READER_H
#define UTIL_FILE_BLOCK_READER_H

#include "util/block_reader.h"
#include "fs/file.h"

namespace util {
	class file_block_reader : public block_reader {
		public:
			// Constructor.
			file_block_reader(fs::file& file);

		protected:
			ssize_t read(void* buf, size_t count, int timeout, error& err);

		private:
			fs::file& _M_file;
	};

	inline file_block_reader::file_block_reader(fs::file& file)
		: _M_file(file)
	{
	}
}

#endif // UTIL_FILE_BLOCK_READER_H
