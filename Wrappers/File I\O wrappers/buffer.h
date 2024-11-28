#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "logger.h"

#define BUFFER_SIZE 1024

typedef struct {
    char data[BUFFER_SIZE];
    int fd;
    size_t size;
} Buffer;

void buffer_init(Buffer *buffer , int fd);
void buffer_add(Buffer *buffer, const char *data, size_t data_size);
void buffer_flush(Buffer *buffer);

#endif // BUFFER_H