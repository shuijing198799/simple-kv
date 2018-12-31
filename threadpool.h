//write a thread pool to process command, any time get the task from the 
//network channel, submmit it to the threadpool, and threadpool will process
//it and take the result to the client.
#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "zmalloc.h"

#define LOCK(lock) pthread_mutex_lock(&lock) 
#define UNLOCK(lock) pthread_mutex_unlock(&lock) 
#define JOIN(thread) pthread_join(thread, NULL);

typedef void* (*runnable)(void*);//real task info

typedef struct task_t {
	runnable runner; //task func;
	void* args; //parameters of runner
	struct task_t* pre; //pre node of task
	struct task_t* next; //next node of task
}task_t;

typedef struct threadPool {
	pthread_t * threads; // thread queue
	int maxsize; //maxsize of threadpool
	int coresize; // main size of threadpool
	int idle; //idle thread
	int count; //thread number
	task_t *head; //task link 
	task_t *tail; //task link 
	int stop; //stop the thread pool before destoryed
	pthread_mutex_t tasklock; // tasklock 
	pthread_mutex_t countlock; // countlock
	pthread_mutex_t idlelock; // countlock
}threadPool;

void initThreadPool(threadPool* pool, int coresize,int maxsize);
void destroyThreadPool(threadPool* pool);
void execute(threadPool* pool, runnable task,void* args);
void threadpool_test();

#endif

