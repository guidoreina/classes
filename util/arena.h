#ifndef UTIL_ARENA_H
#define UTIL_ARENA_H

// Simple allocator.

#include <stdlib.h>

namespace util {
	class arena {
		public:
			// Constructor.
			arena();

			// Destructor.
			~arena();

			// Allocate.
			void* allocate(size_t size);

			// Get count.
			size_t count() const;

		private:
			static const size_t kBlockSize = 4 * 1024;
			static const size_t kDataSize = kBlockSize - sizeof(void*);

			struct block {
				struct block* next;
				char data[1];
			};

			block* _M_head;
			size_t _M_used;
			size_t _M_count;

			// Create block.
			bool create_block(size_t data_size);
	};

	inline arena::arena()
	: _M_head(NULL),
	_M_used(kDataSize),
	_M_count(0)
	{
	}

	inline size_t arena::count() const
	{
		return _M_count;
	}
}

#endif // UTIL_ARENA_H
