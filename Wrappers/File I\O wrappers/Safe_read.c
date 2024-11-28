#include "Safe_read.h"
#include "logger.h"

ssize_t safe_read(int fd, void *buf, size_t count) {
    struct stat file_stat;
    
    // Get file statistics
    if (fstat(fd, &file_stat) == -1) {
        perror("Failed to get file statistics");
        return -1;
    }

    // Check if the file is empty
    if (file_stat.st_size == 0) {
        perror("Warning: The file is empty.\n");
        return 0;
    }

    // Check if the requested byte count is greater than the file size
    if ((long long)count > (long long)file_stat.st_size) {
        perror("Error: Requested byte count is greater than the file size.\n");
        return -1;
    }

    // Perform the read operation
    ssize_t bytes_read = read_logger(fd, buf, count);
    if (bytes_read == -1) {
        perror("Failed to read from file");
    }

    return bytes_read;
}