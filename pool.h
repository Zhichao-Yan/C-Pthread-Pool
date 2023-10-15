#ifndef POOL_H
#define POOL_H
#include <pthread.h>




/* thread */
typedef struct thread
{
    pthread_t td;
    int id; 
}thread;


#endif