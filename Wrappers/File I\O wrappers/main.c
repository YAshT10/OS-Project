#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int main() {
    // Initialize buffer
    int buffer_fd = open("output.txt", O_WRONLY , S_IRUSR | S_IWUSR);  
     
    // Open file
    const char *filename = "example.txt";
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        // log_message("Failed to open file with open");
        return -1;
    }
    // Read from file
    char read_buf[100];
    // lseek(fd, 10, SEEK_CUR);
    ssize_t bytes_read = read(fd, read_buf, sizeof(read_buf));
    if (bytes_read == -1) {
        // log_message("Failed to read file with read");
        // printf("Failed to read file with read\n");
        close(fd);
        return -1;
    }
    // printf("Read %zd bytes from file\n", bytes_read);
    // for(int i = 0; i <= 15; i++)
    // {
    //     char read_buf[5];
    //     ssize_t bytes_read = read(fd, read_buf, sizeof(read_buf));
    //     if (bytes_read == -1) {
    //         // log_message("Failed to read file with read");
    //         close(fd);
    //         return -1;
    //     }
    //     printf("Read %zd bytes from file\n", bytes_read);
    // }
    // Write to file
    char write_buf[] = "HEEELLO WORLD";
    // printf("Writing %d to file\n", sizeof(write_buf));
    ssize_t bytes_written = write(buffer_fd , write_buf, sizeof(write_buf));
    
    // printf("Wrote %zd bytes to file\n", bytes_written);
    bytes_written = write(fd, read_buf, sizeof(read_buf));
    // Close file
    if (close(fd) == -1) {
        return -1;
    }
    close(buffer_fd);
    return 0;
}