#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

ssize_t safe_read(int fd, void *buf, size_t count) {
    struct stat file_stat;
    
    // Get file statistics
    if (fstat(fd, &file_stat) == -1) {
        perror("Failed to get file statistics");
        return -1;
    }

    // Check if the file is empty
    if (file_stat.st_size == 0) {
        fprintf(stderr, "Warning: The file is empty.\n");
        return 0;
    }

    // Check if the requested byte count is greater than the file size
    if (count > file_stat.st_size) {
        fprintf(stderr, "Error: Requested byte count is greater than the file size.\n");
        return -1;
    }

    // Perform the read operation
    ssize_t bytes_read = read(fd, buf, count);
    if (bytes_read == -1) {
        perror("Failed to read from file");
    }

    return bytes_read;
}

int main(){
    // test safe_read on some file example.txt
    int fd = open("example.txt", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return 1;
    }

    char buf[256];
    ssize_t bytes_read = safe_read(fd, buf, 11);
    if (bytes_read == -1) {
        fprintf(stderr, "Error reading file\n");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
    

}