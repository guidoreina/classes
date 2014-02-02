#ifndef UTIL_BLOCK_H
#define UTIL_BLOCK_H

#include <stdint.h>
#include <endian.h>

namespace util {
	typedef uint32_t blocklen_t;

	class block {
		private:
			struct header {
				blocklen_t size;
			};

			const unsigned char* _M_data;

		public:
			static const size_t kMinSize = sizeof(struct header);
			static const size_t kMaxSize = 1024 * 1024;

			// Constructor.
			block();
			block(const unsigned char* data);

			// Get size.
			blocklen_t size() const;

			// Get data length.
			blocklen_t datalen() const;

			// Get data.
			const unsigned char* data() const;
			const unsigned char* data(blocklen_t& len) const;
	};

	inline block::block()
		: _M_data(NULL)
	{
	}

	inline block::block(const unsigned char* data)
		: _M_data(data)
	{
	}

	inline blocklen_t block::size() const
	{
		const struct header* h = reinterpret_cast<const struct header*>(_M_data);

		switch (sizeof(blocklen_t)) {
			case 1:
				return h->size;
			case 2:
				return be16toh(h->size);
			case 4:
				return be32toh(h->size);
			case 8:
				return be64toh(h->size);
			default:
				return 0;
		}
	}

	inline blocklen_t block::datalen() const
	{
		return size() - sizeof(struct header);
	}

	inline const unsigned char* block::data() const
	{
		return (datalen() == 0) ? reinterpret_cast<const unsigned char*>("") : _M_data + sizeof(struct header);
	}

	inline const unsigned char* block::data(blocklen_t& len) const
	{
		len = datalen();
		return (len == 0) ? reinterpret_cast<const unsigned char*>("") : _M_data + sizeof(struct header);
	}
}

#endif // UTIL_BLOCK_H
