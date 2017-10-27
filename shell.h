#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#include "serverlib.h"

#define COMMAND_BUFSIZE 256
#define TOKEN_DELIMITERS " \t\r\n\a"
#define ARGSIZE 128
#define PIPE_CAPACITY 65536

int sockfd;

std::string pipe_buf[1000];
std::string ordinary_pipe_buf;
int cmd_count;


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
    writen(sockfd, "\n", 1);
}

void err_dump(const char* str) {
    if (str == NULL) {
        writen(sockfd, "", 0);
        return;
    }
    writen(sockfd, str, strlen(str));
    writen(sockfd, "\n", 1);
}

void log(const int i) {
    char numstr[10];
    sprintf(numstr, "%d", i);
    std::cerr << "log: " << numstr << std::endl;
    writen(sockfd, numstr, strlen(numstr));
    writen(sockfd, "\n", 1);
}

void log(const std::string str) {
    std::cerr << "log: " << str << std::endl;
    writen(sockfd, str.c_str(), str.length());
    writen(sockfd, "\n", 1);
}

void log(const char* str) {
    if (str == NULL) {
        std::string errmsg = "trying to log NULL string\n";
        std::cerr << "log: " << errmsg << std::endl;
        writen(sockfd, "", 0);
        return;
    }
    std::cerr << "log: " << str << std::endl;
    writen(sockfd, str, strlen(str));
    writen(sockfd, "\n", 1);
}

void client_output(const std::string str) {
    writen(sockfd, str.c_str(), str.length());
}

void client_output(const char* str) {
    if (str == NULL) {
        writen(sockfd, "", 0);
        return;
    }
    writen(sockfd, str, strlen(str));
}

struct command {
    char *args[ARGSIZE];
    struct command *next;
    int pipe_to; // 0 for ordinary pipe, -1 for stdout, n for special pipe
    bool write_file;
    char *file_name;
};

struct command* parse_command(std::string line) {
    //struct command *cmd = new struct command();
    struct command *root = new struct command();
    struct command *cur = root;

    char *sep = new char[line.length() + 1];
    std::copy(line.begin(), line.end(), sep);
    sep[line.length()] = '\0';
    char *segment;
    int pipe_to;
    int argc;

    cur->pipe_to = -1; // -1 for stdout
    cur->write_file = 0;
    cur->file_name = NULL;
    segment = strtok(sep, TOKEN_DELIMITERS);
    for (argc = 0; argc < ARGSIZE; argc++) {
        if (segment != NULL && (segment[0] == '|' || segment[0] == '>')) {
            cur->args[argc] = NULL;
            break;
        }
        cur->args[argc] = segment;
        if (cur->args[argc] == NULL)
            break;
        segment = strtok(NULL, TOKEN_DELIMITERS);
    }

    while (segment != NULL) {
        if (segment[0] == '|') {
            if (segment[1] == '\0') {
                pipe_to = 0; // 0 for ordinary pipe
            }
            else {
                char junk;
                sscanf(segment, "%c%d",&junk, &pipe_to);
            }
            cur->pipe_to = pipe_to;
            segment = strtok(NULL, TOKEN_DELIMITERS);
        }
        else if (segment[0] == '>') {
            cur->write_file = 1;
            cur->file_name = strtok(NULL, TOKEN_DELIMITERS);
            break;
        }
        else {
            struct command *cmd = new struct command();
            cur = cur->next = cmd;
            cur->pipe_to = -1; // -1 for stdout
            cur->write_file = 0;
            cur->file_name = NULL;
            for (argc = 0; argc < ARGSIZE; argc++) {
                if (segment != NULL && (segment[0] == '|' || segment[0] == '>')) {
                    cur->args[argc] = NULL;
                    break;
                }
                cur->args[argc] = segment;
                if (cur->args[argc] == NULL)
                    break;
                segment = strtok(NULL, TOKEN_DELIMITERS);
            }
        }
    }

    // for (cur = root; cur != NULL; cur = cur->next) {
    //     for (argc = 0; argc < 10; argc++) {
    //         std::cout << cur->args[argc] << std::endl;
    //         if(cur->args[argc] == NULL)
    //             break;
    //     }
    //     if (cur->write_file) {
    //         std::cout << "write to " << cur->file_name << std::endl;
    //     }
    //     else {
    //         std::cout << "pipe to: " << cur->pipe_to << std::endl;
    //     }
    // }

    return root;
}

void execute_exit() {
    exit(0);
}

