#include "buffer.h"
#include "control_permission.h"
#include "logger.h"
#include "rate_limiting.h"
#include "Safe_open.h"
#include "Safe_read.h"

// Function to open a file with controlled and rate-limited access
RateLimiter rateLimiter = {0, 0};
Buffer buffer = {0, -1, 0};

int open(const char *pathname, int flags, mode_t mode) {
    
    if(rateLimiter.start_time == 0) {
        rate_limiter_init(&rateLimiter);
    }
    if (check_permission(pathname) != -1) {
        int fd = rate_limited_open(&rateLimiter, pathname, flags, mode);
        if(fd == -1) {
            return -1;
        }
        else {
            if(buffer.fd != -1) {
                buffer_flush(&buffer);
            }
            buffer_init(&buffer, fd);
            return fd;
        }
    } else {
        log_message("Permission denied for file - open");
        return -1;
    }
}

// Function to read from a file with rate limiting
ssize_t read(int fd, void *buf, size_t count) {
    if(rateLimiter.start_time == 0) {
        rate_limiter_init(&rateLimiter);
    }
    if (check_read_fd(fd)) {
        if(buffer.fd == fd) {
            buffer_flush(&buffer);
        }
        return rate_limited_read(&rateLimiter, fd, buf, count);
    } else {
        return -1;
    }
}

// Function to write to a file
ssize_t write(int fd, const void *buf, size_t count) {
    if (check_write_fd(fd)) {
        if(buffer.fd == fd) {
            if(count >= BUFFER_SIZE) {
                buffer_flush(&buffer);
                write_logger(fd, buf, count);
            }
            else
            buffer_add(&buffer, buf, count);
            
            return count;
        }
        else {
            if(count < buffer.size || count >= BUFFER_SIZE) {
                return write_logger(fd, buf, count);
            }
            else {
                buffer_flush(&buffer);
                buffer_init(&buffer, fd);
                buffer_add(&buffer, buf, count);
                return count;
            }
        }
    } else {
        return -1;
    }
    
}

// Function to close a file
int close(int fd) {
    if(buffer.fd == fd) {
        buffer_flush(&buffer);
        buffer.fd = -1;
    }
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