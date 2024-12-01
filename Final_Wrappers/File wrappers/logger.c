#include "logger.h"

void log_message(const char *message) {
    FILE *log_fp = fopen(LOG_FILE, "a");
    if (log_fp == NULL) {
        perror("Failed to open log file");
        return;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0';

    fprintf(log_fp, "[%s] %s\n", timestamp, message);
    fclose(log_fp);
}

int open_logger(const char *pathname, int flags, mode_t mode) {
    ssize_t (*orig_open)(const char *, int, mode_t) = dlsym(RTLD_NEXT, "open");
    int fd = orig_open(pathname, flags, mode);
    if (fd == -1) {
        if(errno == EINVAL) {
            log_message("Invalid flags or mode");
        } else {
        log_message("Failed to open file");
    }
    } else {
        char log_msg[256];
        time_t now = time(NULL);
    char *timestamp = ctime(&now);
        snprintf(log_msg, sizeof(log_msg), "[%s] Opened file: %s with fd = %d", timestamp, pathname, fd);
        log_message(log_msg);
    }
    return fd;
}

ssize_t read_logger(int fd, void *buf, size_t count) {
    ssize_t (*orig_read)(int, void *, size_t) = dlsym(RTLD_NEXT, "read");
    ssize_t bytes_read = orig_read(fd, buf, count);
    if (bytes_read == -1) {
        if(errno == EBADF) {
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Invalid file descriptor for reading ; fd : %d", fd);
            log_message(log_msg);
        } else if(errno == EFAULT) {
            log_message("Bad address - read");
        } else if (errno == EIO) {
            log_message("I/O error - read");
        }
        else{
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Failed to read file ; fd : %d", fd);
            log_message(log_msg);
        }
    } else {
        if(buf == NULL) {
            return 0;
        }
        char log_msg[256];
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        snprintf(log_msg, sizeof(log_msg), "[%s] Read %zd bytes from file descriptor %d", timestamp, bytes_read, fd);
        log_message(log_msg);
    }
    return bytes_read;
}

ssize_t write_logger(int fd, const void *buf, size_t count) {
    ssize_t (*orig_write)(int, const void *, size_t) = dlsym(RTLD_NEXT, "write");
    ssize_t bytes_written = orig_write(fd, buf, count);
    if (bytes_written == -1) {
        if(errno == EBADF) {
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Invalid file descriptor for writing ; fd : %d", fd);
            log_message(log_msg);
        } else if(errno == EFAULT) {
            log_message("Bad address - write");
        } else if (errno == EIO) {
            log_message("I/O error - write");
        }
        else{
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Failed to write file ; fd : %d", fd);
            log_message(log_msg);
        }
    } else {
        char log_msg[256];
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        snprintf(log_msg, sizeof(log_msg), "[%s] Wrote %zd bytes to file descriptor %d", timestamp, bytes_written, fd);
        log_message(log_msg);
    }
    return bytes_written;
}

ssize_t buffer_logger(int fd, const void *buf, size_t count) {
    ssize_t (*orig_write)(int, const void *, size_t) = dlsym(RTLD_NEXT, "write");
    ssize_t bytes_written = orig_write(fd, buf, count);
    if (bytes_written == -1) {
        if(errno == EBADF) {
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Invalid file descriptor for writing ; fd : %d", fd);
            log_message(log_msg);
        } else if(errno == EFAULT) {
            log_message("Bad address - write");
        } else if (errno == EIO) {
            log_message("I/O error - write");
        }
        else{
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Failed to flush buffer to file ; fd : %d", fd);
            log_message(log_msg);
        }
    } else {
        char log_msg[256];
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        snprintf(log_msg, sizeof(log_msg), "[%s] Flushed %zd bytes from buffer to file descriptor %d", timestamp, bytes_written, fd);
        log_message(log_msg);
    }
    return bytes_written;
}

int close_logger(int fd) {
    int (*orig_close)(int) = dlsym(RTLD_NEXT, "close");
    int result = orig_close(fd);
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