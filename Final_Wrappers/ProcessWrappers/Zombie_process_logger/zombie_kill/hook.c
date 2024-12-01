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
    // FILE *log_file = fopen("./zombie_logger.log", "a");
    // if (log_file) {
    //     fprintf(log_file, "%s\n", message);
    //     fclose(log_file);
    // }
    printf("%s\n",message);
}

// Wrapper for kill()
int kill(pid_t pid, int sig) {

    int (*original_kill)(pid_t pid, int sig)=NULL;
    if (original_kill == NULL) {
        original_kill = dlsym(RTLD_NEXT, "kill");
        if (!original_kill) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }


    char log[256];
    int result = original_kill(pid, sig);  // Sending the signal to the process

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
