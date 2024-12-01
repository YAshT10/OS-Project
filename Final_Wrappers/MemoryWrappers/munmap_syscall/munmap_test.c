#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main() {
    const size_t length = 4096; 
    const char *message = "Testing munmap wrapper!";

    void *addr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    printf("Memory mapped at address: %p\n", addr);

    strncpy((char *)addr, message, length);
    printf("Written to memory: %s\n", (char *)addr);

    if (strncmp((char *)addr, message, length) == 0) {
        printf("Data verified: %s\n", (char *)addr);
    } else {
        fprintf(stderr, "Data verification failed.\n");
    }

    if (munmap(addr, length) != 0) {
        perror("munmap");
        return 1;
    }
    printf("Memory unmapped successfully.\n");

    printf("Attempting to read unmapped memory (should crash): %c\n", ((char *)addr)[0]);

    return 0;
}
