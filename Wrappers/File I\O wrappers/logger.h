#ifndef LOG_H
#define LOG_H
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <dlfcn.h>
#include <fcntl.h>
void log_message(const char *message);
int open_logger(const char *pathname, int flags, mode_t mode);
ssize_t read_logger(int fd, void *buf, size_t count);
ssize_t write_logger(int fd, const void *buf, size_t count);
int close_logger(int fd);

#endif // LOG_H