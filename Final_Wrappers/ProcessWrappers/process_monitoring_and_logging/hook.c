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
// Function to log resource usage
void log_resource_usage(struct rusage *usage) {
    printf("User CPU time: %ld.%06lds\n", usage->ru_utime.tv_sec, usage->ru_utime.tv_usec);
    printf("System CPU time: %ld.%06lds\n", usage->ru_stime.tv_sec, usage->ru_stime.tv_usec);
    printf("Maximum resident set size (memory): %ld KB\n", usage->ru_maxrss);
    printf("Voluntary context switches: %ld\n", usage->ru_nvcsw);
    printf("Involuntary context switches: %ld\n", usage->ru_nivcsw);
}

// Wrapper function to monitor and log process execution
int execvp(const char *program, char *const argv[]) {
    int (*original_execvp)(const char *file, char *const argv[])=NULL;
    if (original_execvp == NULL) {
        original_execvp= dlsym(RTLD_NEXT, "execvp");
    }
    
    if (!original_execvp) {
        fprintf(stderr, "dlsym failed to find 'read': %s\n", dlerror());
        exit(1);
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return -1;
    }

    if (pid == 0) {
        // Child process: Execute the given program
        original_execvp(program, argv);
        // If execvp fails
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: Monitor child
        struct rusage usage;
        struct timeval start, end;
        int status;

        // Record start time
        gettimeofday(&start, NULL);

        // Wait for the child process to finish
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            return -1;
        }

        // Record end time
        gettimeofday(&end, NULL);

        // Get resource usage of the child process
        if (getrusage(RUSAGE_CHILDREN, &usage) == -1) {
            perror("getrusage failed");
            return -1;
        }

        // Calculate and print execution time
        long elapsed_sec = end.tv_sec - start.tv_sec;
        long elapsed_usec = end.tv_usec - start.tv_usec;
        if (elapsed_usec < 0) {
            elapsed_sec -= 1;
            elapsed_usec += 1000000;
        }

        printf("Process Execution Report:\n");
        printf("--------------------------\n");
        printf("Start time: %ld.%06ld\n", start.tv_sec, start.tv_usec);
        printf("End time: %ld.%06ld\n", end.tv_sec, end.tv_usec);
        printf("Elapsed time: %ld.%06ld seconds\n", elapsed_sec, elapsed_usec);

        // Log resource usage
        log_resource_usage(&usage);

        // Log exit status
        if (WIFEXITED(status)) {
            printf("Process exited with status %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Process was killed by signal %d\n", WTERMSIG(status));
        } else {
            printf("Process exited abnormally.\n");
        }
    }

    return 0;
}
