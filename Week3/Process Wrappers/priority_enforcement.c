#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/resource.h>
#include <string.h>

// Set I/O priority using the ionice system call wrapper
void set_io_priority(int pid, int io_class, int priority)
{
    char command[128];
    snprintf(command, sizeof(command), "ionice -c %d -n %d -p %d", io_class, priority, pid);
    if (system(command) == -1)
    {
        perror("Failed to set I/O priority");
        exit(EXIT_FAILURE);
    }
}

// Function to set CPU priority (niceness)
void set_cpu_priority(int nice_value)
{
    if (setpriority(PRIO_PROCESS, 0, nice_value) == -1)
    {
        perror("Failed to set CPU priority");
        exit(EXIT_FAILURE);
    }
}

// Priority-aware fork wrapper
pid_t priority_fork(int cpu_priority, int io_class, int io_priority)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // In the child process
        printf("[Child %d] Setting CPU priority (nice value): %d\n", getpid(), cpu_priority);
        set_cpu_priority(cpu_priority);

        printf("[Child %d] Setting I/O priority: class %d, priority %d\n", getpid(), io_class, io_priority);
        set_io_priority(getpid(), io_class, io_priority);

        return 0; // Return control to the child process
    }

    // In the parent process
    return pid;
}

void cpu_intensive_workload()
{
    printf("[Process %d] Starting CPU-intensive workload...\n", getpid());
    long long result = 0;
    for (long long i = 0; i < 1e10; i++)
    {
        result += i;
    }
    printf("[Process %d] CPU-intensive workload complete. Result: %lld\n", getpid(), result);
}

// Function for an I/O-intensive workload
void io_intensive_workload(const char *filename)
{
    printf("[Process %d] Starting I/O-intensive workload...\n", getpid());
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 100000000; i++)
    {
        fprintf(file, "This is line %d written by process %d.\n", i, getpid());
    }
    fclose(file);
    printf("[Process %d] I/O-intensive workload complete. File: %s\n", getpid(), filename);
}

int main()
{
    int cpu_priority = -10;
    int io_class = 1;
    int io_priority = 0;
    pid_t high_priority_pid = priority_fork(cpu_priority, io_class, io_priority);

    if (high_priority_pid == 0)
    {
        // Child process: simulate workload
        printf("[Child %d] Child process started with high priorities.\n", getpid());
        cpu_intensive_workload();
        io_intensive_workload("high_priority_output.txt");
        exit(EXIT_SUCCESS);
    }
    else
    {
        int cpu_priority = 10;
        int io_class = 3;
        int io_priority = 7;
        pid_t low_priority_pid = priority_fork(cpu_priority, io_class, io_priority);

        if (low_priority_pid == 0)
        {
            // Child process: simulate workload
            printf("[Child %d] Child process started with low priorities.\n", getpid());
            cpu_intensive_workload();
            io_intensive_workload("low_priority_output.txt");
            exit(EXIT_SUCCESS);
        }
        else
        {
            waitpid(high_priority_pid, NULL, 0);
            printf("High-priority process %d completed.\n", high_priority_pid);
            waitpid(low_priority_pid, NULL, 0);
            printf("Low-priority process %d completed.\n", low_priority_pid);
        }
    }

    return 0;
}
// The low priority process might complete before high priority process because there are multiple CPUs and less processes running.So, priority enforcement doesn't make much difference.
// It will matter when there are a lot of processes running together and CPU is busy.