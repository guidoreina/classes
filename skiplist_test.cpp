#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <sched.h>
#include "util/skiplist.h"
#include "util/concurrent/skiplist.h"
#include "util/concurrent/locks/spinlock.h"

static const unsigned kNumberReaders = 33;
static const unsigned kNumberWriters = 33;
static const unsigned kNumberErasers = 33;
static const unsigned kNumberIterators = 33;
static const unsigned kHighestRandom = 64 * 1024;

struct longcmp {
	int operator()(long x, long y) const
	{
		return x - y;
	}
};

enum state {
	NUMBER_NOT_USED,
	NUMBER_IN_USE,
	NUMBER_INSERTED,
	NUMBER_ERASED
};

static state numbers[kHighestRandom];
static bool running = true;

static util::concurrent::locks::spinlock lock;

static void test_concurrent_skiplist();
static void* reader(void* arg);
static void* writer(void* arg);
static void* eraser(void* arg);
static void* iterator(void* arg);

static void test_concurrent_skiplist_iterators();
static void print_list(const util::concurrent::skiplist<long, longcmp>& list, bool forward);

static void test_single_threaded_skiplist();
static void print_list(const util::skiplist<long, longcmp>& list, bool forward);

int main(int argc, char** argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <test-number>\n", argv[0]);
		fprintf(stderr, "\t0: Test concurrent skip list with:\n");
		fprintf(stderr, "\t\t%u readers\n", kNumberReaders);
		fprintf(stderr, "\t\t%u writers, %u\n", kNumberWriters);
		fprintf(stderr, "\t\t%u erasers\n", kNumberErasers);
		fprintf(stderr, "\t\t%u iterators.\n", kNumberIterators);
		fprintf(stderr, "\t1: Test iterators in concurrent skip list.\n");
		fprintf(stderr, "\t2: Test skip list (one thread).\n");

		return -1;
	}

	switch (atoi(argv[1])) {
		case 0:
			test_concurrent_skiplist();
			break;
		case 1:
			test_concurrent_skiplist_iterators();
			break;
		case 2:
			test_single_threaded_skiplist();
			break;
		default:
			fprintf(stderr, "Wrong test number %s.\n", argv[1]);
			return -1;
	}

	return 0;
}

void test_concurrent_skiplist()
{
	util::concurrent::skiplist<long, longcmp> list;

	// Initialize list.
	if (!list.init()) {
		fprintf(stderr, "Couldn't initialize skip list.\n");
		return;
	}

	// Block signals SIGINT and SIGTERM.
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0) {
		fprintf(stderr, "Error blocking signals.\n");
		return;
	}

	for (unsigned i = 0; i < kHighestRandom; i++) {
		numbers[i] = NUMBER_NOT_USED;
	}

	// Create readers.
	pthread_t readers[kNumberReaders];
	for (unsigned i = 0; i < kNumberReaders; i++) {
		if (pthread_create(&readers[i], NULL, reader, &list) != 0) {
			fprintf(stderr, "Couldn't create reader %u.\n", i);
			return;
		}
	}

	// Create writers.
	pthread_t writers[kNumberWriters];
	for (unsigned i = 0; i < kNumberWriters; i++) {
		if (pthread_create(&writers[i], NULL, writer, &list) != 0) {
			fprintf(stderr, "Couldn't create writer %u.\n", i);
			return;
		}
	}

	// Create erasers.
	pthread_t erasers[kNumberErasers];
	for (unsigned i = 0; i < kNumberErasers; i++) {
		if (pthread_create(&erasers[i], NULL, eraser, &list) != 0) {
			fprintf(stderr, "Couldn't create eraser %u.\n", i);
			return;
		}
	}

	// Create iterators.
	pthread_t iterators[kNumberIterators];
	for (unsigned i = 0; i < kNumberIterators; i++) {
		if (pthread_create(&iterators[i], NULL, iterator, &list) != 0) {
			fprintf(stderr, "Couldn't create iterator %u.\n", i);
			return;
		}
	}

	printf("Send SIGINT (Control + C) or SIGTERM to finish the test...\n");

	// Wait for signal to arrive.
	int sig;
	while (sigwait(&set, &sig) != 0);

	printf("Signal received.\n");

	running = false;

	// Wait for readers.
	for (unsigned i = 0; i < kNumberReaders; i++) {
		pthread_join(readers[i], NULL);
	}

	// Wait for writers.
	for (unsigned i = 0; i < kNumberWriters; i++) {
		pthread_join(writers[i], NULL);
	}

	// Wait for erasers.
	for (unsigned i = 0; i < kNumberErasers; i++) {
		pthread_join(erasers[i], NULL);
	}

	// Wait for iterators.
	for (unsigned i = 0; i < kNumberIterators; i++) {
		pthread_join(iterators[i], NULL);
	}
}

