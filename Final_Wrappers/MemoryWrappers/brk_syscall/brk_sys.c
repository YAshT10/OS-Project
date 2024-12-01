#define _GNU_SOURCE
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

pthread_mutex_t brk_lock = PTHREAD_MUTEX_INITIALIZER;
#define LOG_FILE "/tmp/log/brklog"

int brk(void *to_allot) {
    static int (*original_brk)(void *to_allot) = NULL;
    if (!original_brk) {
        original_brk = dlsym(RTLD_NEXT, "brk");
        if (!original_brk) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    pthread_mutex_lock(&brk_lock);
    void *current_brk = sbrk(0);
    int result = original_brk(to_allot);

    if (result != 0) {
        char log_entry[256];
        snprintf(log_entry, sizeof(log_entry), "brk failed with error code: %d", errno);
        FILE *log = fopen(LOG_FILE, "a");
        if (log) {
            fprintf(log, "%s\n", log_entry);
            fclose(log);
        }
        size_t size = (char *)to_allot - (char *)current_brk;
        if (size > 0) {
            void *fallback = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            char log_entry[256];
            FILE *log = fopen(LOG_FILE, "a");
            if (log) {
                if (fallback == MAP_FAILED) {
                    snprintf(log_entry, sizeof(log_entry), "mmap failed with error code: errno=%d", errno);   
                }
                else{
                    snprintf(log_entry, sizeof(log_entry), "mmap fallback succeeded: addr=%p, size=%zu", fallback, size);
                    result = 0; //returning success
                }
                fprintf(log, "%s\n", log_entry);
                fclose(log);
            }
        }
    } 
    else{
        char log_entry[256];
        snprintf(log_entry, sizeof(log_entry), "brk succeeded: new break=%p", to_allot);
        FILE *log = fopen(LOG_FILE, "a");
        if (log) {
            fprintf(log, "%s\n", log_entry);
            fclose(log);
        }
    }
    pthread_mutex_unlock(&brk_lock);
    return result;
}

