#pragma once
#include <errno.h>
#include <cstdlib>
#include <unistd.h>
#include <string>

#define MAXLINE 10001

int read_cnt = 0;
char *read_ptr;
char read_buf[MAXLINE];

int bufread(int sockfd, char *ptr) {
    while (read_cnt <= 0) {
        if ((read_cnt = read(sockfd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        else if (read_cnt == 0)
            return 0;
        read_ptr = read_buf;
    }
    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

int _readline(int sockfd, char *vptr, size_t maxlen) {
    char *ptr = vptr;
    char c;
    ssize_t rc, n;
    for (n = 1; n < maxlen; n++) {
        if ((rc = bufread(sockfd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0) {
            break;
        }
        else
            return -1;
    }
    *ptr = '\0';
    return 0;
}

int myreadline(int sockfd, std::string &str) {
    char cstr[MAXLINE];
    if (_readline(sockfd, cstr, sizeof(read_buf)) == 0) {
        str = std::string(cstr);
        return 0;
    }
    return -1;
}

ssize_t writen(int sockfd, const char *vptr, size_t n) {
    size_t left = n;
    ssize_t written;
    const char *ptr = vptr;

    while (left > 0) {
        if ((written = write(sockfd, ptr, left)) <= 0) {
            if (written < 0 && errno == EINTR) {
                written = 0;
            }
            else {
                return -1;
            }
        }

        left -= written;
        ptr += written;
    }
    return n;
}
