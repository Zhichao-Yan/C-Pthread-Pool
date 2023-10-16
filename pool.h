#ifndef POOL_H
#define POOL_H
#include <pthread.h>
#include "queue.h"


typedef struct thread
{
    pthread_t td;
    int id; 
    struct pool *pl;
}thread;

typedef struct pool{
	thread**   threads;         /* pointer to threads*/
	volatile int alive;                  /* threads currently alive   */
	volatile int busy;                   /* threads currently working */
	pthread_mutex_t  count_lock;       /* used for thread count etc */
	pthread_cond_t  all_threads_idle;    /* signal to thpool_wait     */
	queue  job_queue;                  /* job queue                 */
}pool;




/* thread */


pool* pool_init(int num);
int pool_add_job(pool* pl, void (*func)(void*), void* arg);
void pool_wait(pool* pl);
void pool_pause(pool* pl);
void pool_resume(pool* pl);
void pool_hold(int sig_id);
void pool_destroy(pool* pl);
int pool_busy_threads(pool* pl);

void thread_destroy(thread* ptr);
int thread_init(pool* pl, thread** thread_ptr, int i);
void* thread_do(void *arg);
void execute(job *jp);
#endif