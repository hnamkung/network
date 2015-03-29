#include "io_library.h"

// duplicate handle func
void add_duplicate_slash(char *buf, int *len)
{
    int i, j;
    for(i=0; i<*len; i++) {
        if(buf[i] == '\\') {
            for(j=*len-1; j>=i; j--) {
                buf[j+1] = buf[j];
            }
            *len = *len + 1;
            i++;
        }
    }
}

void remove_duplicate_slash(char *buf, int *len)
{
    int i, j;
    for(i=0; i<*len; i++) {
        if(buf[i] == '\\') {
            for(j=i+1; j<*len-1; j++) {
                buf[j] = buf[j+1];
            }
            *len = *len - 1;
        }
    }
}

// read write wrapper
void read_message(int fd, struct message *m)
{
    char buf[10];
    rio_read_nobuf(fd, buf, 8);

    m->op = buf[0];
    m->proto = buf[1];
    m->checksum = ( *(short *)(buf+2) );
    m->trans_id = ( *(int *)(buf+4) );

    m->checksum = ntohs( *(short *)(buf+2) );
    m->trans_id = ntohl( *(int *)(buf+4) );
}

void write_message(int fd, struct message *m)
{
    struct message m_copy = *m;

    m_copy.trans_id = htonl(m_copy.trans_id);
    set_checksum(&m_copy);
    
    rio_write_nobuf(fd, &m_copy, sizeof m_copy);
}

// read write without buffer, handle shortcount
ssize_t rio_read_nobuf(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;
    while(nleft > 0) {
        if( (nread = read(fd, bufp, nleft)) < 0) {
            return -1;
        }
        else if(nread == 0)
            break;
    //    printf("read_nobuf : %d\n", (int)nread);
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}


ssize_t rio_write_nobuf(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
        if( (nwritten = write(fd, bufp, nleft)) <= 0) {
            return -1;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

// read write with buffer
void rio_init(rio_t *rp, int fd)
{
    rp->fd = fd;
    rp->cnt = 0;
    rp->bufptr = rp->buf;
}
ssize_t read_string_from_stdin(int fd, char *usrbuf, size_t maxlen)
{
    ssize_t nread;

    nread = read(fd, usrbuf, maxlen);
    usrbuf[nread] = 0;
    return nread;
}

ssize_t read_string_from_server(rio_t *rp, char *usrbuf, int proto)
{
    if(proto == 1) {
        int i, nread;
        char c, *bufp = usrbuf;
        int toggle=0;

        for(i=1; ; i++) {
            if((nread = rio_read(rp, &c, 1)) == 1) {
                *bufp = c;
                if(*bufp == '\\') 
                    toggle = !toggle;
                if(toggle && *(bufp-1) == '\\' && *bufp == '0') {
                    break;
                }
                bufp++;
            }
            else if(nread == 0) {
               return -2; // error, could not complete string
            } else
                return -1;
        }
        return i-2;
    }
    else if(proto == 2) {
        char lenbuf[10];
        unsigned int len;
        rio_read_nobuf(rp->fd, lenbuf, 4);
        len = ntohl(*(unsigned int*)lenbuf); 

        rio_read_nobuf(rp->fd, usrbuf, len);
        return len;
    }
}

ssize_t rio_read_buf(rio_t *rp, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while(nleft > 0) {
        if((nread = rio_read(rp, bufp, nleft)) < 0) {
            return -1;
        }
        else if(nread == 0)
            break;
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}


ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;
//    printf("------rio_buffer start\n");
    while (rp->cnt <= 0) {
        rp->cnt = read(rp->fd, rp->buf, sizeof(rp->buf));
 //       printf("------cnt : %d\n", rp->cnt);
        if(rp->cnt < 0) {
            return -1; // error will not happen
        }
        else if(rp->cnt == 0)
            return 0; // EOF
        else
            rp->bufptr = rp->buf; // reset buffer ptr
    }

    cnt = n;
    if(rp->cnt < n)
        cnt = rp->cnt;
    memcpy(usrbuf, rp->bufptr, cnt);
    rp->bufptr += cnt;
    rp->cnt -= cnt;
  //  printf("------rio_buffer done\n");
    return cnt;
}

// calculate checksum
void set_checksum(struct message *m)
{
    //printf("set checksum\n");
    unsigned short checksum1, checksum2, checksum3;
    unsigned int checksum, carry;

    checksum1 = *(char *)m << 8;
    checksum1 += *((char *)m+1) & 0xff;
  //  printf("%x\n", checksum1);

    checksum2 = *((char *)m+4) << 8;
    checksum2 += *((char *)m+5) & 0xff;
 //   printf("%x\n", checksum2);

    checksum3 = *((char *)m+6) << 8;
    checksum3 += *((char *)m+7) & 0xff;
//    printf("%x\n", checksum3);

    checksum = checksum1 + checksum2 + checksum3;
    
    //printf("%x\n", checksum);

    while(checksum >> 16 != 0) {
       carry = checksum >> 16;
       checksum = checksum & 0xffff;
       checksum += carry;
    }
    m->checksum = ntohs(~checksum);
}

// for debug 
void printMessage(struct message *m)
{
    int j;

    char temp[3];
//    printf("test--\n");
//    printf("%p %p %p\n", temp, temp+1, temp+2);
//    printf("test--\n");

    for(j=0; j<8; j++) {
        char c = *((char *)m+j);

        if(j == 0)
            printf("op         (%p) : 0x%10x (", &m->op, (int)m->op);
        if(j == 1)
            printf(")\nproto      (%p) : 0x%10x (", &m->proto, (int)m->proto);
        if(j == 2)
            printf(")\nchecksum   (%p) : 0x%10x (", &m->checksum, m->checksum);
        if(j == 4)
            printf(")\ntrnas_id   (%p) : 0x%10x (", &m->trans_id, m->trans_id);
        //printf(".%p.", ((char *)m+j));
        printf(BPattern, BOrder(c));
    }

    printf(")\n\n");
}

