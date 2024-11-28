#ifndef CONTROL_PERMISSION_H
#define CONTROL_PERMISSION_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>

int check_permission(const char *pathname);
int controlled_open(const char *pathname, int flags);

#endif // CONTROL_PERMISSION_H