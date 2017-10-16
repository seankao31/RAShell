#include <errno.h>
#include <cstdlib>
#include <unistd.h>

ssize_t readn(int sockfd, char *vptr, size_t n) {
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nread = read(sockfd, ptr, nleft)) < 0) {
            if (errno == EINTR) {
                nread = 0;
            }
            else {
                return -1;
            }
        }
        else if (nread == 0)
            break;

        nleft -= nread;
        ptr += nread;
    }
    return n - nleft;
}

ssize_t writen(int sockfd, const char *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(sockfd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR) {
                nwritten = 0;
            }
            else {
                return -1;
            }
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

// TODO: don't read one byte at a time
ssize_t readline(int sockfd, char *vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ((rc = read(sockfd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0) {
            *ptr = 0;
            return n-1;
        }
        else {
            if (errno == EINTR)
                n--;
            else
                return -1;
        }
    }
    *ptr = 0;
    return n;
}
