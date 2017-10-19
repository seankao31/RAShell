#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "serverlib.h"

#define COMMAND_BUFSIZE 256
#define TOKEN_DELIMITERS " \t\r\n\a"
#define ARGSIZE 128

int sockfd;

struct command_segment {
    char *args[ARGSIZE];
    struct command_segment *next;
};

struct command {
    struct command_segment *root;
    int pipe_to;
};

void welcome() {
    std::string greet = "****************************************\n"
                        "** Welcome to the information server. **\n"
                        "****************************************\n";
    writen(sockfd, greet.c_str(), greet.length());
}

void prompt() {
    writen(sockfd, "% ", 2);
}

void err_dump(const std::string str) {
    writen(sockfd, str.c_str(), str.length());
}

void err_dump(const char* str) {
    writen(sockfd, str, strlen(str));
}

void loop() {
    std::string line;
    struct command *command;
    int status = 0;

    do {
        prompt();
        myreadline(sockfd, line);
        if (line.length() == 0) {
            continue;
        }
        err_dump(line);
        err_dump(line.c_str());
    } while (status >= 0);
}

int shell(int fd) {
    sockfd = fd;
    welcome();
    loop();
    return 0;
}
