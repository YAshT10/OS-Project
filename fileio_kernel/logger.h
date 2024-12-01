#ifndef LOG_H
#define LOG_H
#define _GNU_SOURCE

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/slab.h>

#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02

void log_message(const char *message);
int open_logger(const char *pathname, int flags, mode_t mode);
ssize_t read_logger(int fd, void *buf, size_t count);
ssize_t write_logger(int fd, const void *buf, size_t count);
int close_logger(int fd);

#endif // LOG_H
