#include "queue.h"
#include <stdlib.h>

int jobqueue_init(jobqueue* jobqueue_p)
{
    jobqueue_p->len = 0;
	jobqueue_p->front = NULL;
	jobqueue_p->rear  = NULL;
    jobqueue_p->has_jobs = (struct bsem*)malloc(sizeof(struct bsem));
	if (jobqueue_p->has_jobs == NULL){
		return -1;
	}
    bsem_init(jobqueue_p->has_jobs, 0);
	pthread_mutex_init(&(jobqueue_p->rwmutex), NULL);
    return 0;
}