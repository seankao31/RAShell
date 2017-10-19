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

void log(const std::string str) {
    writen(sockfd, str.c_str(), str.length());
}

void log(const char* str) {
    writen(sockfd, str, strlen(str));
    writen(sockfd, str, strlen(str) * sizeof(char));
}

struct command {
    char *args[ARGSIZE];
    struct command_segment *next;
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
    int argc = 0;
    segment = strsep(&sep, "|");
    //for (argc = 0; argc < ARGSIZE && (cur->args[argc] = strtok(segment, TOKEN_DELIMITERS)) != NULL; argc++)
        //segment = NULL;
    //cur->args[argc] = NULL;
    //log(sep);
    log(segment);

    //while ((segment = strsep(&sep, "|")) != NULL) {
        //struct command_segment *cmd_seg = malloc(sizeof(struct command_segment));
        //cur = cur->next = cmd_seg;
        //for (argc = 0; argc < ARGSIZE - 1 && (cur->args[argc] = strtok(segment, TOKEN_DELIMITERS)) != NULL; argc++)
            //segment = NULL;
        //cur->args[argc] = NULL;
    //}
    //cur->next = NULL;

    //delete[] sep;
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
