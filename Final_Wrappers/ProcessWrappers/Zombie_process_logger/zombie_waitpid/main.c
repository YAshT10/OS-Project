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
        sleep(2); // Simulate work
        printf("Child process (PID: %d) exiting...\n", getpid());
        exit(0);  // Exit to create a zombie process
    }

    // Parent process
    printf("Parent process (PID: %d) is waiting for zombie process...\n", getpid());

    // Sleep to allow the child process to exit and become a zombie
    sleep(3);

    // Check for zombie processes
    int status;
    pid_t result = waitpid(child_pid, &status, WNOHANG);
    if (result == 0) {
        printf("Zombie process detected (PID: %d).\n", child_pid);
    } else if (result > 0) {
        printf("Zombie process (PID: %d) reaped.\n", result);

        // Log the status of the process
        if (WIFEXITED(status)) {
            printf("Process exited with status %d.\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Process terminated by signal %d.\n", WTERMSIG(status));
        }
    } else {
        perror("Error waiting for process");
        return EXIT_FAILURE;
    }

    printf("Parent process (PID: %d) cleanup complete.\n", getpid());
    return EXIT_SUCCESS;
}
