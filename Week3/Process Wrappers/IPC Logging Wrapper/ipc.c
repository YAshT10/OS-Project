#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

// Define IPC-related constants and structures for testing
#define MSG_KEY 1234
#define SHM_KEY 5678
#define SEM_KEY 91011

// Test function for pipes
void test_pipe() {
    int pipefds[2];
    if (pipe(pipefds) == -1) {
        perror("pipe");
    } else {
        write(pipefds[1], "Test message", 12);
        close(pipefds[0]);
        close(pipefds[1]);
    }
}

// Test function for message queues
void test_msgget_msgsnd() {
    int msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        return;
    }

    struct {
        long mtype;
        char mtext[100];
    } message;

    message.mtype = 1;
    strcpy(message.mtext, "Hello, message queue!");

    if (msgsnd(msgid, &message, sizeof(message.mtext), 0) == -1) {
        perror("msgsnd");
    }

    // Clean up message queue
    msgctl(msgid, IPC_RMID, NULL);
}

// Test function for shared memory
void test_shmget_shmat_shmdt() {
    int shmid = shmget(SHM_KEY, 1024, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return;
    }

    char *shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("shmat");
        return;
    }

    strcpy(shm_ptr, "Shared memory test message.");

    printf("Shared Memory Content: %s\n", shm_ptr);

    // Detach shared memory
    if (shmdt(shm_ptr) == -1) {
        perror("shmdt");
    }

    // Clean up shared memory segment
    shmctl(shmid, IPC_RMID, NULL);
}

// Test function for semaphores
void test_semget_semop() {
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return;
    }

    // Initialize semaphore
    semctl(semid, 0, SETVAL, 1);

    struct sembuf sb = { 0, -1, 0 }; // Decrement semaphore

    if (semop(semid, &sb, 1) == -1) {
        perror("semop");
    }

    // Clean up semaphore
    semctl(semid, 0, IPC_RMID);
}

// Signal handler function
void sigusr1_handler(int signo) {
    if (signo == SIGUSR1) {
        printf("Parent process received SIGUSR1\n");
    }
}

// Test function for signals
void test_kill_signal() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        printf("Child process (PID: %d) sleeping...\n", getpid());
        sleep(2); // Simulate some work
        printf("Child process sending signal to parent\n");
        kill(getppid(), SIGUSR1);  // Send signal to parent
        exit(0);
    } else {
        // Parent process
        // Set the signal handler for SIGUSR1
        if (signal(SIGUSR1, sigusr1_handler) == SIG_ERR) {
            perror("Error setting signal handler");
            exit(1);
        }

        wait(NULL); // Wait for child process to finish
    }
}

int main() {
    // Test IPC mechanisms
    printf("Testing Pipe...\n");
    test_pipe();

    printf("\nTesting Message Queue...\n");
    test_msgget_msgsnd();

    printf("\nTesting Shared Memory...\n");
    test_shmget_shmat_shmdt();

    printf("\nTesting Semaphore...\n");
    test_semget_semop();

    printf("\nTesting Signal...\n");
    test_kill_signal();

    return 0;
}
