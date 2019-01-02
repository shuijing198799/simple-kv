/*
 * this is the network module for the use multiple I/O module to 
 * communicate with the client and server. the multiple 
 * multiplexer us epoll and I'd like user block socket int this 
 * demo. only file event is occur in this demo. so I ignore time
 * event and no other multiplexer is implement. this module is 
 * draw lessions from redis network module and libevent.
*/

#ifndef __EPOLL_H
#define __EPOLL_H

#include <stdio.h>
#include <sys/epoll.h>

#define AE_OK 0
#define AE_ERR -1

#define AE_NONE 0
#define AE_READABLE 1
#define AE_WRITABLE 2

struct aeEventLoop;

typedef void aeFileProc(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);

//structure for file event
typedef struct aeFileEvent { 
	int mask; //register the kind of event
	aeFileProc* rfileProc; //register read call back function
	aeFileProc* wfileProc; // register write call back function
	void* clientData; // callback args.
} aeFileEvent;

//struct for invoke event fd.
typedef struct aeFiredEvent {
	int fd;
	int mask;
} aeFiredEvent;

//main structure of network loop
typedef struct aeEventLoop {
	int setsize;
	aeFileEvent *events;
	aeFiredEvent *fired;
	int stop;
	int epollfd;
	struct epoll_event *ep_events;
	void* (*reportPool) (void*); //report threadpool monitor everytime
} aeEventLoop;

aeEventLoop* aeCreateEventLoop (int setsize);
void aeDeleteEventLoop(aeEventLoop* eventLoop);
void aeStop(aeEventLoop* eventLoop);
int aeCreateFileEvent(aeEventLoop* eventLoop, int fd, int mask, aeFileProc *proc, void* clientData);
void aeDeleteFileEvent(aeEventLoop* eventLoop, int fd, int mask);
int aeProcessEvents(aeEventLoop* eventLoop);
void aeMain(aeEventLoop* eventLoop);

#endif
