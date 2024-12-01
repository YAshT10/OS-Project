#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

#define PROC_CHILDREN_PATH "/proc/%d/task/%d/children"

void wait_for_all_children(pid_t parent_pid)
{
    char path[256];
    // PROC_CHILDREN_PATH : directory that contains all the child processes
    snprintf(path, sizeof(path), PROC_CHILDREN_PATH, parent_pid, parent_pid);

    FILE *file = fopen(path, "r");
    if (!file)
    {
        perror("Failed to open children file");
        return;
    }

    printf("Child processes of PID %d:\n", parent_pid);
    pid_t child_pid;
    // iterate through the child prcesses and call waitpid on all of them
    while (fscanf(file, "%d", &child_pid) == 1)
    {
        printf("Waiting on child PID %d...\n", child_pid);

        int status;
        if (waitpid(child_pid, &status, 0) == -1)
        {
            perror("waitpid failed");
        }
        else if (WIFEXITED(status))
        {
            printf("Child PID %d exited with status %d\n", child_pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Child PID %d terminated by signal %d\n", child_pid, WTERMSIG(status));
        }
        else
        {
            printf("Child PID %d ended unexpectedly\n", child_pid);
        }
    }

    fclose(file);
}

int main()
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Simulate child process work
        sleep(2);
        exit(0);
    }
    else if (pid > 0)
    {
        // Parent process
        pid_t second_pid = fork();
        if (second_pid == 0)
        {
            // Second child process
            sleep(2);
            exit(0);
        }
        else if (second_pid > 0)
        {
            // Parent Process
            sleep(1); // Let child process run a bit
            wait_for_all_children(getpid());
        }
    }
    else
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}
