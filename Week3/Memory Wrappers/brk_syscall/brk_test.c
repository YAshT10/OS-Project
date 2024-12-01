#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main() {
    void *initial_brk = sbrk(0);
    printf("Initial program break: %p\n", initial_brk);

    void *new_brk = initial_brk + 4096; 
    printf("Attempting to set program break to: %p\n", new_brk);

    if (brk(new_brk) == 0) {
        printf("brk call succeeded. New program break: %p\n", sbrk(0));
    } else {
        printf("brk call failed. errno: %d (%s)\n", errno, strerror(errno));
    }

    printf("Resetting program break to: %p\n", initial_brk);
    if (brk(initial_brk) == 0) {
        printf("Program break reset. Current break: %p\n", sbrk(0));
    } else {
        printf("Failed to reset program break. errno: %d (%s)\n", errno, strerror(errno));
    }

    return 0;
}
