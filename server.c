#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"

//Global info server
serverInfo server;

//test the main datasturct sortmap curd function.
//it can be see in eyes;
void test_curd() {

	for(int i = 0; i < 100000; i++) {
		char string[20];
		itoa(i,string);
		set(string,string,server.sm);
	}
	set("9999","ljoejgieg",server.sm);
	del("9998",server.sm);

/*	char* tmp = scan(server.sm);
	if(!tmp)
		printf("tmp is null\n");
	printf("%s\n",tmp);
	freestr(tmp);*/

/*	for(int i = 0; i < 9000; i++) {
		char string[20];
		itoa(i,string);
		del(string,server.sm);
	}
	

	//delete null
//	del("10001",server.sm);
*/
}

client* initClient(const char* ip, int port,int fd) {
	client* newclient = zmalloc(sizeof(* newclient));
	if(!newclient) {
		return NULL;
	}
	newclient->inbuf = emptystr();
	newclient->outbuf = emptystr();
	newclient->client_ip = newstr(ip);
	newclient->port = port;
	newclient->fd = fd;
	newclient->inlen = 0;
	newclient->outlen = 0;
}

void destroyClient(client* client) {
	aeDeleteFileEvent(server.el,client->fd,AE_READABLE | AE_WRITABLE);
	server.client_channel--;
	freestr(client->client_ip);
	freestr(client->inbuf);
	freestr(client->outbuf);
	zfree(client);
}

void* reportPoolInfo(void* val) {
	time_t now = time(NULL);
	if(now - server.pool->lastReport >= 10) {
		printf("pool monitor run count %d idle %d core %d max %d\n",
				server.pool->count,server.pool->idle,server.pool->coresize,server.pool->maxsize);
		server.pool->lastReport = now;
	}
	return NULL;
}


int main() {
	server.sm = initSortMap();
//	str_test();
//	sortmap_test();
//	threadpool_test();
 	test_curd();
	server.el = aeCreateEventLoop(1024);
	server.pool = initThreadPool(8,12);
	server.el->reportPool = reportPoolInfo;
	int fd = anetCreateSocket();
	anetTcpServer(fd, 9876, "127.0.0.1",1024);

	server.fd = fd;
	server.client_channel = 0;

	if (aeCreateFileEvent(server.el, server.fd, AE_READABLE,acceptTcpHandler,NULL) == AE_ERR) {
		printf("Unrecoverable error creating server.ipfd file event.\n");
	}
	aeMain(server.el);
	destroySortMap(server.sm);
	destroyThreadPool(server.pool);
	aeDeleteEventLoop(server.el); 
}
