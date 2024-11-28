#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
int main() {
    // Initialize buffer
    int buffer_fd = open("output.txt", O_WRONLY);  
    // Open file
    const char *filename = "example.txt";
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        // log_message("Failed to open file with open");
        return -1;
    }

    // Read from file
    char read_buf[10];
    ssize_t bytes_read = read(fd, read_buf, sizeof(read_buf));
    if (bytes_read == -1) {
        // log_message("Failed to read file with read");
        close(fd);
        return -1;
    }

    // Close file
    if (close(fd) == -1) {
        // log_message("Failed to close file with close");
        return -1;
    }

    return 0;
}