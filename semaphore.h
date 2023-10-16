#ifndef SEMA_H
#define SEMA_H
#include "pthread.h"
#ifdef POOL_DEBUG
#define err(str) fprintf(stderr, str)
#else
#define err(str)
#endif


typedef struct sem {
	pthread_mutex_t mutex;
	pthread_cond_t   cond;
	int v;
}sem;


void sem_init(sem *s,int value);
void sem_reset(sem *s);
void sem_post(sem *s);
void sem_post_all(sem *s);
void sem_reset(sem *s);
void sem_wait(sem* s);

#endif