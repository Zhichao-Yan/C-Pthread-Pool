#include "pool.h"
#include "semaphore.h"
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //sleep()



#ifdef POOL_DEBUG
#define POOL_DEBUG 1
#define err(str) fprintf(stderr, str)
#else
#define POOL_DEBUG 0
#define err(str)
#endif

#if defined(__APPLE__)
#include <AvailabilityMacros.h>
#else
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif
#if defined(__linux__)
#include <sys/prctl.h>
#endif






static volatile int threads_keep_alive = 1;
static volatile int threads_hold_on = 0; // 初始化线程挂起标志，当前所有线程不会被挂起

pool* pool_init(int num_threads)
{
	// make a new pool
    pool *pl = (pool*)malloc(sizeof(pool));
    if(pl == NULL)
    {
        err("pool_init(): Could not allocate memory for pool\n");
		return NULL;
    }
	pthread_mutex_init(&(pl->count_lock), NULL);
	pthread_cond_init(&(pl->all_threads_idle), NULL);
	pl->alive = 0; // number of alive threads
	pl->busy = 0; // number of busy threads
	// init the job queue
    if(queue_init(&(pl->job_queue)) == -1)
    {
        err("pool_init(): Could not allocate memory for job queue\n");
		free(pl);
		return NULL;
    }
	// make threads in pool
    pl->threads = (thread**)malloc(num_threads * sizeof(thread*));
	if (pl->threads == NULL){
		err("pool_init(): Could not allocate memory for threads\n");
		queue_destroy(&pl->job_queue);
		free(pl);
		return NULL;
	}
	// init all the threads in pool
    for (int i = 0; i < num_threads; i++){
		thread_init(pl, &pl->threads[i], i);
#if POOL_DEBUG
		printf("Created thread%d in pool!\n", i);
#endif
	}
    while(pl->alive != num_threads){} // wait to return
    return pl;
}

void pool_destroy(pool* pl)
{
	if(pl == NULL) // pl为空，直接退出
		return;
	int n = pl->alive; // 必须在下面一句之前，如果在下一句之后，线程可能提前退出，导致计数不正确，导致后面线程无法全部释放
	threads_keep_alive = 0; // 终止工作线程的无限循环

	double TIMEOUT = 1.0;
	time_t start, end;
	double pass_t = 0.0;
	time (&start);
	while (pass_t < TIMEOUT && pl->alive){
		sem_post_all(pl->job_queue.empty);
		time(&end);
		pass_t = difftime(end,start);
	}
	while (pl->alive)
	{
		sem_post_all(pl->job_queue.empty);
		sleep(1);
	}

	queue_destroy(&pl->job_queue);
	for(int i = 0; i < n; ++i)
		thread_destroy(pl->threads[i]);
	free(pl->threads);
	free(pl);
}

// add job to the job queue of the pool
int pool_add_job(pool* pl, void (*func)(void*), void* arg)
{
	job* newjob;
	newjob = (job*)malloc(sizeof(job));
	if (newjob == NULL){
		err("pool_add_job(): Could not allocate memory for new job\n");
		return -1;
	}
	newjob->func = func;
	newjob->arg = arg; 
	queue_push(&pl->job_queue, newjob);
	return 0;
}
// wait for queue is empty && all the threads are idle
void pool_wait(pool* pl)
{
	pthread_mutex_lock(&pl->count_lock);
	while (pl->job_queue.len || pl->busy)  // queue is not empty or some threads are busy processing jobs
    { 
		pthread_cond_wait(&pl->all_threads_idle, &pl->count_lock);
	}
	pthread_mutex_unlock(&pl->count_lock);
}


void pool_pause(pool* pl) 
{
	for ( int i = 0; i < pl->alive; i++){
		pthread_kill(pl->threads[i]->td, SIGUSR1);
	}
}

int pool_busy_threads(pool* pl)
{
	return pl->busy;
}

void pool_hold(int sig_id) 
{
    if(sig_id == SIGUSR1)
	{
		threads_hold_on = 1;
		while (threads_hold_on){
			sleep(1); // 线程循环休眠，直到pool_resume将threads_hold_on置0，线程不在休眠
		}
	}
	return;
}
void pool_resume(pool* pl) 
{
	threads_hold_on = 0; // 所有线程重新醒过来
}



int thread_init(pool* pl, thread** thread_ptr, int i)
{
	(*thread_ptr) = (thread*)malloc(sizeof(thread));
	if((*thread_ptr) == NULL){
		err("thread_init(): Could not allocate memory for thread\n");
		return -1;
	}
	(*thread_ptr)->pl = pl;
	(*thread_ptr)->id  = i;
	pthread_create(&((*thread_ptr)->td),NULL,thread_do,(*thread_ptr));
	pthread_detach((*thread_ptr)->td);
	return 0;
}

void* thread_do(void *arg)
{
    thread *cur = (thread*)arg;
    char thread_name[16] = {0};
	snprintf(thread_name,16,"PoolThread-%d", cur->id); // put thread name into thread_name

#if defined(__linux__)
	/* Use prctl instead to prevent using _GNU_SOURCE flag and implicit declaration */
	// PR_SET_NAM：Set the name of the calling thread
	// using the value in the location pointed to by  (char *) arg2
	prctl(PR_SET_NAME, thread_name); 
#elif defined(__APPLE__) && defined(__MACH__)
	pthread_setname_np(thread_name); // 设置线程名字，这个名字可以在debbug时看到，方便区分线程
#else
	err("thread_do(): pthread_setname_np is not supported on this system");
#endif

    struct sigaction act; // 数据结构struct sigaction
	sigemptyset(&act.sa_mask); // sigemptyset()清除sa_mask中的信号集
	act.sa_flags = 0;
	act.sa_handler = pool_hold;
    // 调用sigaction函数修改信号SIGUSR1相关的处理动作，即以后用这个处理函数响应信号
    // 在调用信号处理函数之前act.sa_mask会被加到进程的信号屏蔽字中
    // 信号处理函数sa_handle返回之前进程信号集会被还原
    // SIGUSR1 也在操作系统建立的新信号屏蔽字中，保证了在处理给定信号时，如果这种信号再次发生，它会被阻塞到信号处理完毕
    // 出错返回-1
	if (sigaction(SIGUSR1, &act, NULL) == -1) 
    { 
		err("thread_do(): cannot handle SIGUSR1");
	}
    pool* pl = cur->pl;
	pthread_mutex_lock(&pl->count_lock);
	++pl->alive;
	pthread_mutex_unlock(&pl->count_lock);

    while(threads_keep_alive){
		sem_wait((pl->job_queue).empty); // 队列为空，则陷入等待
		if (threads_keep_alive){
            pthread_mutex_lock(&pl->count_lock);
            ++pl->busy;
            pthread_mutex_unlock(&pl->count_lock);

            // pull out a job and execute it
			job* jp = queue_pull(&pl->job_queue); 
			if (jp) {
                execute(jp);
			}
			pthread_mutex_lock(&pl->count_lock);
			--pl->busy;
			if (!pl->busy) {
				pthread_cond_signal(&(pl->all_threads_idle));
			}
			pthread_mutex_unlock(&pl->count_lock);
		}
	}
	pthread_mutex_lock(&pl->count_lock);
	--pl->alive;
	pthread_mutex_unlock(&pl->count_lock);
	return NULL;
}

void thread_destroy(thread* ptr)
{
	free(ptr);
}

void execute(job *jp)
{
    (jp->func)(jp->arg);
    free(jp);
}

