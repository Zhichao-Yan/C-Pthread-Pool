#ifndef SEMA_H
#define SEMA_H
#include "pthread.h"

typedef struct sem {
	pthread_mutex_t mutex;
	pthread_cond_t   cond;
	int v;
}sem;


void sem_init(sem *s,int value);
void sem_reset(sem *s);
void sem_post(sem *s);
void sem_reset(sem *s);
void sem_wait(sem* s);

#endif