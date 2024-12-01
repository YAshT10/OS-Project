#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <dlfcn.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

#define MAX_LOG_SIZE 200
#define MAX_RES_COUNT 100
#define TEMP_GRAPH_FILE "/home/reroot/Documents/temp_process_resource_graph.txt"
#define HELD_RESOURCES_FILE "/home/reroot/Documents/held_resources.txt"  // File to store held resources
#define WAITING_RESOURCES_FILE "/home/reroot/Documents/waiting_resources.txt"  // File to store waiting resources

// Structure to store PID, TID, and mutex address
typedef struct Resource {
    pid_t pid;
    pthread_t tid;
    void *mutex_addr;  // Store the memory address of the mutex
} Resource;

Resource held_resources[MAX_RES_COUNT];  
Resource waiting_resources[MAX_RES_COUNT];  
int held_count = 0;
int waiting_count = 0;

// Function to log held resources
void log_held_resource(pid_t pid, pthread_t tid, void *mutex_addr) {
    FILE *file = fopen(HELD_RESOURCES_FILE, "a");
    if (!file) {
        perror("Error opening held resources file");
        return;
    }
    fprintf(file, "Process %d (Thread %lu) -> MemLoc: %p\n", pid, tid, mutex_addr);
    fclose(file);
}

// Function to log waiting resources
void log_waiting_resource(pid_t pid, pthread_t tid, void *mutex_addr) {
    FILE *file = fopen(WAITING_RESOURCES_FILE, "a");
    if (!file) {
        perror("Error opening waiting resources file");
        return;
    }
    fprintf(file, "Process %d (Thread %lu) -> MemLoc: %p\n", pid, tid, mutex_addr);
    fclose(file);
}

// Function to remove a resource from the file
void remove_resource_from_file(const char *filename, pid_t pid, pthread_t tid, void *mutex_addr) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening resource file");
        return;
    }

    FILE *temp_file = fopen(TEMP_GRAPH_FILE, "w");
    if (!temp_file) {
        perror("Error creating temp file");
        fclose(file);
        return;
    }

    char line[MAX_LOG_SIZE];
    while (fgets(line, sizeof(line), file)) {
        pid_t pid_in_file;
        pthread_t tid_in_file;
        void *mutex_addr_in_file;
        if (sscanf(line, "Process %d (Thread %lu) -> Mutex: %p", &pid_in_file, &tid_in_file, &mutex_addr_in_file) == 3) {
            if (pid_in_file != pid || tid_in_file != tid || mutex_addr_in_file != mutex_addr) {
                fputs(line, temp_file);  // Copy the line to temp file if it's not the one being removed
            }
        }
    }

    fclose(file);
    fclose(temp_file);

    // Replace the original file with the temp file
    rename(TEMP_GRAPH_FILE, filename);
}

// Function to read the held resources and store them
void read_held_resources() {
    FILE *file = fopen(HELD_RESOURCES_FILE, "r");
    if (!file) {
        perror("Error opening held resources file");
        return;
    }

    char line[MAX_LOG_SIZE];
    while (fgets(line, sizeof(line), file)) {
        pid_t pid;
        pthread_t tid;
        void *mutex_addr;

        // Parse the held resource details
        if (sscanf(line, "Process %d (Thread %lu) -> MemLoc: %p", &pid, &tid, &mutex_addr) != 3)
            continue;

        held_resources[held_count].pid = pid;
        held_resources[held_count].tid = tid;
        held_resources[held_count].mutex_addr = mutex_addr;
        held_count++;
    }
    fclose(file);
}

// Function to read the waiting resources and store them
void read_waiting_resources() {
    FILE *file = fopen(WAITING_RESOURCES_FILE, "r");
    if (!file) {
        perror("Error opening waiting resources file");
        return;
    }

    char line[MAX_LOG_SIZE];
    while (fgets(line, sizeof(line), file)) {
        pid_t pid;
        pthread_t tid;
        void *mutex_addr;

        // Parse the waiting resource details
        if (sscanf(line, "Process %d (Thread %lu) -> MemLoc: %p", &pid, &tid, &mutex_addr) != 3)
            continue;

        waiting_resources[waiting_count].pid = pid;
        waiting_resources[waiting_count].tid = tid;
        waiting_resources[waiting_count].mutex_addr = mutex_addr;
        waiting_count++;
    }
    fclose(file);
}

