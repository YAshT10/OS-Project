#include "buffer.h"
#include "control_permission.h"
#include "logger.h"
#include "rate_limiting.h"
#include "Safe_open.h"
#include "Safe_read.h"

// Function to open a file with controlled and rate-limited access
RateLimiter rateLimiter = {0, 0};

int my_open(const char *pathname, int flags, mode_t mode) {
    if(rateLimiter.start_time == 0) {
        rate_limiter_init(&rateLimiter);
    }
    if (check_permission(pathname)) {
        return rate_limited_open(&rateLimiter, pathname, flags, mode);
    } else {
        log_message("Permission denied for file");
        return -1;
    }
}

// Function to read from a file with rate limiting
ssize_t my_read(int fd, void *buf, size_t count) {
    if(rateLimiter.start_time == 0) {
        rate_limiter_init(&rateLimiter);
    }
    return rate_limited_read(&rateLimiter, fd, buf, count);
}

// Function to write to a file
ssize_t my_write(int fd, const void *buf, size_t count) {
    return write_logger(fd, buf, count);
}

// Function to close a file
int my_close(int fd) {
    return close_logger(fd);
}

// Function to safely open a file
int my_safe_open(const char* filename, int flags, mode_t mode) {
    return safe_open(filename, flags, mode);
}

// Function to safely read from a file
ssize_t my_safe_read(int fd, void *buf, size_t count) {
    return safe_read(fd, buf, count);
}

SYSCALL_DEFINE3(my_open, const char*, pathname, int, flags, mode_t, mode) {
    return my_open(pathname, flags, mode);
}

SYSCALL_DEFINE3(my_read, int, fd, void*, buf, size_t, count) {
    return my_read(fd, buf, count);
}

SYSCALL_DEFINE3(my_write, int, fd, const void*, buf, size_t, count) {
    return my_write(fd, buf, count);
}

SYSCALL_DEFINE1(my_close, int, fd) {
    return my_close(fd);
}

SYSCALL_DEFINE3(my_safe_open, const char*, filename, int, flags, mode_t, mode) {
    return my_safe_open(filename, flags, mode);
}

SYSCALL_DEFINE3(my_safe_read, int, fd, void*, buf, size_t, count) {
    return my_safe_read(fd, buf, count);
}