int execute_single_command(struct command *command, int in_fd, int out_fd) {
// int execute_single_command(struct command *command, int out_fd_tochild, int in_fd_fromchild = -1, int out_fd_fromchild = -1) {
    if (command->args[0] == NULL) {
        // do nothing (empty command)
        return 0;
    }

    for (int i = 0; i < ARGSIZE; i++) {
        if (command->args[i] == NULL)
            break;
        std::string str(command->args[i]);
        if (str.find('/') != std::string::npos) {
            err_dump("command should not contain character '/'");
            return 0;
        }
    }

    std::cout << "command count for [" << command->args[0] << "]: " << cmd_count << std::endl;

    int status = 0;

    // built in command
    if (strcmp(command->args[0], "exit") == 0) {
        cmd_count = (cmd_count + 1) % 1000;
        execute_exit();
    }
    else if (strcmp(command->args[0], "printenv") == 0) {
        char* pPath;
        if (command->args[1] == NULL) {
            err_dump(std::string("incorrect argument number: [") + command->args[0] + "]");
            return -1;
        }

        cmd_count = (cmd_count + 1) % 1000;

        pPath = getenv(command->args[1]);
        std::string path;
        if (pPath == NULL) {
            path = "";
        }
        else {
            path = pPath;
        }
        client_output(std::string(command->args[1]) + "=" + path + "\n");
        status = 0;
    }
    else if (strcmp(command->args[0], "setenv") == 0) {
        if (command->args[1] == NULL || command->args[2] == NULL) {
            err_dump(std::string("incorrect argument number: [") + command->args[0] + "]");
            return -1;
        }

        cmd_count = (cmd_count + 1) % 1000;
        status = setenv(command->args[1], command->args[2], 1);
    }
    else {
        pid_t pid;
        switch (pid = fork()) {
            case -1:
                perror("fork");
                break;
            case 0:
                std::cout << std::string("Command [") + command->args[0] + "] executed by pid=" + std::to_string(getpid()) << std::endl;
                dup2(in_fd, 0);
                if (out_fd == 1) {
                    dup2(sockfd, 1);
                }
                else {
                    dup2(out_fd, 1);
                }
                dup2(sockfd, 2);

                if (in_fd != 0) {
                    close(in_fd);
                }
                if (out_fd != 1) {
                    close(out_fd);
                }

                if (execvp(command->args[0], command->args) == -1) {
                    if (errno == ENOENT) {
                        err_dump(std::string("Unknown command: [") + command->args[0] + "].");
                    }
                    else {
                        err_dump(strerror(errno));
                    }
                    exit(1);
                }
            default:
                if (waitpid(pid, &status, 0) == -1) {
                    err_dump(strerror(errno));
                }
                if (status != 0) {
                    status = -1;
                }
                else {
                    cmd_count = (cmd_count + 1) % 1000;
                }
        }
    }

    return status;
}

int execute_command(struct command *command) {
    struct command *cur;
    int status = 0;
    int in = 0, fd_tochild[2], fd_fromchild[2];
    // in is used to indicate if input is stdin
    // in can NOT be used as file descriptor

    for (cur = command; cur != NULL && status != -1; cur = cur->next) {
        bool write_file;
        char *file_name;
        pipe(fd_tochild);
        pipe(fd_fromchild);

        // generate input for child
        std::string input = pipe_buf[cmd_count];
        if (in != 0) {
            input += ordinary_pipe_buf;
            // TODO: somewhere clear this buf
        }
        // char readbuf[PIPE_CAPACITY];
        // read(in, readbuf, sizeof(readbuf));
        // input += std::string(readbuf);
        log("input: ");
        log(input);
        write(fd_tochild[0], input.c_str(), input.length() + 1);

        // file
        if (cur->write_file) {
            int filefd = creat(cur->file_name, 0644);
            status = execute_single_command(cur, fd_tochild[0], filefd);
            // status = execute_single_command(cur, in, filefd);

            close(fd_tochild[0]);
            close(fd_tochild[1]);
            close(fd_fromchild[0]);
            close(fd_fromchild[1]);
        }
        // ordinary pipe
        else if (cur->pipe_to == 0) {
            status = execute_single_command(cur, fd_tochild[0], fd_fromchild[1]);

            char readbuf[PIPE_CAPACITY];
            read(fd_fromchild[0], readbuf, sizeof(readbuf));
            ordinary_pipe_buf = readbuf;
            log("ordinary_pipe_buf: ");
            log(ordinary_pipe_buf);

            in = fd_fromchild[0];

            close(fd_tochild[0]);
            close(fd_tochild[1]);
            close(fd_fromchild[0]);
            close(fd_fromchild[1]);
        }
        // stdout
        else if (cur->pipe_to == -1) {
            status = execute_single_command(cur, fd_tochild[0], 1);
            close(fd_tochild[0]);
            close(fd_tochild[1]);
            close(fd_fromchild[0]);
            close(fd_fromchild[1]);
        }
        // pipe n
        else {

        }


        std::cout << "status: " << status << std::endl;
    }
    // just leaved for loop with status == -1 due to first command
    if (status == -1 && cur == command->next) {
        cmd_count = (cmd_count + 1) % 1000;
    }

    return status;
}

void loop() {
    std::string line;
    struct command *command;
    int status = 0;

    while (status >= 0) {
        prompt();
        myreadline(sockfd, line);
        if (line.length() == 0) {
            continue;
        }
        command = parse_command(line.c_str());
        status = execute_command(command);
        if (status == -1) {
            std::cerr << "command execution stopped" << std::endl;
            status = 0;
        }
    }
}

void init() {
    char *home;
    home = getenv("HOME");
    chdir(home);
    chdir("ras");
    setenv("PATH", "bin:.", 1);
    cmd_count = 0;
}

int shell(int fd) {
    sockfd = fd;
    init();
    welcome();
    loop();
    return 0;
}
