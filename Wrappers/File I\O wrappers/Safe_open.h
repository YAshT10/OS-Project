#ifndef SAFE_OPEN_H
#define SAFE_OPEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>


void check_path(const char* path);
FILE* safe_open(const char* filename, const char* mode);

#endif