void* reader(void* arg)
{
	util::concurrent::skiplist<long, longcmp> *list = reinterpret_cast<util::concurrent::skiplist<long, longcmp>*>(arg);

	pthread_t thread = pthread_self();

	struct drand48_data data;
	srand48_r(0xc0c0c0c0, &data);

	do {
		long n;
		state state;

		do {
			if (!running) {
				return 0;
			}

			lrand48_r(&data, &n);
			n %= kHighestRandom;

			lock.lock();

			state = numbers[n];
			if ((state == NUMBER_INSERTED) || (state == NUMBER_ERASED)) {
				numbers[n] = NUMBER_IN_USE;
				lock.unlock();

				break;
			}

			lock.unlock();
		} while (true);

		if (state == NUMBER_INSERTED) {
			if (!list->contains(n)) {
				fprintf(stderr, "[%ld] %u not found.\n", thread, n);
			}
		} else {
			if (list->contains(n)) {
				fprintf(stderr, "[%ld] %u found.\n", thread, n);
			}
		}

		lock.lock();
		numbers[n] = state;
		lock.unlock();
	} while (true);

	return NULL;
}

void* writer(void* arg)
{
	util::concurrent::skiplist<long, longcmp> *list = reinterpret_cast<util::concurrent::skiplist<long, longcmp>*>(arg);

	pthread_t thread = pthread_self();

	struct drand48_data data;
	srand48_r(0xc0c0c0c0, &data);

	do {
		long n;
		state state;

		do {
			if (!running) {
				return 0;
			}

			lrand48_r(&data, &n);
			n %= kHighestRandom;

			lock.lock();

			state = numbers[n];
			if ((state == NUMBER_NOT_USED) || (state == NUMBER_ERASED)) {
				numbers[n] = NUMBER_IN_USE;
				lock.unlock();

				break;
			}

			lock.unlock();
		} while (true);

		if (!list->insert(n)) {
			fprintf(stderr, "[%ld] Couldn't insert %u.\n", thread, n);
		}

		lock.lock();
		numbers[n] = NUMBER_INSERTED;
		lock.unlock();
	} while (true);

	return NULL;
}

void* eraser(void* arg)
{
	util::concurrent::skiplist<long, longcmp> *list = reinterpret_cast<util::concurrent::skiplist<long, longcmp>*>(arg);

	pthread_t thread = pthread_self();

	struct drand48_data data;
	srand48_r(0xc0c0c0c0, &data);

	do {
		long n;
		state state;

		do {
			if (!running) {
				return 0;
			}

			lrand48_r(&data, &n);
			n %= kHighestRandom;

			lock.lock();

			state = numbers[n];
			if (state == NUMBER_INSERTED) {
				numbers[n] = NUMBER_IN_USE;
				lock.unlock();

				break;
			}

			lock.unlock();
		} while (true);

		if (!list->erase(n)) {
			fprintf(stderr, "[%ld] Couldn't erase %u.\n", thread, n);
		}

		lock.lock();
		numbers[n] = NUMBER_ERASED;
		lock.unlock();
	} while (true);

	return NULL;
}

void* iterator(void* arg)
{
	util::concurrent::skiplist<long, longcmp> *list = reinterpret_cast<util::concurrent::skiplist<long, longcmp>*>(arg);

	util::concurrent::skiplist<long, longcmp>::iterator it;

	bool forward = true;

	do {
		if (forward) {
			if (list->begin(it)) {
				while ((running) && (list->next(it)));
			}
		} else {
			if (list->end(it)) {
				while ((running) && (list->previous(it)));
			}
		}

		forward = !forward;

		sched_yield();
	} while (running);

	return NULL;
}

