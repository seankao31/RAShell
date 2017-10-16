#pragma once
#include <unistd.h>

ssize_t readn(int, char*, size_t);
ssize_t writen(int, const char*, size_t);
ssize_t readline(int, char*, size_t);