// Function to detect deadlocks by checking for cycles between held and waiting resources
int detect_deadlock() {
    // Read the resources
    read_held_resources();
    read_waiting_resources();

    // Traverse all waiting resources to detect cycles
    for (int i = 0; i < waiting_count; i++) {
        void *resource_a = waiting_resources[i].mutex_addr;
        int current_pid = waiting_resources[i].pid;
        long unsigned int current_tid = waiting_resources[i].tid;
        
        // Array to track visited resources during the graph traversal (cycle detection)
        int marked[MAX_RES_COUNT] = {0};  // 0 = not visited, 1 = visited
        char cycle_details[MAX_RES_COUNT * 100] = "";  // To store the chain of resources involved in the deadlock
        marked[i] = 1;
        
        // Start building the cycle path from the current waiting resource
        snprintf(cycle_details, sizeof(cycle_details), "Thread %lu (PID %d) waiting for MemLoc: %p\n", current_tid, current_pid, resource_a);

        // Traverse the resource dependencies
        int found_cycle = 0;
        int path_index = strlen(cycle_details);  // Starting point for appending to the cycle path
        int cycle_depth = 0; // To track the depth of the cycle

        while (1) {
            // Find which held resource is holding the current resource
            int found = 0;
            for (int j = 0; j < held_count; j++) {
                if (held_resources[j].mutex_addr == resource_a) {
                    // If a thread is holding the resource, check what it's waiting for
                    int held_pid = held_resources[j].pid;
                    long unsigned int held_tid = held_resources[j].tid;

                    // Add this resource to the cycle path
                    snprintf(cycle_details + path_index, sizeof(cycle_details) - path_index, 
                             "Thread %lu (PID %d) holding MemLoc: %p\n", held_tid, held_pid, resource_a);
                    path_index = strlen(cycle_details);  // Update the path index
                    cycle_depth++;

                    // Now check what the current holder is waiting for
                    for (int k = 0; k < waiting_count; k++) {
                        if (waiting_resources[k].pid == held_pid && waiting_resources[k].tid == held_tid) {
                            // Add this resource to the cycle path
                            snprintf(cycle_details + path_index, sizeof(cycle_details) - path_index, 
                                     "Thread %lu (PID %d) waiting for MemLoc: %p\n", held_tid, held_pid, waiting_resources[k].mutex_addr);
                            cycle_depth++;
                            path_index = strlen(cycle_details);  // Update the path index

                            if (marked[k] == 1) {
                                // Cycle detected: We've already visited this resource, so we have a deadlock
                                found_cycle = 1;
                                break;
                            } else {
                                // Update resource to the next waiting resource and mark it as visited
                                resource_a = waiting_resources[k].mutex_addr;
                                marked[k] = 1;  // Mark this resource as visited
                                found = 1;
                                break;
                            }
                        }
                    }
                }

                if (found || found_cycle) break;
            }

            // If no further dependency is found or cycle detected, break out of the loop
            if (!found || found_cycle) {
                break;
            }

        }

        // If a cycle is found, print detailed information about the deadlock chain
        if (found_cycle) {
            printf("Deadlock detected!\n");

            // Print the full chain of dependencies (the cycle)
            printf("Deadlock depth: %d\n", cycle_depth);
            printf("Deadlock chain:\n");
            printf("%s\n", cycle_details);

            return 1; // Deadlock found
        }
    }

    // Reset the counts after checking all waiting resources
    waiting_count = 0;
    held_count = 0;

    return 0; // No deadlock detected
}

// Hook for pthread_mutex_lock (log when mutex is held)
int pthread_mutex_lock(pthread_mutex_t *mutex) {
    static int (*original_pthread_mutex_lock)(pthread_mutex_t *) = NULL;
    if (original_pthread_mutex_lock == NULL) {
        original_pthread_mutex_lock = dlsym(RTLD_NEXT, "pthread_mutex_lock");
    }

    pid_t pid = getpid();
    pthread_t tid = pthread_self();

    // Log the held resource
    log_waiting_resource(pid, tid, mutex);

    if(detect_deadlock()){
        exit(EXIT_FAILURE);
    }

    int result = original_pthread_mutex_lock(mutex);

    remove_resource_from_file(WAITING_RESOURCES_FILE, pid, tid, mutex);
    log_held_resource(pid, tid, mutex);

    return result;    
}

// Hook for pthread_mutex_unlock (log when mutex is released and remove from file)
int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    static int (*original_pthread_mutex_unlock)(pthread_mutex_t *) = NULL;
    if (original_pthread_mutex_unlock == NULL) {
        original_pthread_mutex_unlock = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
    }

    pid_t pid = getpid();
    pthread_t tid = pthread_self();

    // Remove the released resource from the file
    remove_resource_from_file(HELD_RESOURCES_FILE, pid, tid, mutex);

    return original_pthread_mutex_unlock(mutex);
}

// Wrapper for sem_wait to track semaphore locking
int sem_wait(sem_t *sem) {
    static int (*original_sem_wait)(sem_t *) = NULL;
    if (original_sem_wait == NULL) {
        original_sem_wait = dlsym(RTLD_NEXT, "sem_wait");
    }

    pid_t pid = getpid();
    pthread_t tid = pthread_self();

    // Log the semaphore as a waiting resource
    log_waiting_resource(pid, tid, (void *)sem);

    // Detect deadlocks
    if (detect_deadlock()) {
        printf("Deadlock detected! Terminating process %d.\n", getpid());
        exit(1);  // Kill the process to break the deadlock
    }

    int result = original_sem_wait(sem);

    // After acquiring the semaphore, remove from waiting list and log as held
    remove_resource_from_file(WAITING_RESOURCES_FILE, pid, tid, (void *)sem);
    log_held_resource(pid, tid, (void *)sem);

    return result;
}

// Wrapper for sem_post to remove semaphore resource from the graph
int sem_post(sem_t *sem) {
    static int (*original_sem_post)(sem_t *) = NULL;
    if (original_sem_post == NULL) {
        original_sem_post = dlsym(RTLD_NEXT, "sem_post");
    }

    pid_t pid = getpid();
    pthread_t tid = pthread_self();

    // Remove the semaphore from the held resources list
    remove_resource_from_file(HELD_RESOURCES_FILE, pid, tid, (void *)sem);

    // Release the semaphore and return the result
    return original_sem_post(sem);
}