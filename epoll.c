#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/epoll.h>
#include "epoll.h"
#include "zmalloc.h"

aeEventLoop *aeCreateEventLoop(int setsize) {
    aeEventLoop *eventLoop;
    int i;

    if ((eventLoop = zmalloc(sizeof(*eventLoop))) == NULL) goto err;
    eventLoop->events = zmalloc(sizeof(aeFileEvent)*setsize);
    eventLoop->fired = zmalloc(sizeof(aeFiredEvent)*setsize);
	eventLoop->ep_events = zmalloc(sizeof(struct epoll_event)*setsize);
    if (eventLoop->events == NULL || eventLoop->fired == NULL \
			|| eventLoop->ep_events == NULL) goto err;
    eventLoop->setsize = setsize;
    eventLoop->stop = 0;
	eventLoop->epollfd = epoll_create(1024);
	if(eventLoop->epollfd == -1) goto err;

    for (i = 0; i < setsize; i++)
        eventLoop->events[i].mask = AE_NONE;
    return eventLoop;

err:
    if (eventLoop) {
        zfree(eventLoop->events);
        zfree(eventLoop->fired);
		zfree(eventLoop->ep_events);
        zfree(eventLoop);
    }
    return NULL;
}

void aeDeleteEventLoop(aeEventLoop *eventLoop) {
	close(eventLoop->epollfd);
	zfree(eventLoop->events);
    zfree(eventLoop->ep_events);
    zfree(eventLoop->fired);
    zfree(eventLoop);
}

void aeStop(aeEventLoop *eventLoop) {
    eventLoop->stop = 1;
}

int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask, aeFileProc *proc, void *clientData) {
    if (fd >= eventLoop->setsize) {
		printf("out of max setsize\n");
        errno = ERANGE;
        return AE_ERR;
    }
    aeFileEvent *fe = &eventLoop->events[fd];

	struct epoll_event ee = {0};

	// check epoll action type ,if this fd is already register event
	// only need epoll_ctl_mod else use epoll_ctl_add
	int op = (fe->mask == AE_NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
	ee.events = 0;
	//printf("op is %d fd is %d mask %d\n",op,fd,mask);
	fe->mask |= mask;
	//printf("op is %d fd is %d mask %d\n",op,fd,mask);

	//register read and wirte event to epoll
	if(fe->mask & AE_READABLE) ee.events |= EPOLLIN;
	if(fe->mask & AE_WRITABLE) ee.events |= EPOLLOUT;
	ee.data.fd = fd;

	//add event to epoll method
	if(epoll_ctl(eventLoop->epollfd,op,fd,&ee) == -1) { 
		printf("modify epoll error\n");
		return AE_ERR;
	}

	//add mask and call back function and arg to eventloop
	//an error occurs here make me a lot time to debug. I
	//use write proc overlap the read event. this is stupid
	//check every detail when use other one's code.
    if (mask & AE_READABLE) fe->rfileProc = proc;
    if (mask & AE_WRITABLE) fe->wfileProc = proc;
    fe->clientData = clientData;
    return AE_OK;
}

void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask) {
    if (fd >= eventLoop->setsize) return;

	///if this event is already no register return;
    aeFileEvent *fe = &eventLoop->events[fd];
    if (fe->mask == AE_NONE) return;

	struct epoll_event ee = {0};

	//printf(" fd is %d mask %d\n",fd,fe->mask);
	fe->mask = fe->mask & (~mask);
	//printf(" fd is %d mask %d\n",fd,fe->mask);

	int op = (fe->mask == AE_NONE) ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;

	//set epoll event mask
	ee.events = 0;
	if(fe->mask & AE_READABLE) ee.events |= EPOLLIN;
	if(fe->mask & AE_WRITABLE) ee.events |= EPOLLOUT;

	//set epoll event fd
	ee.data.fd = fd;
	
	//real delete event in epoll
	epoll_ctl(eventLoop->epollfd,op,fd,&ee);
}

int aeProcessEvents(aeEventLoop *eventLoop) {
	printf("begin process Event loop\n");

	int processed = 0, numevents, i, j;

	//unlike redis , this demo have no time event to proces,
	//so I choose to make epoll_wait wating forserver, if no
	//file event occurs;
	numevents = epoll_wait(eventLoop->epollfd, eventLoop->ep_events,eventLoop->setsize, 1000);

	printf("n events has been fired %d\n",numevents);

	//process the events from kernel, and it to the fired list.
	for(i = 0; i < numevents; i++) {
		int mask = 0;
		struct epoll_event *e = eventLoop->ep_events+i;
		if (e->events & EPOLLIN) mask |= AE_READABLE;
		if (e->events & EPOLLOUT) mask |= AE_WRITABLE;
		if (e->events & EPOLLERR) mask |= AE_WRITABLE;
		if (e->events & EPOLLHUP) mask |= AE_WRITABLE;
		eventLoop->fired[i].fd = e->data.fd;
		eventLoop->fired[i].mask = mask;
	}

	//process the fired events in this loop.
	for (j = 0; j < numevents; j++) {
		aeFileEvent *fe = &eventLoop->events[eventLoop->fired[j].fd];
		int mask = eventLoop->fired[j].mask;
		int fd = eventLoop->fired[j].fd;
		int fired = 0; 

		if (mask & AE_READABLE) {
			fe->rfileProc(eventLoop,fd,fe->clientData,mask);
			fired++;
		}

		if (mask & AE_WRITABLE) {
			fe->wfileProc(eventLoop,fd,fe->clientData,mask);
			fired++;
		}
		
		processed++;
	}
	return processed;
}

void aeMain(aeEventLoop* eventLoop) {
	while(!eventLoop->stop) {
		eventLoop->reportPool(NULL);
		aeProcessEvents(eventLoop);
	}
}
 
