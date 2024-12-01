#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_WORKERS 5
#define MAX_TASKS 100

// Structure for tasks
typedef struct
{
    char function_name[256]; // Name of the function to execute
    int arg;                 // Argument for the function
} Task;

typedef struct
{
    int read_fd;
    int write_fd;
    pid_t pid;
} Worker;

Worker workers[MAX_WORKERS];
Task task_queue[MAX_TASKS];
int task_count = 0;
int current_task = 0;
int worker_index = 0;

// Function declarations
void compute_square(int n);
void print_message(int n);
void task_processor(int read_fd, int write_fd);
void add_task(const char *function_name, int arg);
void create_worker(int index);
void assign_task_to_worker(int worker_index);
void signal_handler(int signo);
void shutdown_pool();

// Function implementations
void compute_square(int n)
{
    int result = n * n;
    printf("Task performed by process %d\n : Square of %d is %d\n", getpid(), n, result);
}

void print_message(int n)
{
    printf("Task performed by process %d\n : Task data is %d\n", getpid(), n);
}

void task_processor(int read_fd, int write_fd)
{
    while (1)
    {
        Task task;
        if (read(read_fd, &task, sizeof(Task)) <= 0)
        {
            break; // No more tasks, exit
        }

        // Match function name and execute corresponding function
        if (strcmp(task.function_name, "compute_square") == 0)
        {
            compute_square(task.arg);
        }
        else if (strcmp(task.function_name, "print_message") == 0)
        {
            print_message(task.arg);
        }
        else
        {
            fprintf(stderr, "Unknown task: %s\n", task.function_name);
        }
    }
}

void create_worker(int index)
{
    int pipe_parent[2], pipe_child[2];

    if (pipe(pipe_parent) == -1 || pipe(pipe_child) == -1)
    {
        perror("Pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    { // Child process
        close(pipe_parent[1]);
        close(pipe_child[0]);

        task_processor(pipe_parent[0], pipe_child[1]);
        exit(EXIT_SUCCESS);
    }
    else
    { // Parent process
        close(pipe_parent[0]);
        close(pipe_child[1]);

        workers[index].read_fd = pipe_child[0];
        workers[index].write_fd = pipe_parent[1];
        workers[index].pid = pid;
    }
}

void assign_task_to_worker(int worker_index)
{
    if (current_task < task_count)
    {
        write(workers[worker_index].write_fd, &task_queue[current_task], sizeof(Task));
        current_task++;
    }
}

void add_task(const char *function_name, int arg)
{
    if (task_count >= MAX_TASKS)
    {
        fprintf(stderr, "Task queue full\n");
        return;
    }

    strncpy(task_queue[task_count].function_name, function_name, sizeof(task_queue[task_count].function_name) - 1);
    task_queue[task_count].arg = arg;
    task_count++;

    // Assign tasks to workers
    assign_task_to_worker(worker_index);
    worker_index = (worker_index + 1) % MAX_WORKERS;
}

void signal_handler(int signo)
{
    if (signo == SIGCHLD)
    {
        for (int i = 0; i < MAX_WORKERS; i++)
        {
            int status;
            if (waitpid(workers[i].pid, &status, WNOHANG) > 0)
            {
                close(workers[i].read_fd);
                close(workers[i].write_fd);
                create_worker(i); // Restart worker after completion
            }
        }
    }
}

void shutdown_pool()
{
    for (int i = 0; i < MAX_WORKERS; i++)
    {
        close(workers[i].read_fd);
        close(workers[i].write_fd);
        kill(workers[i].pid, SIGTERM);
        waitpid(workers[i].pid, NULL, 0);
    }
}

int main()
{
    signal(SIGCHLD, signal_handler);

    // Create workers (fork processes)
    for (int i = 0; i < MAX_WORKERS; i++)
    {
        create_worker(i);
    }

    // Add tasks with function names
    for (int i = 0; i < 2; i++)
    {
        add_task("compute_square", 3);
        add_task("compute_square", 5);
        add_task("print_message", 42);
        add_task("print_message", 7);
    }

    // Allow some time for tasks to complete
    sleep(1);

    // Shut down workers
    shutdown_pool();

    return 0;
}