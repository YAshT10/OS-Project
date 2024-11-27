#include "buffer.h"
#include "control_permission.h"
#include "logger.h"
#include "rate_limiting.h"
#include "Safe_open.h"
#include "Safe_read.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// Function to open a file with controlled and rate-limited access
int my_open(const char *pathname, int flags) {
    if (check_permission(pathname)) {
        RateLimiter rateLimiter;
        rate_limiter_init(&rateLimiter);
        return rate_limited_open(&rateLimiter, pathname, flags);
    } else {
        log_message("Permission denied for file");
        return -1;
    }
}

// Function to read from a file with rate limiting
ssize_t my_read(int fd, void *buf, size_t count) {
    RateLimiter rateLimiter;
    rate_limiter_init(&rateLimiter);
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
FILE* my_safe_open(const char* filename, const char* mode) {
    return safe_open(filename, mode);
}

// Function to safely read from a file
ssize_t my_safe_read(int fd, void *buf, size_t count) {
    return safe_read(fd, buf, count);
}

int main() {
    // Initialize buffer
    Buffer buffer;
    buffer_init(&buffer);

    // Open file
    const char *filename = "example.txt";
    int fd = my_open(filename, O_RDONLY);
    if (fd == -1) {
        log_message("Failed to open file with my_open");
        return EXIT_FAILURE;
    }

    // Read from file
    char read_buf[BUFFER_SIZE];
    ssize_t bytes_read = my_read(fd, read_buf, sizeof(read_buf));
    if (bytes_read == -1) {
        log_message("Failed to read file with my_read");
        my_close(fd);
        return EXIT_FAILURE;
    }

    // Add data to buffer
    buffer_add(&buffer, read_buf, bytes_read);

    // Flush buffer
    buffer_flush(&buffer);

    // Close file
    if (my_close(fd) == -1) {
        log_message("Failed to close file with my_close");
        return EXIT_FAILURE;
    }

    // Safe open and read example
    FILE *file = my_safe_open(filename, "r");
    if (file == NULL) {
        log_message("Failed to open file with my_safe_open");
        return EXIT_FAILURE;
    }

    char safe_read_buf[BUFFER_SIZE];
    ssize_t safe_bytes_read = my_safe_read(fileno(file), safe_read_buf, sizeof(safe_read_buf));
    if (safe_bytes_read == -1) {
        log_message("Failed to read file with my_safe_read");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Process safe read data
    // (Add your processing code here)

    fclose(file);

    return EXIT_SUCCESS;
}