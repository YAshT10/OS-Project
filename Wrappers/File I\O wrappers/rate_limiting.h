#ifndef RATE_LIMITING_H
#define RATE_LIMITING_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#define MAX_OPERATIONS 10
#define TIME_WINDOW 60 // in seconds

typedef struct {
    int operations;
    time_t start_time;
} RateLimiter;

void rate_limiter_init(RateLimiter *limiter);
int rate_limiter_check(RateLimiter *limiter);
int rate_limited_open(RateLimiter *limiter, const char *pathname, int flags);
ssize_t rate_limited_read(RateLimiter *limiter, int fd, void *buf, size_t count);

#endif // RATE_LIMITING_H