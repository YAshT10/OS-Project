#include "Safe_read.h"
#include "logger.h"

ssize_t safe_read(struct file *file, char __user *buf, size_t count) {
    struct kstat file_stat;
    mm_segment_t old_fs;
    ssize_t bytes_read;
    loff_t pos = 0;
    int bytes_available;

    // Get file statistics
    if (vfs_getattr(&file->f_path, &file_stat, STATX_BASIC_STATS, AT_STATX_SYNC_AS_STAT) != 0) {
        pr_err("Failed to get file statistics\n");
        return -1;
    }

    // Get the number of bytes available to read
    bytes_available = i_size_read(file_inode(file)) - file->f_pos;

    // Check if the file is empty
    if (bytes_available == 0) {
        pr_warn("Warning: The file is empty.\n");
        return 0;
    }

    // Check if the requested byte count is greater than the file size
    if ((long long)count > (long long)bytes_available) {
        pr_err("Error: Requested byte count is greater than the available bytes.\n");
        return -1;
    }

    // Perform the read operation
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    bytes_read = vfs_read(file, buf, count, &pos);
    set_fs(old_fs);

    if (bytes_read < 0) {
        pr_err("Failed to read from file\n");
    }

    return bytes_read;
}
