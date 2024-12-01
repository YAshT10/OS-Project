#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    const char *filename = "test_file.txt";
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        perror("Failed to open file");
        return 1;
    }

    const char *data = "This is a test file for mmap.\n";
    write(fd, data, 32);
    lseek(fd, 0, SEEK_SET);

    size_t length = 4096;
    void *map = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return 1;
    }

    snprintf((char *)map, length, "Modified content in memory.\n");

    if (munmap(map, length) != 0) {
        perror("munmap failed");
    }

    close(fd);

    return 0;
}
