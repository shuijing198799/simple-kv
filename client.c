#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MAXLINE 1024 * 1024

#define NORMOUSLINE 256 * 1024 * 1024

int max(int a, int b) {
	return a > b ? a : b;
}

void clientMain(FILE *fp, int sockfd) {
	int maxfdp1,nread;   
	fd_set rset;
	char sendline[MAXLINE],recvline[MAXLINE];
	FD_ZERO(&rset);
	for( ; ;) {
		memset(recvline,'\0',MAXLINE);
		memset(sendline,'\0',MAXLINE);
		FD_SET(fileno(fp), &rset);
		FD_SET(sockfd, &rset);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		select(maxfdp1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(sockfd, &rset)) {
			do {
				nread = read(sockfd,recvline, MAXLINE);
				if (nread==0) {
					close(sockfd);
					exit(0);
				}
				fputs(recvline, stdout);
			} while(nread == MAXLINE);
		}
		if(FD_ISSET(fileno(fp), &rset)){
			if(fgets(sendline, MAXLINE, fp)==0)
				return;
			write(sockfd, sendline, strlen(sendline));
		}
	}
}

int main(int argc, int** argv) {
	int fd;
	struct sockaddr_in seraddr;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("creating socket: %s\n", strerror(errno));
		exit(0);
	}

	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(9876);
	seraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if(connect(fd, (const struct sockaddr *)&seraddr, sizeof(seraddr)) == -1) {
		printf("connecting socket: %s\n", strerror(errno));
		exit(0);
	}

	clientMain(stdin,fd);
}
