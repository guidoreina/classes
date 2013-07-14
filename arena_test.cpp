#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "util/arena.h"
#include "util/concurrent/arena.h"

static void test_multi_threaded();
static void* allocator(void* arg);
static void test_single_thread();

static const unsigned kNumberAllocators = 40;

int main(int argc, char** argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <test-number>\n", argv[0]);
		fprintf(stderr, "\t0: Test allocator with %u threads.\n", kNumberAllocators);
		fprintf(stderr, "\t1: Test allocator with 1 thread.\n");
		return -1;
	}

	switch (atoi(argv[1])) {
		case 0:
			test_multi_threaded();
			break;
		case 1:
			test_single_thread();
			break;
		default:
			fprintf(stderr, "Wrong test number %s.\n", argv[1]);
			return -1;
	}

	return 0;
}

void test_multi_threaded()
{
	util::concurrent::arena arena;

	// Create allocators.
	pthread_t allocators[kNumberAllocators];
	for (unsigned i = 0; i < kNumberAllocators; i++) {
		if (pthread_create(&allocators[i], NULL, allocator, &arena) != 0) {
			fprintf(stderr, "Couldn't create allocator %u.\n", i);
			return;
		}
	}

	// Wait for allocators.
	for (unsigned i = 0; i < kNumberAllocators; i++) {
		pthread_join(allocators[i], NULL);
	}

	printf("Allocated %u bytes.\n", arena.count());
}

void* allocator(void* arg)
{
	util::concurrent::arena* arena = reinterpret_cast<util::concurrent::arena*>(arg);

	for (size_t i = 1; i <= 8 * 1024; i++) {
		if (!arena->allocate(i)) {
			fprintf(stderr, "Couldn't allocate %u bytes.\n", i);
			return NULL;
		}
	}

	return NULL;
}

void test_single_thread()
{
	util::arena arena;

	for (size_t i = 1; i <= 8 * 1024; i++) {
		if (!arena.allocate(i)) {
			fprintf(stderr, "Couldn't allocate %u bytes.\n", i);
			return;
		}
	}

	printf("Allocated %u bytes.\n", arena.count());
}
