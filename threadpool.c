#include "threadpool.h"


threadPool* initThreadPool( int coresize, int maxsize) {
	threadPool* pool = zmalloc(sizeof(threadPool));
	pool->coresize = coresize;
	pool->maxsize = maxsize;
	pool->idle = 0;
	pool->count = 0;
	pool->head = zmalloc(sizeof(task_t));
	pool->tail = zmalloc(sizeof(task_t));
	pool->stop = 0;
	pool->head->pre = NULL;
	pool->head->next = pool->tail;
	pool->tail->next = NULL;
	pool->tail->pre = pool->head;
	pthread_mutex_init(&pool->tasklock,NULL);
	pthread_mutex_init(&pool->countlock,NULL);
	pthread_mutex_init(&pool->idlelock,NULL);
	pool->threads = (pthread_t*)zmalloc(maxsize * sizeof(pthread_t));
	memset(pool->threads, 0, sizeof(pthread_t) * maxsize);
	return pool;
}

void destroyThreadPool(threadPool* pool) {
	pool->stop = 1;
	for(int i = 0; i < pool->maxsize; i++) {
		if(pool->threads[i] != 0) {
			JOIN(pool->threads[i]);
		} else {
			printf("thread space no use %lu\n",pool->threads[i]);
		}
	}
	//printf("at last pool count is %d\n",pool->count);
	LOCK(pool->tasklock);
	pthread_mutex_destroy(&pool->tasklock);
	LOCK(pool->countlock);
	pthread_mutex_destroy(&pool->countlock);
	LOCK(pool->idlelock);
	pthread_mutex_destroy(&pool->idlelock);
	zfree(pool->tail);
	zfree(pool->head);
	zfree(pool->threads);
	zfree(pool);
}

void* task_func(void* arg) {
	pthread_t tid = pthread_self();
	printf("Thread %lu starting\n", (size_t)tid);
	threadPool *pool = (threadPool*)arg;

	while(1) {
		//	actually we need not maintenance the count in the pool.the count and 
		//	pid array will be destroy at last.
		if(pool->stop && pool->head->next == pool->tail) {
			LOCK(pool->countlock);
			pool->count--;
			printf("stop thread %d left idle %d\n",pool->count,pool->idle);
			UNLOCK(pool->countlock);
			return NULL;
		}
		if(pool->head->next == pool->tail) {
			continue;
		}
		LOCK(pool->idlelock);
		pool->idle--;
		UNLOCK(pool->idlelock);

		LOCK(pool->tasklock);
		task_t* tmp = pool->tail->pre;
		pool->tail->pre = tmp->pre;
		tmp->pre->next = pool->tail;
		UNLOCK(pool->tasklock);

		//run the function
		tmp->runner(tmp->args);
		//free the node;
		zfree(tmp);

		LOCK(pool->idlelock);
		pool->idle++;
		UNLOCK(pool->idlelock);
		
	}
}

void execute(threadPool* pool, runnable task,void* args) {
	if(pool->stop) {
		//if threadpool is stop no task anymore
		return;
	}
	task_t* t = (task_t*)zmalloc(sizeof(task_t));
	t->args = args;
	t->runner = task;

	//at first, we must insert node to the head
	//a lock should be add here, let me consider the cas or lock
	
	LOCK(pool->tasklock);
	t->pre = pool->head;
	t->next = pool->head->next;
	pool->head->next = t;
	t->next->pre = t;
	UNLOCK(pool->tasklock);

	if(pool->count < pool->coresize || 
			(pool->count >= pool->coresize && pool->count < pool->maxsize 
			  && pool->idle == 0 && pool->head->next != pool->tail)) {
		//if thread number is less than core number or less than max number
		//and idle thread is zero create a thread to run the task directly
		//and the taskpool has task todo
		pthread_t tid;
		if((pthread_create(&tid, NULL, task_func, (void*)pool)) == -1) {
			printf("create thread error %s\n",strerror(errno));
		}
		
		LOCK(pool->countlock);
		pool->threads[pool->count] = tid;
		pool->count++;
		UNLOCK(pool->countlock);
		LOCK(pool->idlelock);
		pool->idle++;
		UNLOCK(pool->idlelock);
	} 
}

void* printTest(void* arg) {
	pthread_t tid = pthread_self();
	printf("Thread %lu print\n", (size_t)tid);
	return NULL;
}

void threadpool_test() {
	for(int time = 0; time < 1; time++) {
		printf("test times is %d\n",time);
		threadPool* pool = initThreadPool(4,10);
		int tmp = 0;
		for(int i = 0; i < 100000; i++) {
			execute(pool,printTest,NULL);
		}
		destroyThreadPool(pool);
	}
}
