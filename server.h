#ifndef __SERVER_H
#define __SERVER_H

#include "str.h"
#include "tools.h"
#include "hash.h"
#include "epoll.h"
#include "sortmap.h"
#include "inet.h"

typedef struct client { 
	int fd; //file descriptor
	char* inbuf; // requset buf
	char* outbuf; // reply buf
	size_t inlen; // temporary number to remember how many bytes have been write every event.
	size_t outlen; // temporary number to remember how many bytes have been write every event.
	char* client_ip; //client ip
	int port; //client port
} client;

typedef struct serverInfo {
	sortmap* sm; //main store structure
	aeEventLoop* el; // main eventloop
	int fd; //main process fd
	int client_channel; //number fo clients
} serverInfo;

client* initClient(const char* ip, int port,int fd);
void destroyClient(client* client);

#endif


