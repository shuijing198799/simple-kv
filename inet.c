#include <stdio.h>
#include <stdlib.h>

#include "inet.h"
#include "server.h"

extern serverInfo server;

//just a easy socket ignore  nonblock
int anetCreateSocket() {
    int s,yes = 1;
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        //printf("creating socket: %s\n", strerror(errno));
        return ANET_ERR;
	}

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		//printf("setsockopt SO_REUSEADDR: %s\n", strerror(errno));
		return ANET_ERR;
	}
	return s;
}

int anetSetNonBlock(int fd, int non_block) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        printf("fcntl(F_GETFL): %s\n", strerror(errno));
        return ANET_ERR;
    }

    if (non_block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1) {
        printf("fcntl(F_SETFL,O_NONBLOCK): %s\n", strerror(errno));
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetListen(int s, struct sockaddr *sa, socklen_t len, int backlog) {
    if (bind(s,sa,len) == -1) {
        printf("bind: %s\n", strerror(errno));
        close(s);
        return ANET_ERR;
    }

    if (listen(s, backlog) == -1) {
        printf("listen: %s\n", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    return ANET_OK;
}

int anetTcpServer(int s, int port, char *bindaddr, int backlog) {
	char _port[6];  
	struct sockaddr_in serverAddr;
	socklen_t len;

	snprintf(_port,6,"%d",port);
	memset(&serverAddr,0,sizeof(serverAddr));
	len = sizeof(serverAddr);

	serverAddr.sin_family=AF_INET;
	serverAddr.sin_addr.s_addr=inet_addr(bindaddr);
	serverAddr.sin_port=htons(port);

	if (anetListen(s,(struct sockaddr*)&serverAddr,len,backlog) == ANET_ERR)
		return ANET_ERR;

	return s;
}

void acceptTcpHandler(aeEventLoop *el, int fd, void* privdata, int mask) {
    int cport, cfd;
	char cip[NET_IP_STR_LEN];
	struct sockaddr_storage sa;
	client* newclient;

	//convert fd to char* and save it in client; for the time limit
	//i can't convert the map to void* and void* type and use function
	//ptr to complish polymorphic.
	char _fd[8]; // 100 milliom is enough.
	snprintf(_fd,8,"%d",fd);
	char* sfd = newstr(_fd);

	if(!sfd) goto err;

	socklen_t salen = sizeof(sa);
	cfd = accept(fd,(struct sockaddr*)&sa,&salen);
	if(cfd == -1) {
		printf("connect errror %s\n",strerror(errno));
		goto err;
	}
	if(anetSetNonBlock(cfd,1)) {
		printf("nonblock error %s\n",strerror(errno));
		goto err;
	}

	//create a new client struct and add it into server and use socket
	//to identify the client in server.
	newclient = initClient(cip,cport,cfd);
	if(!newclient) {
		goto err;
	}

	//register a new read event.
	if (aeCreateFileEvent(server.el,cfd,AE_READABLE,readQueryFromClient, newclient) == AE_ERR) {
		printf("register readQuery error %s\n",strerror(errno));
		close(cfd);
		goto err;
	}

	struct sockaddr_in *s = (struct sockaddr_in *)&sa;
	inet_ntop(AF_INET,(void*)&(s->sin_addr),cip,sizeof(cip));
	cport = ntohs(s->sin_port);
	//printf("accpet a link from client ip %s, port %d fd %d, clientnums %d\n",cip,cport,cfd,server.client_channel);

	//in fact the max_client is reached.
	if(server.client_channel > MAX_CLIENTS) {
		char *err = "-ERR max number of clients reached\r\n";
		write(cfd,err,strlen(err));
		close(cfd);
		goto err;
	}

	server.client_channel++;
	return;

err:
	close(cfd);
	destroyClient(newclient);
	return;
}

void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask) {
	client *c = (client*) privdata;
	int nread, readlen;

    readlen = PROTO_IOBUF_LEN;
	printf("fd->%d\n",c->fd);

	char buf[readlen];
	memset(buf,'\0',readlen);

	do {
		nread = read(fd, buf, readlen);
		printf("read %d byte from %d fd\n",nread,c->fd);
		if (nread == -1) {
			//if no buf is ready ,return to implement next part.
			if (errno == EAGAIN) {
				break;
			} else {
				printf("error connection ip %s port %d fd %d\n",c->client_ip,c->port,fd);
				close(fd);
				destroyClient(c);
				return;
			}
		} else if (nread == 0) {
			//0 means get the fin package, close the socket
			printf("Client closed connection ip %s port %d fd %d\n",c->client_ip,c->port,fd);
			close(fd);
			destroyClient(c);
			return;
		} else {
			c->inbuf = strnumcat(c->inbuf,buf,nread);
		}
	} while (nread == readlen);

	printf("len %d inlen %lu readbuf %s",nread,c->inlen,c->inbuf);
	
	_processCommand(c); 
	return;
	
	//printf("len %lu\r\n%s",getlen(c->inbuf),c->inbuf);
}

int _processCommand(void* tmp) {
	client* c = (client*)tmp;
	char *newline,*aux;
    int  j, linefeed_chars = 1;
    size_t querylen;

	printf("c->inbuf is %s\n",c->inbuf + c->inlen);
    newline = strchr(c->inbuf+c->inlen,'\n');

    // can't find /r/n means a request is too big can't read all of itvia a 16K buf
    if (newline == NULL) {
		printf("no enough package!\n");
        return ANET_ERR;
    }

    // find a command and implement it.
    if (newline && newline != c->inbuf+c->inlen && *(newline-1) == '\r')
        newline--, linefeed_chars++;

    // Split the input buffer up to the \r\n 
    querylen = newline-(c->inbuf+c->inlen);
    aux = newstrlen(c->inbuf+c->inlen,querylen);
	printf("get command %s %lu %d\n",aux,querylen,linefeed_chars);
	if(!aux) {
		return ANET_ERR;
	}

	char* result;

	//process real command protocol here
	int argc = 0,retnum = 0;

	//check result is malloc or not, if malloc
	//need free.
	int resultFree = 0;

	char** vector = parseCommand(aux, &argc);
	if(vector == NULL)
		result = PARSE_ERROR;
	else if(argc == 2 && strcmp(vector[0],"get") == 0) {
		char* ret = get(vector[1],server.sm);
		if(!ret)
			result = NOKEY_ERROR;
		else {
			result = ret;
		}
	} else if (argc == 3 && strcmp(vector[0],"set") == 0) {
		set(vector[1],vector[2],server.sm);
		result = SUCCESS;
	} else if (argc == 2 && strcmp(vector[0],"del") == 0) {
		retnum = del(vector[1],server.sm);
		if(retnum == -1)
			result = NOKEY_ERROR;
		else
			result = SUCCESS;
	} else if (argc == 1 && strcmp(vector[0],"scan") == 0) {
		result = scan(server.sm);
		resultFree = 1;
	} else {
		result = USAGE;
	}

	//printf("get result %s %lu %d\n",result,querylen,linefeed_chars);

	//delelte processed command in inbuf.
	c->inlen = c->inlen + querylen + linefeed_chars;

	if(c->inlen == strlen(c->inbuf)) {
		cleanstr(c->inbuf);
		c->inlen = 0;
	}

	//send reply to reply buf.
	_sendReply(c,result);

	if(resultFree)
		freestr(result);

    freestr(aux);
}

int _sendReply(void* tmp, char* buf) {
	client* c = (client*)tmp;
//	printf("send reply %s\n",buf);
	c->outbuf = strnumcat(c->outbuf,buf,strlen(buf));
	c->outbuf = strnumcat(c->outbuf,"\r\n",2);
	if (aeCreateFileEvent(server.el,c->fd,AE_WRITABLE,sendReplyToClient, c) == AE_ERR) { 
		return AE_ERR;
	}
	return AE_OK;
}

//TODO: acutually we can seperate scan and get method or check the number of
//wirting bytes.if reply is small, we can just send to the write fd.
//another point is if we feel the write buf is to big , we have to wait for a 
//long time, we can limit the transmit buf size every event.
void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask) {
	printf("enter function sendReplyToClient\n");
	client* c = (client*) privdata;
	int nwrite;

	nwrite = write(fd, c->outbuf + c->outlen,getlen(c->outbuf) - c->outlen);
	printf("after clean buflen %lu outlen %lu nwrite %d\n",getlen(c->outbuf),c->outlen,nwrite);
	if(nwrite == 0) {
		c->outlen = 0;
		cleanstr(c->outbuf);
		aeDeleteFileEvent(server.el,c->fd,AE_WRITABLE);
	} else if (nwrite < 0) {
		printf("write error %s\n",strerror(errno));
	} else {
		c->outlen += nwrite;
	}
}
