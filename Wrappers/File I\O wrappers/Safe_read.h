#ifndef SAFE_READ_H
#define SAFE_READ_H

#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

ssize_t safe_read(int fd, void *buf, size_t count);

#endif // SAFE_READ_H