#include "semaphore.h"
#include <stdio.h>
#include <stdlib.h>

void sem_init(sem *s,int value)
{
    if (value < 0 || value > 1) 
    {
		err("sem_init(): Binary semaphore can take only values 1 or 0");
		exit(1);
	}
	pthread_mutex_init(&(s->mutex), NULL);
	pthread_cond_init(&(s->cond), NULL);
	s->v = value;
}
void sem_reset(sem *s) 
{
	sem_init(s, 0);
}

void sem_post(sem *s) 
{
	pthread_mutex_lock(&s->mutex);
	s->v = 1;
	pthread_cond_signal(&s->cond);
	pthread_mutex_unlock(&s->mutex);
}

void sem_post_all(sem *s) 
{
	pthread_mutex_lock(&s->mutex);
	s->v = 1;
	pthread_cond_broadcast(&s->cond);
	pthread_mutex_unlock(&s->mutex);
}

void sem_wait(sem* s)
{
	pthread_mutex_lock(&s->mutex);
	while (s->v == 0) {  // s->v为0，表示任务队列为空，等待变化
		pthread_cond_wait(&s->cond, &s->mutex);
	}
    // 跳出上面循环时s->v == 1,下面重新把它变成0
	s->v = 0;
	pthread_mutex_unlock(&s->mutex);
}