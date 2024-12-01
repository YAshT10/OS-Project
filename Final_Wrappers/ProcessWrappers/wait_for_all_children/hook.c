#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROC_CHILDREN_PATH "/proc/%d/task/%d/children"

pid_t waitpid(pid_t parent_pid,int *status, int options) {
    pid_t (*original_waitpid)(pid_t parent_pid,int *status, int options)=NULL;
    if (original_waitpid == NULL) {
        original_waitpid = dlsym(RTLD_NEXT, "waitpid");
        if (!original_waitpid) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }
    char path[256];
    // PROC_CHILDREN_PATH : directory that contains all the child processes
    snprintf(path, sizeof(path), PROC_CHILDREN_PATH, parent_pid, parent_pid);
    
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Failed to open children file");
        return -1;
    }

    printf("Child processes of PID %d:\n", parent_pid);
    pid_t child_pid;
    // iterate through the child prcesses and call waitpid on all of them
    while (fscanf(file, "%d", &child_pid) == 1) {
        printf("Waiting on child PID %d...\n", child_pid);
        int status=0;
        if (original_waitpid(child_pid, &status, 0) == -1) {
            perror("waitpid failed");
        } else if (WIFEXITED(status)) {
            printf("Child PID %d exited with status %d\n", child_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child PID %d terminated by signal %d\n", child_pid, WTERMSIG(status));
        } else {
            printf("Child PID %d ended unexpectedly\n", child_pid);
        }
    }

    fclose(file);
    return 0;
}
