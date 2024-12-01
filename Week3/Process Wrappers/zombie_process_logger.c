#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

// Logger function
void log_message(const char *message) {
    FILE *log_file = fopen("/var/log/zombie_logger.log", "a");
    if (log_file) {
        fprintf(log_file, "%s\n", message);
        fclose(log_file);
    }
}

// Wrapper for kill()
int kill_wrapper(pid_t pid, int sig) {
    char log[256];
    int result = kill(pid, sig);  // Sending the signal to the process

    // Log whether the signal was sent successfully or failed
    if (result == 0) {
        snprintf(log, sizeof(log), "Signal %d sent to process %d successfully.", sig, pid);
    } else {
        snprintf(log, sizeof(log), "Failed to send signal %d to process %d. Error: %s", sig, pid, strerror(errno));
    }
    log_message(log);

    // Ensure proper cleanup by checking for any zombie processes
    int status;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            snprintf(log, sizeof(log), "Cleaned up zombie process %d, terminated normally with exit code %d.", child_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            snprintf(log, sizeof(log), "Cleaned up zombie process %d, terminated by signal %d.", child_pid, WTERMSIG(status));
        }
        log_message(log);
    }

    return result;  // Return the result of the kill() syscall (success/failure)
}

// Wrapper for waitpid()
pid_t waitpid_wrapper(pid_t pid, int *status, int options) {
    char log[256];
    pid_t result = waitpid(pid, status, options);

    // Log the result of the waitpid
    if (result > 0) {
        if (WIFEXITED(*status)) {
            snprintf(log, sizeof(log), "Process %d terminated normally with exit code %d.", result, WEXITSTATUS(*status));
        } else if (WIFSIGNALED(*status)) {
            snprintf(log, sizeof(log), "Process %d terminated by signal %d.", result, WTERMSIG(*status));
        } else if (WIFSTOPPED(*status)) {
            snprintf(log, sizeof(log), "Process %d stopped by signal %d.", result, WSTOPSIG(*status));
        }
    } else if (result == 0) {
        snprintf(log, sizeof(log), "No child process state change detected for PID %d.", pid);
    } else {
        snprintf(log, sizeof(log), "waitpid failed for PID %d. Error: %s", pid, strerror(errno));
    }
    log_message(log);

    // Check for any zombie processes after waitpid
    int cleanup_status;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &cleanup_status, WNOHANG)) > 0) {
        if (WIFEXITED(cleanup_status)) {
            snprintf(log, sizeof(log), "Cleaned up zombie process %d, terminated normally with exit code %d.", child_pid, WEXITSTATUS(cleanup_status));
        } else if (WIFSIGNALED(cleanup_status)) {
            snprintf(log, sizeof(log), "Cleaned up zombie process %d, terminated by signal %d.", child_pid, WTERMSIG(cleanup_status));
        }
        log_message(log);
    }

    return result;
}

int main(int argc, char *argv[]) {
    // Example usage of the wrapper functions
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <pid> <signal>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t pid = atoi(argv[1]);
    int sig = atoi(argv[2]);

    // Test the kill_wrapper
    printf("Sending signal %d to process %d...\n", sig, pid);
    if (kill_wrapper(pid, sig) == 0) {
        printf("Signal sent successfully.\n");
    } else {
        perror("Failed to send signal");
    }

    // Test the waitpid_wrapper
    int status;
    printf("Waiting for process %d...\n", pid);
    if (waitpid_wrapper(pid, &status, 0) > 0) {
        printf("Process %d cleanup complete.\n", pid);
    } else {
        perror("Failed to wait for process");
    }

    return EXIT_SUCCESS;
}
