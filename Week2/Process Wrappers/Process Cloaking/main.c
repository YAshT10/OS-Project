#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>   // For waitpid()

int main() {
    printf("Current pid %d\n", getpid());
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        printf("This is the child process\n");
          printf("Child Working\n");
          sleep(30);  // Keep the child alive
    } else if (pid > 0) {
        // Parent process
        printf("Parent process, monitoring child with PID: %d\n", pid);

        // Wait for the child process to exit (if needed)
        int status;
        waitpid(pid, &status, 0);
        printf("Child process terminated.\n");

    } else {
        perror("fork");
    }
    return 0;
}
