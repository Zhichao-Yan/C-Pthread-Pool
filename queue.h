#ifndef QUEUE_H
#define QUEUE_H
#include "pthread.h"

typedef struct bsem {
	pthread_mutex_t mutex;
	pthread_cond_t   cond;
	int v;
}bsem;

/* Job */
typedef struct job{
	void (*func)(void* arg);       /* function pointer          */
	void* arg;                      /* function's argument       */
    struct job* prev;               /* pointer to previous job   */
}job;

typedef struct jobqueue{
	job  *front;                         /* pointer to front of queue */
	job  *rear;                          /* pointer to rear  of queue */
	int   len;                           /* number of jobs in queue   */
    bsem *has_jobs;                      /* flag as binary semaphore  */
    pthread_mutex_t rwmutex;             /* used for queue r/w access */
}jobqueue;



int jobqueue_init(jobqueue* jobqueue_p);

#endif