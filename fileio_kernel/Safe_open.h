#ifndef SAFE_OPEN_H
#define SAFE_OPEN_H


void check_path(const char* path);
int safe_open(const char* filename, int flags , mode_t mode);

#endif