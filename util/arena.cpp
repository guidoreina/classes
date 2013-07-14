#include <assert.h>
#include "util/arena.h"
#include "macros/macros.h"

util::arena::~arena()
{
	while (_M_head) {
		block* next = _M_head->next;
		free(_M_head);
		_M_head = next;
	}
}

void* util::arena::allocate(size_t size)
{
	assert(size > 0);

	// Align.
	size_t mod;
	if ((mod = size % sizeof(void*)) != 0) {
		size += sizeof(void*) - mod;
	}

	if (_M_used + size > kDataSize) {
		if (!create_block(MAX(kDataSize, size))) {
			return NULL;
		}
	}

	char* p = _M_head->data + _M_used;
	_M_used += size;
	_M_count += size;

	return p;
}

bool util::arena::create_block(size_t data_size)
{
	block* b;
	if ((b = reinterpret_cast<block*>(malloc(sizeof(block*) + data_size))) == NULL) {
		return false;
	}

	b->next = _M_head;
	_M_head = b;

	_M_used = 0;

	return true;
}
