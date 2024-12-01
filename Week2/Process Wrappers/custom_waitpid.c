#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#define TIMEOUT_SECONDS 5  // Timeout period for the wait

// Custom wrapper for waitpid with additional features
pid_t custom_waitpid(pid_t pid, int *status, int options) {
    time_t start_time = time(NULL);

    // Polling loop to implement a timeout feature
    while (1) {
        pid_t ret_pid = waitpid(pid, status, options);

        if (ret_pid == -1) {
            // Error occurred during waitpid
            perror("custom_waitpid: waitpid failed");
            return -1;
        }

        if (ret_pid > 0) {
            // Child process status changed
            if (WIFEXITED(*status)) {
                printf("custom_waitpid: Child process %d exited with status %d\n", pid, WEXITSTATUS(*status));
            } else if (WIFSIGNALED(*status)) {
                printf("custom_waitpid: Child process %d was terminated by signal %d\n", pid, WTERMSIG(*status));
            } else if (WIFSTOPPED(*status)) {
                printf("custom_waitpid: Child process %d was stopped by signal %d\n", pid, WSTOPSIG(*status));
            }
            return ret_pid;
        }

        // Check for timeout
        if (difftime(time(NULL), start_time) >= TIMEOUT_SECONDS) {
            printf("custom_waitpid: Timeout reached while waiting for process %d\n", pid);
            return -1;
        }

        // Sleep briefly to avoid busy-waiting
        usleep(100000);  // Sleep for 0.1 seconds
    }
}

int main() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child process
        printf("Child process %d started, simulating work...\n", getpid());
        sleep(2);  // Simulate work with sleep
        printf("Child process %d finished work.\n", getpid());
        _exit(0);  // Exit with status 0
    } else {
        // Parent process
        int status;
        printf("Parent waiting for child process %d to complete.\n", pid);

        pid_t result = custom_waitpid(pid, &status, 0);

        if (result == -1) {
            printf("Parent: Failed to wait for child process %d\n", pid);
        } else {
            printf("Parent: Successfully waited for child process %d\n", pid);
        }
    }

    return 0;
}