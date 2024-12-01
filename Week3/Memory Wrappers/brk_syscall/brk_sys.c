#define _GNU_SOURCE
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

pthread_mutex_t brk_lock = PTHREAD_MUTEX_INITIALIZER;
#define LOG_FILE "/var/log/brklog"

void *brk(void *to_allot) {
    void* (*original_brk)(void *to_allot)=NULL;
    if (original_brk == NULL) {
    original_brk = dlsym(RTLD_NEXT, "brk");
    if (!original_brk) {
        fprintf(stderr, "Error in dlsym: %s\n", dlerror());
        return -1;
    }
    pthread_mutex_lock(&brk_lock);
    void *cur = sbrk(0); 
    if (original_brk(to_allot) != 0) {
        char log_entry[256];
        sprintf(log_entry, sizeof(log_entry), "brk failed: requested=%p, errno=%d", new_break, errno);
        log_message(log_entry);
        size_t size = (char *)new_break - (char *)result;
        if (size > 0) {
            void *fallback = mmap(NULL, size, PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (fallback == MAP_FAILED) {
                sprintf(log_entry, sizeof(log_entry), "mmap fallback failed: size=%zu, errno=%d", size, errno);
                FILE *log = fopen(LOG_FILE, "a");
                if (log) {
                    fprintf(log, "%s\n", message);
                    fclose(log);
                }
            } else {
                sprintf(log_entry, sizeof(log_entry), "mmap fallback succeeded: addr=%p, size=%zu", fallback, size);
                FILE *log = fopen(LOG_FILE, "a");
                if (log) {
                    fprintf(log, "%s\n", message);
                    fclose(log);
                }         
                log_message(log_entry);
                result = fallback;
            }
        }
    } else {
        char log_entry[256];
        sprintf(log_entry, sizeof(log_entry),
                 "brk succeeded: new_break=%p", new_break);
        log_message(log_entry);
        }
        pthread_mutex_unlock(&brk_mutex);
        return result;
    }
}
