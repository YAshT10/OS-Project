#include "Safe_read.h"
#include "logger.h"

ssize_t safe_read(int fd, void *buf, size_t count) {
    struct stat file_stat;
    
    // Get file statistics
    if (fstat(fd, &file_stat) == -1) {
        perror("Failed to get file statistics");
        return -1;
    }

    int bytes_available;
    ioctl(fd, FIONREAD, &bytes_available);

    // Check if the file is empty
    if (bytes_available == 0) {
        fprintf(stderr, "Warning: The file is empty.\n");
        return 0;
    }

    // Check if the requested byte count is greater than the file size
    if ((long long)count > (long long)bytes_available) {
        printf("Error: Requested byte count is greater than the available bytes.\nDo you want to read all the remaining bytes? Y/N\n");
        char c;
        scanf(" %c", &c);
        if (c == 'Y' || c == 'y') {
            count = file_stat.st_size;
        } else {
            return -1;
        }
    }

    // Perform the read operation
    ssize_t bytes_read = read_logger(fd, buf, count);
    if (bytes_read == -1) {
        perror("Failed to read from file");
    }

    return bytes_read;
}