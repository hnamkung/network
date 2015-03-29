#include <stdio.h>
#include <stdlib.h>
#include "io_library.h"


// phase 1
void phase1(int fd_from_client, int proto);

// phase 2
void phase2(int fd_from_client, int proto);

// open socket
int open_clientfd(char* ip, int port);

void main(int argc, char **argv)
{
    char *ip;
    int port=-1, proto=-1;
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
    if(strlen(ip) == 0) {
        printf("no ip, error\n");
        return;
    }
    if(port == -1) {
        printf("no port, error\n");
        return;
    }
    if(proto != 1 && proto != 2) {
        printf("-m should be either 1 or 2\n");
        return;
    }

    fd_from_client = open_clientfd(ip, port);

    // phase 1 - hand shake first
    phase1(fd_from_client, proto);

    // phase 2
    phase2(fd_from_client, proto);

    close(fd_from_client);
}

// phase 1
void phase1(int fd, int proto)
{
    struct message read_m, write_m;
    read_message(fd, &read_m);

    write_m.op = 1;
    write_m.proto = proto;
    write_m.trans_id = read_m.trans_id;

    write_message(fd, &write_m);
}

// phase 2
void phase2(int fd, int proto)
{
    char buf[MAXLINE_BUFSIZE];
    int stdin_len, server_len;
    int count = 0;
    rio_t rio;
    rio_init(&rio, fd);

    if(proto == 1) {
        while(1) {
           stdin_len = read_string_from_stdin(STDIN_FILENO, buf, MAXLINE);
           if(stdin_len  > 0) {
               add_duplicate_slash(buf, &stdin_len);
               buf[stdin_len] = '\\';
               buf[stdin_len+1] = '0';
               stdin_len += 2;

               rio_write_nobuf(fd, buf, stdin_len);

               if((server_len = read_string_from_server(&rio, buf, proto)) < 0) {
                    printf("Error! read_string_from_server returned %d\n", server_len);
               }

               remove_duplicate_slash(buf, &server_len);

               int i; 
               for(i=0; i<server_len; i++)
                   printf("%c", buf[i]);
           }
           else if(stdin_len == 0) 
               break;
        }
    }

    if(proto == 2) {
        while(1) {
           stdin_len = read_string_from_stdin(STDIN_FILENO, buf+4, MAXLINE);
           if(stdin_len  > 0) {
               *(unsigned int*)buf = htonl(stdin_len);
                
               rio_write_nobuf(fd, buf, stdin_len+4);

               if((server_len = read_string_from_server(&rio, buf, proto)) < 0) {
                    printf("Error! read_string_from_server returned %d\n", server_len);
               }

               int i; 
               for(i=0; i<server_len; i++)
                   printf("%c", buf[i]);
           }
           else if(stdin_len == 0) 
               break;
        }
    }
}

// open socket
int open_clientfd(char *hostname, int port) 
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1; /* check errno for cause of error */

    /* Fill in the server's IP address and port */
    if ((hp = gethostbyname(hostname)) == NULL)
        return -2; /* check h_errno for cause of error */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0], 
              (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    /* Establish a connection with the server */
    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}

