#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "util/skiplist.h"

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

static void test_single_threaded_skiplist();
static void print_list(const util::skiplist<long, longcmp>& list, bool forward);

int main(int argc, char** argv)
{
	test_single_threaded_skiplist();
	return 0;
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
