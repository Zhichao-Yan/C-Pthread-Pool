#include "queue.h"
#include <stdlib.h>


int queue_init(queue* q)
{
    q->len = 0;
	q->front = NULL;
	q->rear  = NULL;
    q->empty = (sem*)malloc(sizeof(sem));
	if (q->empty == NULL){  // failed to allocate
		return -1;
	}
    sem_init(q->empty, 0);
	pthread_mutex_init(&(q->rw), NULL);
    return 0;
}

void queue_push(queue* q,job* newjob)
{

	pthread_mutex_lock(&q->rw);
	switch(q->len)
    { 
		case 0:  /* if no jobs in queue */
        	        newjob->next = NULL;
					q->front = newjob;
					q->rear  = newjob;
					break;

		default: /* if there are jobs in queue */
					q->rear->next = newjob; 
					q->rear = newjob;
	}
	q->len++;
	sem_post(q->empty);
	pthread_mutex_unlock(&q->rw);
}
job* queue_pull(queue* q)
{

	pthread_mutex_lock(&q->rw);
	job* p = q->front;

	switch(q->len){
		case 0:  /* if no jobs in queue */
		  			break;

		case 1:  /* if one job in queue */
					q->front = NULL;
					q->rear  = NULL;
					q->len = 0;
					break;

		default: /* if >1 jobs in queue */
					q->front = p->next;
					q->len--;
					/* more than one job in queue -> post it */
                    // you can still take job from it
                    sem_post(q->empty); 
	}
	pthread_mutex_unlock(&q->rw);
	return p;
}

void queue_clear(queue* q)
{
	while(q->len){
		free(queue_pull(q)); // free the job
	}
    q->len = 0;
	q->front = NULL;
	q->rear  = NULL;
	sem_reset(q->empty);
}
void queue_destroy(queue* p)
{
	queue_clear(p);
	free(p->empty);
    pthread_mutex_destroy(&(p->rw));
}