#include <stdlib.h>
#include <stdio.h>
#include "util/concurrent/atomic/markable_ptr.h"

int main()
{
	util::concurrent::atomic::markable_ptr<int> p = new int;

	*p = 5;

	printf("p is %smarked.\n", p.marked() ? "" : "not ");

	printf("Value of *p is %d.\n", *p);

	printf("Marking p...\n");
	p.mark();
	printf("p is %smarked.\n", p.marked() ? "" : "not ");

	printf("Value of *p is %d.\n", *p);

	printf("Unmarking p...\n");
	p.unmark();
	printf("p is %smarked.\n", p.marked() ? "" : "not ");

	printf("Value of *p is %d.\n", *p);

	delete p.get();

	return 0;
}
