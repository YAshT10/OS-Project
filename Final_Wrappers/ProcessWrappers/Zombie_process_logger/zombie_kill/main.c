#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main() {
    pid_t child_pid;

    // Create a child process
    if ((child_pid = fork()) == -1) {
        perror("Failed to fork process");
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        // Child process: Simulate some work and then exit
        printf("Child process (PID: %d) is running...\n", getpid());
        sleep(2); // Simulate some work
        printf("Child process (PID: %d) exiting...\n", getpid());
        exit(0);  // Exit to create a zombie process
    }

    // Parent process
    printf("Parent process (PID: %d) is waiting for the child process to become a zombie...\n", getpid());

    // Sleep to allow the child process to exit and become a zombie
    sleep(3);

    // Now we will send a signal to the child process to kill it (using kill wrapper)
    int sig = SIGTERM;  // Signal to terminate the child process
    printf("Sending signal %d to child process (PID: %d) using the kill wrapper...\n", sig, child_pid);
    if (kill(child_pid, sig) == 0) {
        printf("Signal %d sent to child process (PID: %d) successfully.\n", sig, child_pid);
    } else {
        perror("Failed to send signal");
        return EXIT_FAILURE;
    }
}
