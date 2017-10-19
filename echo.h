#pragma once
#include <iostream>
#include <string>
#include "serverlib.h"

void str_echo(int sockfd) {
    int r;
    std::string line;
    while (true) {
        r = myreadline(sockfd, line);
        if (r < 0) {
            std::cerr << "str_echo: readline error" << std::endl;
            return;
        }
        if (writen(sockfd, line.c_str(), line.length()) != line.length()) {
            std::cerr << "str_echo: writen error" << std::endl;
        }
    }
}
