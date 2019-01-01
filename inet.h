/*
 * add server and client module in this module , we can start a
 * server and write handler to handle write and read event when
 * event occurs. it has to include epoll module as a multiplexer
 * for this is a demo ,so I only use block socket with multiplexer
 */

#ifndef __INET_H
#define __INET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include "epoll.h"
#include "server.h"
#include "threadpool.h"

#define ANET_OK 0 //success
#define ANET_ERR -1 //fail
#define NET_IP_STR_LEN 46 //lenght of a net ip
#define MAX_CLIENTS 1 // max num of client ip
#define PROTO_IOBUF_LEN 16 *1024 // read and wirte buf is 16K use 1byte for test

#define USAGE "usage:\n    get key\n    set key val\n    del key\n    scan\n"
#define SUCCESS "OK\n"
#define PARSE_ERROR "PARSE ERROR\n"
#define NOKEY_ERROR "null\n"

int anetCreateSocket();
int anetListen(int s, struct sockaddr *sa, socklen_t len, int backlog);
int anetTcpServer(int s, int port, char *bindaddr, int backlog);
int anetRead(int fd, char *buf, int count);
int anetWrite(int fd, char *buf, int count);
int anetSetNonBlock( int fd, int non_block);
void acceptTcpHandler(aeEventLoop *el, int fd, void* privdata, int mask);
void readQueryFromClient(aeEventLoop *el, int fd, void* privdata, int mask);
void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask);
int _processCommand(void* c);
int _sendReply(void* c, char* buf);

#endif
