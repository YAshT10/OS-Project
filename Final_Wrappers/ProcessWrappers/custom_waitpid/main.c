#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int main() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child process
        printf("Child process %d started, simulating work...\n", getpid());
        sleep(5);  // Simulate work with sleep
        printf("Child process %d finished work.\n", getpid());
        _exit(0);  // Exit with status 0
    } else {
        // Parent process
        int status;
        printf("Parent waiting for child process %d to complete.\n", pid);

        pid_t result = waitpid(pid, &status, 0);

        if (result == -1) {
            printf("Parent: Timeout or error occurred while waiting for child process %d\n", pid);
        } else {
            printf("Parent: Successfully waited for child process %d\n", pid);
        }
    }

    return 0;
}
