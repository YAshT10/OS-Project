#include "logger.h"

#define LOG_FILE "file_io.log"

void log_message(const char *message) {
    FILE *log_fp = fopen(LOG_FILE, "a");
    if (log_fp == NULL) {
        perror("Failed to open log file");
        return;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline character

    fprintf(log_fp, "[%s] %s\n", timestamp, message);
    fclose(log_fp);
}

int open_logger(const char *pathname, int flags, mode_t mode) {
    int fd = open(pathname, flags, mode);
    if (fd == -1) {
        log_message("Failed to open file");
    } else {
        char log_msg[256];
        time_t now = time(NULL);
    char *timestamp = ctime(&now);
        snprintf(log_msg, sizeof(log_msg), "[%s] Opened file: %s", timestamp, pathname);
        log_message(log_msg);
    }
    return fd;
}

ssize_t read_logger(int fd, void *buf, size_t count) {
    ssize_t bytes_read = read(fd, buf, count);
    if (bytes_read == -1) {
        log_message("Failed to read file");
    } else {
        char log_msg[256];
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        snprintf(log_msg, sizeof(log_msg), "[%s] Read %zd bytes from file descriptor %d", timestamp, bytes_read, fd);
        log_message(log_msg);
    }
    return bytes_read;
}

ssize_t write_logger(int fd, const void *buf, size_t count) {
    ssize_t bytes_written = write(fd, buf, count);
    if (bytes_written == -1) {
        log_message("Failed to write file");
    } else {
        char log_msg[256];
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        snprintf(log_msg, sizeof(log_msg), "[%s] Wrote %zd bytes to file descriptor %d", timestamp, bytes_written, fd);
        log_message(log_msg);
    }
    return bytes_written;
}

int close_logger(int fd) {
    int result = close(fd);
    if (result == -1) {
        log_message("Failed to close file");
    } else {
        char log_msg[256];
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        snprintf(log_msg, sizeof(log_msg), "[%s] Closed file descriptor %d", timestamp, fd);
        log_message(log_msg);
    }
    return result;
}