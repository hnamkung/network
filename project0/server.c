#include <stdio.h>
#include <stdlib.h>
#include "io_library.h"


// connection_start
void handle_connection(int connfd, unsigned int trans_id);

// process duplicate char, server job
void process_duplicate_char(char *buf, int *len, char *previous, int start);

// interrupt handler
void sigchld_handler(int sig);

// open listenfd
int open_listenfd(int port);


void main(int argc, char **argv)
{
    int port, i;
    int listenfd, connfd;
    struct sockaddr_in clientaddr;
    unsigned int trans_id=0;
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
        
        trans_id++;

        if(fork() == 0) {
            close(listenfd);
            // use connfd
            printf("new connection : %d\n", connfd);
            handle_connection(connfd, trans_id);
            close(connfd);
            exit(0);
        }
        close(connfd);
    }
}

void handle_connection(int connfd, unsigned int trans_id)
{
    struct message m, mread;
    int proto;

    // phase 1
    m.op = 0;
    m.proto = 0;
    m.trans_id = htonl(trans_id);
    write_message(connfd, &m);

    read_message(connfd, &mread);
    
    if(m.trans_id != mread.trans_id) {
        printf("trans_id error, disconnect %d\n", connfd);
        return;
    }
    if(mread.op != 1) {
        printf("op not 1 error, disconnect %d\n", connfd);
        return;
    }
    proto = mread.proto;

    char buf[MAXLINE_BUFSIZE];
    char previous;
    int start=0;
    rio_t rio;
    rio_init(&rio, connfd);
    int client_len;

    // phase 2
    if(proto == 1) {
        while(1) {
            if((client_len = read_string_from_server(&rio, buf, proto)) > 0) {
                remove_duplicate_slash(buf, &client_len);     

                process_duplicate_char(buf, &client_len, &previous, start);
                start = 1; 
                add_duplicate_slash(buf, &client_len);
                buf[client_len] = '\\';
                buf[client_len+1] = '0';
                client_len += 2;
                rio_write_nobuf(connfd, buf, client_len);
            }
        }
    }
    else if(proto == 2) {
        while(1) {
            if((client_len = read_string_from_server(&rio, buf+4, proto)) > 0) {
                process_duplicate_char(buf+4, &client_len, &previous, start);
                start = 1;

                *(unsigned int*)buf = htonl(client_len);

                rio_write_nobuf(connfd, buf, client_len+4);
            }
        }
    }
}

void process_duplicate_char(char *buf, int *len, char *previous, int start)
{
    if(*len == 0)
        return;

    int i, newlen=-1;
    char newbuf[MAXLINE_BUFSIZE];

    if(start == 0 || *previous != *buf) {
        newlen++;
        *newbuf = *buf;
    }
    for(i=1; i<*len; i++) {
        if(buf[i-1] != buf[i]) {
            newlen++;
            newbuf[newlen] = buf[i];
        }
        *previous = buf[i];
    }
    newlen++;
    *len = newlen;
    memcpy(buf, newbuf, newlen);
}
// interrupt handler
void sigchld_handler(int sig)
{
    while(waitpid(-1, 0, WNOHANG) > 0)
        ;
    return;
}

// open listenfd
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


