#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>   // For waitpid()

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        // Simulate child process work
        sleep(2);
        exit(0);
    } else if (pid > 0) {
        // Parent process
        pid_t second_pid=fork();
        if(second_pid==0)
        {
            // Second child process
            sleep(2);
            exit(0);
        }
        else if(second_pid>0)
        {
            // Parent Process
            sleep(1);  // Let child process run a bit
            int status=0;
            waitpid(getpid(),&status,0);
        }
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}