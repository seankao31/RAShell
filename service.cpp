#include "serverlib.h"
#include <iostream>

#define MAXLINE 512

void str_echo(int sockfd) {
    int n;
    char line[MAXLINE];
    while (true) {
        n = readline(sockfd, line, MAXLINE);
        if (n == 0)
            return;
        else if (n < 0) {
            std::cerr << "str_echo: readline error" << std::endl;
        }
        if (writen(sockfd, line, n) != n) {
            std::cerr << "str_echo: writen error" << std::endl;
        }
    }
}
