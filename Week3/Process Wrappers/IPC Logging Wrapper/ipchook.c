#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

// Define the log file
#define IPC_LOG_FILE "/home/reroot/Documents/log.txt"

// Maximum size of data to log (for safety)
#define MAX_LOG_DATA_SIZE 100

// Function to log IPC calls and their data
void log_ipc_call(const char *call_name, const char *params, const char *result, const char *data) {
    FILE *log_file = fopen(IPC_LOG_FILE, "a");
    if (log_file) {
        time_t now = time(NULL);
        char *timestamp = ctime(&now);
        timestamp[strcspn(timestamp, "\n")] = 0; // Remove newline from timestamp
        
        fprintf(log_file, "[%s] IPC Call: %s, Params: %s, Result: %s", timestamp, call_name, params, result);
        if (data != NULL) {
            fprintf(log_file, ", Data: %s", data); // Log data if available
        }
        fprintf(log_file, "\n");
        fclose(log_file);
    } else {
        perror("Error opening IPC log file");
    }
}

// Hooking read() system call for logging data reading
ssize_t read(int fd, void *buf, size_t count) {
    static ssize_t (*original_read)(int fd, void *buf, size_t count) = NULL;
    if (original_read == NULL) {
        original_read = dlsym(RTLD_NEXT, "read");
        if (!original_read) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    ssize_t result = original_read(fd, buf, count);

    // Log read operation and data (only the first MAX_LOG_DATA_SIZE characters)
    char params[100];
    snprintf(params, sizeof(params), "fd: %d, count: %zu", fd, count);
    
    // Create a data string with a portion of the buffer
    char data[MAX_LOG_DATA_SIZE + 1];
    snprintf(data, sizeof(data), "%.*s", (int)result < MAX_LOG_DATA_SIZE ? (int)result : MAX_LOG_DATA_SIZE, (char *)buf);
    
    log_ipc_call("read", params, result >= 0 ? "Success" : "Failure", result >= 0 ? data : NULL);

    return result;
}

// Hooking write() system call for logging data writing
ssize_t write(int fd, const void *buf, size_t count) {
    static ssize_t (*original_write)(int fd, const void *buf, size_t count) = NULL;
    if (original_write == NULL) {
        original_write = dlsym(RTLD_NEXT, "write");
        if (!original_write) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    ssize_t result = original_write(fd, buf, count);

    // Log write operation and data (only the first MAX_LOG_DATA_SIZE characters)
    char params[100];
    snprintf(params, sizeof(params), "fd: %d, count: %zu", fd, count);
    
    // Create a data string with a portion of the buffer
    char data[MAX_LOG_DATA_SIZE + 1];
    snprintf(data, sizeof(data), "%.*s", (int)count < MAX_LOG_DATA_SIZE ? (int)count : MAX_LOG_DATA_SIZE, (char *)buf);
    
    log_ipc_call("write", params, result >= 0 ? "Success" : "Failure", result >= 0 ? data : NULL);

    return result;
}

// Assuming the message structure looks like this
struct msgbuffer {
    long mtype;   // Message type
    char mtext[200];  // Message text
};

// Hooking msgrcv() for receiving messages from message queues
ssize_t msgrcv(int msgid, void *msgp, size_t msgsz, long msgtyp, int msgflg) {
    static ssize_t (*original_msgrcv)(int msgid, void *msgp, size_t msgsz, long msgtyp, int msgflg) = NULL;
    if (original_msgrcv == NULL) {
        original_msgrcv = dlsym(RTLD_NEXT, "msgrcv");
        if (!original_msgrcv) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    // Cast msgp to the appropriate message type (struct msgbuf)
    struct msgbuffer *msg = (struct msgbuffer *)msgp;

    // Log message receiving (only the first MAX_LOG_DATA_SIZE characters of mtext)
    char params[200];
    snprintf(params, sizeof(params), "msgid: %d, msgsz: %zu, msgtyp: %ld, msgflg: %d", msgid, msgsz, msgtyp, msgflg);
    
    char data[MAX_LOG_DATA_SIZE + 1];
    snprintf(data, sizeof(data), "%.*s", (int)msgsz < MAX_LOG_DATA_SIZE ? (int)msgsz : MAX_LOG_DATA_SIZE, msg->mtext);
    
    log_ipc_call("msgrcv", params, "Attempting to receive message", data);

    // Call the real msgrcv
    ssize_t result = original_msgrcv(msgid, msgp, msgsz, msgtyp, msgflg);

    log_ipc_call("msgrcv", params, result >= 0 ? "Success" : "Failure", result >= 0 ? data : NULL);

    return result;
}

// Hooking msgsnd() for sending messages to message queues
int msgsnd(int msgid, const void *msgp, size_t msgsz, int msgflg) {
    static int (*original_msgsnd)(int msgid, const void *msgp, size_t msgsz, int msgflg) = NULL;
    if (original_msgsnd == NULL) {
        original_msgsnd = dlsym(RTLD_NEXT, "msgsnd");
        if (!original_msgsnd) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    // Log message sending
    char params[200];
    snprintf(params, sizeof(params), "msgid: %d, msgsz: %zu, msgflg: %d", msgid, msgsz, msgflg);

    // Cast msgp to the appropriate message type (struct msgbuf)
    struct msgbuffer *msg = (struct msgbuffer *)msgp;

    // Log the data being sent (only the first MAX_LOG_DATA_SIZE characters of mtext)
    char data[MAX_LOG_DATA_SIZE + 1];
    snprintf(data, sizeof(data), "%.*s", (int)msgsz < MAX_LOG_DATA_SIZE ? (int)msgsz : MAX_LOG_DATA_SIZE, msg->mtext);

    log_ipc_call("msgsnd", params, "Attempting to send message", data);

    // Call the real msgsnd
    ssize_t result = original_msgsnd(msgid, msgp, msgsz, msgflg);

    log_ipc_call("msgsnd", params, result >= 0 ? "Success" : "Failure", result >= 0 ? data : NULL);

    return result;
}

// Hooking shmat() for attaching shared memory
void* shmat(int shmid, const void *shmaddr, int shmflg) {
    static void* (*original_shmat)(int shmid, const void *shmaddr, int shmflg) = NULL;
    if (original_shmat == NULL) {
        original_shmat = dlsym(RTLD_NEXT, "shmat");
        if (!original_shmat) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return (void*) -1;
        }
    }

    // Log shared memory attachment
    char params[100];
    snprintf(params, sizeof(params), "shmid: %d, shmaddr: %p, shmflg: %d", shmid, shmaddr, shmflg);
    log_ipc_call("shmat", params, "Attaching shared memory", NULL);

    // Call the real shmat
    void* result = original_shmat(shmid, shmaddr, shmflg);

    log_ipc_call("shmat", params, result != (void*) -1 ? "Success" : "Failure", NULL);

    return result;
}

// Hooking semop() for performing semaphore operations
int semop(int semid, struct sembuf *sops, size_t nsops) {
    static int (*original_semop)(int semid, struct sembuf *sops, size_t nsops) = NULL;
    if (original_semop == NULL) {
        original_semop = dlsym(RTLD_NEXT, "semop");
        if (!original_semop) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    // Log semaphore operation
    char params[200];
    snprintf(params, sizeof(params), "semid: %d, nsops: %zu", semid, nsops);
    log_ipc_call("semop", params, "Performing semaphore operation", NULL);

    // Call the real semop
    int result = original_semop(semid, sops, nsops);

    log_ipc_call("semop", params, result == -1 ? "Failure" : "Success", NULL);

    return result;
}

// Hooking kill() for logging signal sending
int kill(pid_t pid, int sig) {
    static int (*original_kill)(pid_t pid, int sig) = NULL;
    if (original_kill == NULL) {
        original_kill = dlsym(RTLD_NEXT, "kill");
        if (!original_kill) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    // Log the kill signal
    char params[100];
    snprintf(params, sizeof(params), "pid: %d, sig: %d", pid, sig);
    log_ipc_call("kill", params, "Signal Sent", NULL);

    // Call the real kill
    return original_kill(pid, sig);
}
