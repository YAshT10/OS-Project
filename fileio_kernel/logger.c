#define LOG_FILE "/tmp/file_io.log"
#include "logger.h"

void log_message(const char *message) {
    struct file *log_fp;
    mm_segment_t old_fs;
    loff_t pos = 0;
    char *log_buf;
    size_t log_buf_size;
    struct timespec64 ts;
    struct tm tm;

    log_fp = filp_open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(log_fp)) {
        printk(KERN_ERR "Failed to open log file\n");
        return;
    }

    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec, 0, &tm);

    log_buf_size = snprintf(NULL, 0, "[%04ld-%02d-%02d %02d:%02d:%02d] %s\n",
                            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                            tm.tm_hour, tm.tm_min, tm.tm_sec, message) + 1;
    log_buf = kmalloc(log_buf_size, GFP_KERNEL);
    if (!log_buf) {
        printk(KERN_ERR "Failed to allocate memory for log buffer\n");
        filp_close(log_fp, NULL);
        return;
    }

    snprintf(log_buf, log_buf_size, "[%04ld-%02d-%02d %02d:%02d:%02d] %s\n",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec, message);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    vfs_write(log_fp, log_buf, log_buf_size - 1, &pos);
    set_fs(old_fs);

    kfree(log_buf);
    filp_close(log_fp, NULL);
}

int open_logger(const char *pathname, int flags, umode_t mode) {
    struct file *file;
    int fd;

    file = filp_open(pathname, flags, mode);
    if (IS_ERR(file)) {
        log_message("Failed to open file");
        return PTR_ERR(file);
    }

    fd = get_unused_fd_flags(flags);
    if (fd < 0) {
        log_message("Failed to get unused file descriptor");
        filp_close(file, NULL);
        return fd;
    }

    fd_install(fd, file);

    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Opened file: %s with fd = %d", pathname, fd);
    log_message(log_msg);

    return fd;
}

ssize_t read_logger(int fd, void *buf, size_t count) {
    struct file *file;
    mm_segment_t old_fs;
    ssize_t bytes_read;
    loff_t pos = 0;

    file = fget(fd);
    if (!file) {
        log_message("Failed to get file from fd");
        return -EBADF;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    bytes_read = vfs_read(file, buf, count, &pos);
    set_fs(old_fs);

    fput(file);

    if (bytes_read < 0) {
        log_message("Failed to read file");
    } else {
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "Read %zd bytes from file descriptor %d", bytes_read, fd);
        log_message(log_msg);
    }

    return bytes_read;
}

ssize_t write_logger(int fd, const void *buf, size_t count) {
    struct file *file;
    mm_segment_t old_fs;
    ssize_t bytes_written;
    loff_t pos = 0;

    file = fget(fd);
    if (!file) {
        log_message("Failed to get file from fd");
        return -EBADF;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    bytes_written = vfs_write(file, buf, count, &pos);
    set_fs(old_fs);

    fput(file);

    if (bytes_written < 0) {
        log_message("Failed to write file");
    } else {
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "Wrote %zd bytes to file descriptor %d", bytes_written, fd);
        log_message(log_msg);
    }

    return bytes_written;
}

int close_logger(int fd) {
    struct file *file;
    int result;

    file = fget(fd);
    if (!file) {
        log_message("Failed to get file from fd");
        return -EBADF;
    }

    result = filp_close(file, NULL);
    if (result < 0) {
        log_message("Failed to close file");
    } else {
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "Closed file descriptor %d", fd);
        log_message(log_msg);
    }

    return result;
}
