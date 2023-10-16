#include "pool.h"
#include <stdio.h>

void task(void *arg){
	printf("Thread #%u working on %d\n", (int)pthread_self(), (int) arg);
}

int main()
{
    pool *pl = pool_init(4);

	for (int i=0; i<40; i++){
		pool_add_job(pl, task, (void*)(uintptr_t)i);
	};
    pool_wait(pl);
	pool_destroy(pl);
}