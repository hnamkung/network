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
#define MAXLINE_BUFSIZE 1000000 
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

// duplicate handle func
void add_duplicate_slash(char *buf, int *len);
void remove_duplicate_slash(char *buf, int *len);

// read write wrapper
void read_message(int fd, struct message *m);
void write_message(int fd, struct message *m);

// read write without buffer, handle shortcount
ssize_t rio_read_nobuf(int fd, void *usrbuf, size_t n);
ssize_t rio_write_nobuf(int fd, void *usrbuf, size_t n);

// read write with buffer
void rio_init(rio_t *rp, int fd);
ssize_t read_string_from_stdin(int fd, char *usrbuf, size_t maxlen);
ssize_t read_string_from_server(rio_t *rp, char *usrbuf, int proto);
ssize_t rio_read_buf(rio_t *rp, void *usrbuf, size_t n);
ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);

// calculate checksum
void set_checksum(struct message *m);

// for debug
void printMessage(struct message *m);


