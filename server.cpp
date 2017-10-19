#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <string>
#include <iostream>
#include <errno.h>

#include "serverlib.h"
#include "shell.h"

#define SERV_TCP_PORT 2020

int main(int argc, char const* argv[])
{
    int sockfd, newsockfd, childpid;
    unsigned clilen;
    struct sockaddr_in cli_addr, serv_addr;

    // Open a TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "server: can't open stream socket" << std::endl;
    }

    // Bind our local address so that client can send to us
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_TCP_PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "server: can't bind local address" << std::endl;
    }

    listen(sockfd, 5);

    while(true) {
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            std::cerr << "server: accept error" << std::endl;
        }
        if ((childpid = fork()) < 0) {
            std::cerr << "server: fork error" << std::endl;
        }
        else if (childpid == 0) {
            close(sockfd);
            shell(newsockfd);
            exit(0);
        }
        close(newsockfd);
    }

    return 0;
}