void test_concurrent_skiplist_iterators()
{
	util::concurrent::skiplist<long, longcmp> list;

	if (!list.init()) {
		fprintf(stderr, "Couldn't initialize skip list.\n");
		return;
	}

	for (unsigned i = 0; i < kHighestRandom; i++) {
		numbers[i] = NUMBER_NOT_USED;
	}

	for (unsigned i = 0; i < 1000; i++) {
		for (unsigned j = 0; j < 1000; j++) {
			long n;
			do {
				n = random() % kHighestRandom;
			} while (numbers[n] != NUMBER_NOT_USED);

			if (!list.insert(n)) {
				fprintf(stderr, "Couldn't insert %d.\n", n);
				return;
			}

			numbers[n] = NUMBER_INSERTED;
		}

		for (unsigned j = 0; j < kHighestRandom; j++) {
			if (numbers[j] == NUMBER_INSERTED) {
				if (!list.erase(j)) {
					fprintf(stderr, "Couldn't erase %d.\n", j);
					return;
				}

				numbers[j] = NUMBER_NOT_USED;
			}
		}
	}

	printf("Inserting from 1 to 10...\n");
	for (long i = 10; i > 0; i--) {
		if (!list.insert(i)) {
			fprintf(stderr, "Couldn't insert %d.\n", i);
			return;
		}
	}

	print_list(list, true);
	print_list(list, false);

	printf("From 5 forward...\n");
	util::concurrent::skiplist<long, longcmp>::iterator it;
	if (list.seek(5, it)) {
		do {
			printf("%d\n", it.key());
		} while (list.next(it));
	}

	printf("Erasing 5...\n");
	list.erase(5);

	print_list(list, true);
	print_list(list, false);

	printf("Erasing 1...\n");
	list.erase(1);

	print_list(list, true);
	print_list(list, false);

	printf("Erasing 10...\n");
	list.erase(10);

	print_list(list, true);
	print_list(list, false);
}

void print_list(const util::concurrent::skiplist<long, longcmp>& list, bool forward)
{
	util::concurrent::skiplist<long, longcmp>::iterator it;

	if (forward) {
		printf("Forward...\n");
		if (list.begin(it)) {
			do {
				printf("%d\n", it.key());
			} while (list.next(it));
		}
	} else {
		printf("Backward...\n");
		if (list.end(it)) {
			do {
				printf("%d\n", it.key());
			} while (list.previous(it));
		}
	}
}

void test_single_threaded_skiplist()
{
	util::skiplist<long, longcmp> list;

	if (!list.init()) {
		fprintf(stderr, "Couldn't initialize skip list.\n");
		return;
	}

	for (unsigned i = 0; i < kHighestRandom; i++) {
		numbers[i] = NUMBER_NOT_USED;
	}

	for (unsigned i = 0; i < 1000; i++) {
		for (unsigned j = 0; j < 1000; j++) {
			long n;
			do {
				n = random() % kHighestRandom;
			} while (numbers[n] != NUMBER_NOT_USED);

			if (!list.insert(n)) {
				fprintf(stderr, "Couldn't insert %d.\n", n);
				return;
			}

			numbers[n] = NUMBER_INSERTED;
		}

		for (unsigned j = 0; j < kHighestRandom; j++) {
			if (numbers[j] == NUMBER_INSERTED) {
				if (!list.erase(j)) {
					fprintf(stderr, "Couldn't erase %d.\n", j);
					return;
				}

				numbers[j] = NUMBER_NOT_USED;
			}
		}
	}

	printf("Inserting from 1 to 10...\n");
	for (long i = 10; i > 0; i--) {
		if (!list.insert(i)) {
			fprintf(stderr, "Couldn't insert %d.\n", i);
			return;
		}
	}

	print_list(list, true);
	print_list(list, false);

	printf("From 5 forward...\n");
	util::skiplist<long, longcmp>::iterator it;
	if (list.seek(5, it)) {
		do {
			printf("%d\n", it.key());
		} while (list.next(it));
	}

	printf("Erasing 5...\n");
	list.erase(5);

	print_list(list, true);
	print_list(list, false);

	printf("Erasing 1...\n");
	list.erase(1);

	print_list(list, true);
	print_list(list, false);

	printf("Erasing 10...\n");
	list.erase(10);

	print_list(list, true);
	print_list(list, false);

	printf("Clearing the list...\n");
	list.clear();

	print_list(list, true);
	print_list(list, false);

	printf("Inserting from 1 to 10...\n");
	for (long i = 10; i > 0; i--) {
		if (!list.insert(i)) {
			fprintf(stderr, "Couldn't insert %d.\n", i);
			return;
		}
	}

	print_list(list, true);
	print_list(list, false);
}

void print_list(const util::skiplist<long, longcmp>& list, bool forward)
{
	util::skiplist<long, longcmp>::iterator it;

	if (forward) {
		printf("Forward...\n");
		if (list.begin(it)) {
			do {
				printf("%d\n", it.key());
			} while (list.next(it));
		}
	} else {
		printf("Backward...\n");
		if (list.end(it)) {
			do {
				printf("%d\n", it.key());
			} while (list.previous(it));
		}
	}
}
