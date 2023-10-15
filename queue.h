#ifndef QUEUE_H
#define QUEUE_H
#include "pthread.h"
#include "semaphore.h"


/* Job */
typedef struct job{
	void (*func)(void* arg);       /* function pointer          */
	void* arg;                      /* function's argument       */
    struct job* next;               /* pointer to the next job in the queue  */
}job;

/* Queue */
typedef struct queue{
	job  *front;                         /* pointer to front of queue */
	job  *rear;                          /* pointer to rear  of queue */
	int  len;                           /* number of jobs in queue   */
    sem  *empty;                      /* flag as binary semaphore  */
    pthread_mutex_t rw;             /* used for queue access */
}queue;


int queue_init(queue* q);
void queue_push(queue* q,job* newjob);
job* queue_pull(queue* q);
void queue_clear(queue* q);
void queue_destroy(queue* p);
#endif