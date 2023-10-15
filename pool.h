#ifndef POOL_H
#define POOL_H
#include <pthread.h>
#include "queue.h"


typedef struct pool{
	thread**   threads;         /* pointer to threads*/
	int alive;                  /* threads currently alive   */
	int busy;                   /* threads currently working */
	pthread_mutex_t  count_lock;       /* used for thread count etc */
	pthread_cond_t  all_threads_idle;    /* signal to thpool_wait     */
	queue  job_queue;                  /* job queue                 */
}pool;


/* thread */
typedef struct thread
{
    pthread_t td;
    int id; 
    pool *pl;
}thread;

pool* pool_init(int num);
int pool_add_job(pool* pl, void (*func)(void*), void* arg);

void pool_pause(pool* pl);
void pool_resume(pool* pl);
void pool_hold(int sig_id);
int pool_busy_threads(pool* pl);

void thread_destroy(thread* ptr);
void execute(job *jp);
#endif