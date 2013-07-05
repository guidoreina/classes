#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

namespace string {
	class buffer {
		public:
			static const size_t kDefaultInitialSize = 64;

			// Constructor.
			buffer(size_t initial_size = kDefaultInitialSize);

			// Destructor.
			virtual ~buffer();

			// Free buffer.
			void free();

			// Reset buffer.
			void reset();

			// Get data.
			const char* data() const;

			char* data();

			// Get end.
			char* end();

			// Get size.
			size_t size() const;

			// Get count.
			size_t count() const;

			// Set count.
			void count(size_t value);

			// Increment count.
			void increment_count(size_t inc);

			// Set initial size.
			void set_initial_size(size_t initial_size);

			// Allocate memory.
			bool allocate(size_t size);

			// Append.
			bool append(char c);
			bool append(const char* string);
			bool append(const char* string, size_t len);

			// Append NUL-terminated string.
			bool append_nul_terminated_string(const char* string, size_t len);

			// Format string.
			bool format(const char* format, ...);
			bool vformat(const char* format, va_list ap);

		protected:
			char* _M_data;
			size_t _M_size;
			size_t _M_used;

			size_t _M_initial_size;
	};

	inline buffer::buffer(size_t initial_size)
	{
		_M_data = NULL;
		_M_size = 0;
		_M_used = 0;

		set_initial_size(initial_size);
	}

	inline buffer::~buffer()
	{
		free();
	}

	inline void buffer::free()
	{
		if (_M_data) {
			::free(_M_data);
			_M_data = NULL;
		}

		_M_size = 0;
		_M_used = 0;
	}

	inline void buffer::reset()
	{
		_M_used = 0;
	}

	inline const char* buffer::data() const
	{
		return _M_data;
	}

	inline char* buffer::data()
	{
		return _M_data;
	}

	inline char* buffer::end()
	{
		return _M_data + _M_used;
	}

	inline size_t buffer::size() const
	{
		return _M_size;
	}

	inline size_t buffer::count() const
	{
		return _M_used;
	}

	inline void buffer::count(size_t value)
	{
		_M_used = value;
	}

	inline void buffer::increment_count(size_t inc)
	{
		_M_used += inc;
	}

	inline void buffer::set_initial_size(size_t initial_size)
	{
		if (initial_size == 0) {
			_M_initial_size = kDefaultInitialSize;
		} else {
			_M_initial_size = initial_size;
		}
	}

	inline bool buffer::append(char c)
	{
		if (!allocate(1)) {
			return false;
		}

		_M_data[_M_used++] = c;

		return true;
	}

	inline bool buffer::append(const char* string)
	{
		if (!string) {
			return true;
		}

		return append(string, strlen(string));
	}

	inline bool buffer::append(const char* string, size_t len)
	{
		if (len == 0) {
			return true;
		}

		if (!allocate(len)) {
			return false;
		}

		memcpy(_M_data + _M_used, string, len);
		_M_used += len;

		return true;
	}

	inline bool buffer::append_nul_terminated_string(const char* string, size_t len)
	{
		if (!allocate(len + 1)) {
			return false;
		}

		memcpy(_M_data + _M_used, string, len);
		_M_used += len;
		_M_data[_M_used++] = 0;

		return true;
	}

	inline bool buffer::format(const char* format, ...)
	{
		va_list ap;
		va_start(ap, format);

		bool ret = vformat(format, ap);

		va_end(ap);

		return ret;
	}
}

#endif // STRING_BUFFER_H
