#include <stdlib.h>
#include <stdio.h>
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
    if (str == NULL) {
        std::string errmsg = "trying to err_dump NULL string\n";
        writen(sockfd, errmsg.c_str(), errmsg.length());
        return;
    }
    writen(sockfd, str, strlen(str));
}

void log(const std::string str) {
    writen(sockfd, str.c_str(), str.length());
}

void log(const char* str) {
    if (str == NULL) {
        std::string errmsg = "trying to log NULL string\n";
        writen(sockfd, errmsg.c_str(), errmsg.length());
        return;
    }
    writen(sockfd, str, strlen(str));
}

struct command {
    char *args[ARGSIZE];
    struct command *next;
    int pipe_to;
};

struct command* parse_command(std::string line) {
    //struct command *cmd = new struct command();
    struct command *root = new struct command();
    struct command *cur = root;

    char *sep = new char[line.length() + 1];
    std::copy(line.begin(), line.end(), sep);
    sep[line.length()] = '\0';
    char *segment;
    int argc;
    int pipe_to;

    segment = strsep(&sep, "|");
    //log(sep);
    if (sep && sep[0] != ' ' && sep[0] != '\n') {
        sscanf(sep, "%d", &pipe_to);

        //char numstr[10];
        //sprintf(numstr, "%d", pipe_to);
        //log(numstr);
        //log("\n");
    }
    else
        pipe_to = 1;
    for (argc = 0; argc < ARGSIZE; argc++) {
        cur->args[argc] = strtok(segment, TOKEN_DELIMITERS);
        if (cur->args[argc] == NULL)
            break;
        segment = NULL;
    }

    while ((segment = strsep(&sep, "|")) != NULL) {
        //struct command_segment *cmd_seg = malloc(sizeof(struct command_segment));
        //log(sep);
        if (sep && sep[0] != ' ' && sep[0] != '\n') {
            sscanf(sep, "%d", &pipe_to);

            //char numstr[10];
            //sprintf(numstr, "%d", pipe_to);
            //log(numstr);
            //log("\n");
        }
        else
            pipe_to = 1;
        struct command *cmd = new struct command();
        cur = cur->next = cmd;
        for (argc = 0; argc < ARGSIZE; argc++) {
            cur->args[argc] = strtok(segment, TOKEN_DELIMITERS);
            if (cur->args[argc] == NULL)
                break;
            segment = NULL;
        }
    }
    cur->next = NULL;

    delete[] sep;
    return root;
}

int execute_command(struct command *command) {
    return 0;
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
        command = parse_command(line.c_str());
        status = execute_command(command);
    } while (status >= 0);
}

int shell(int fd) {
    sockfd = fd;
    welcome();
    loop();
    return 0;
}
