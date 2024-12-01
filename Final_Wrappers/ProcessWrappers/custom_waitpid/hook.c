#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define TIMEOUT_SECONDS 15
pid_t (*original_waitpid)(pid_t pid,int *status, int options)=NULL;


// Custom wrapper for waitpid with timeout and termination
pid_t waitpid(pid_t pid, int *status, int options) {

    if (original_waitpid == NULL) {
        original_waitpid = dlsym(RTLD_NEXT, "waitpid");
        if (!original_waitpid) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    time_t start_time = time(NULL);

    while (1) {
        pid_t ret_pid = original_waitpid(pid, status, options | WNOHANG);  // Non-blocking wait

        if (ret_pid == -1) {
            perror("custom_waitpid: waitpid failed");
            return -1;
        }

        if (ret_pid > 0) {
            // Child process status changed
            if (WIFEXITED(*status)) {
                printf("custom_waitpid: Child process %d exited with status %d\n", pid, WEXITSTATUS(*status));
            } else if (WIFSIGNALED(*status)) {
                printf("custom_waitpid: Child process %d was terminated by signal %d\n", pid, WTERMSIG(*status));
            }
            return ret_pid;
        }

        // Check for timeout
        if (difftime(time(NULL), start_time) >= TIMEOUT_SECONDS) {
            printf("custom_waitpid: Timeout reached while waiting for process %d\n", pid);

            // Kill the child process on timeout
            if (kill(pid, SIGKILL) == -1) {
                perror("custom_waitpid: Failed to kill child process");
                return -1;
            } else {
                printf("custom_waitpid: Child process %d killed due to timeout\n", pid);
            }

            // Wait for the child to terminate after SIGKILL
            return original_waitpid(pid, status, 0);
        }

        // Sleep briefly to avoid busy-waiting
        usleep(100000);  // Sleep for 0.1 seconds
    }
}

// int main() {
//     pid_t pid = fork();

//     if (pid == -1) {
//         perror("fork failed");
//         return 1;
//     }

//     if (pid == 0) {
//         // Child process
//         printf("Child process %d started, simulating work...\n", getpid());
//         sleep(5);  // Simulate work with sleep
//         printf("Child process %d finished work.\n", getpid());
//         _exit(0);  // Exit with status 0
//     } else {
//         // Parent process
//         int status;
//         printf("Parent waiting for child process %d to complete.\n", pid);

//         pid_t result = waitpid(pid, &status, 0);

//         if (result == -1) {
//             printf("Parent: Timeout or error occurred while waiting for child process %d\n", pid);
//         } else {
//             printf("Parent: Successfully waited for child process %d\n", pid);
//         }
//     }

//     return 0;
// }
