#ifndef CONTROL_PERMISSION_H
#define CONTROL_PERMISSION_H

int check_permission(const char *pathname);
int controlled_open(const char *pathname, int flags);

#endif // CONTROL_PERMISSION_H
