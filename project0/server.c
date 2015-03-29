#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 8192
#define MAXLINE_BUFSIZE 20000 
#define RIO_BUFSIZE 300

#define BPattern "%d%d%d%d%d%d%d%d"
#define BOrder(byte)  \
      (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0) 

// struct for protocol
struct message {
    char op;
    char proto;
    unsigned short checksum;
    unsigned int trans_id;
};

// struct for io
typedef struct sockaddr SA;
typedef struct {
    int fd;
    int cnt;
    char *bufptr;
    char buf[RIO_BUFSIZE];
}rio_t;

// interrupt handler
void sigchld_handler(int sig);

// open listenfd
int open_listenfd(int port);

int open_listenfd(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
          
    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;
 
    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0)
        return -1;

    /* Listenfd will be an endpoint for all requests to port
              on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, 1024) < 0)
        return -1;
    return listenfd;
}


void main(int argc, char **argv)
{
    int port, i;
    int listenfd, connfd;
    struct sockaddr_in clientaddr;
    socklen_t clientlen; 

    for(i=1; i<argc; i++) {
    	if(!strcmp(argv[i], "-p")) {
            port = atoi(argv[++i]);
        }
    }

    printf("port : %d\n", port);

    signal(SIGCHLD, sigchld_handler);
    listenfd = open_listenfd(port);
    printf("listenfd : %d\n", listenfd);
    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        printf("connfd : %d\n", connfd);

//        if(fork() == 0) {
//            close(listenfd);
//            // use connfd
//            printf("new connection : %d\n", connfd);
//            close(connfd);
//            exit(0);
//        }
        close(connfd);
    }
//    fd_from_client = open_clientfd(ip, port);
//
//    // phase 1 - hand shake first
//    phase1(fd_from_client, proto);
//
//    // phase 2
//    phase2(fd_from_client, proto);

}

void sigchld_handler(int sig)
{
    while(waitpid(-1, 0, WNOHANG) > 0)
        ;
    return;
}



