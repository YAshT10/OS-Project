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

// Logger function
void log_message(const char *message) {
    FILE *log_file = fopen("/var/log/zombie_logger.log", "a");
    if (log_file) {
        fprintf(log_file, "%s\n", message);
        fclose(log_file);
    }
}

// Wrapper for waitpid()
pid_t waitpid(pid_t pid, int *status, int options) {

    pid_t (*original_waitpid)(pid_t pid,int *status, int options)=NULL;
    if (original_waitpid == NULL) {
        original_waitpid = dlsym(RTLD_NEXT, "waitpid");
        if (!original_waitpid) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    char log[256];
    pid_t result = original_waitpid(pid, status, options);

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
    while ((child_pid = original_waitpid(-1, &cleanup_status, WNOHANG)) > 0) {
        if (WIFEXITED(cleanup_status)) {
            snprintf(log, sizeof(log), "Cleaned up zombie process %d, terminated normally with exit code %d.", child_pid, WEXITSTATUS(cleanup_status));
        } else if (WIFSIGNALED(cleanup_status)) {
            snprintf(log, sizeof(log), "Cleaned up zombie process %d, terminated by signal %d.", child_pid, WTERMSIG(cleanup_status));
        }
        log_message(log);
    }

    return result;
}