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

void main(int argc, char **argv)
{
    char *ip;
    int port, proto;
    int fd_from_client;
    int i;

    for(i=1; i<argc; i++) {
        if(!strcmp(argv[i], "-h")) {
            ip = argv[++i];
        }
    	if(!strcmp(argv[i], "-p")) {
            port = atoi(argv[++i]);
        }
        if(!strcmp(argv[i], "-m")) {
            proto = atoi(argv[++i]); 
        }
    }

//    fd_from_client = open_clientfd(ip, port);
//
//    // phase 1 - hand shake first
//    phase1(fd_from_client, proto);
//
//    // phase 2
//    phase2(fd_from_client, proto);

    close(fd_from_client);
}


