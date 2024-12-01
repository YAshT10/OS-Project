#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Example: Hard-coded program and arguments
    const char *program = "/bin/ls";  // Example program
    char *args[] = { "ls", "-l", "/tmp", NULL };  // Arguments for the program

    // Call the wrapper
    execvp(program, args);

    return 0;
}