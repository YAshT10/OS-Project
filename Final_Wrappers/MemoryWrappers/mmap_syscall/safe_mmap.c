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

typedef struct allocated_chunk {
    void *addr;
    size_t size;
    int is_used;
    struct allocated_chunk *next;
} allocated_chunk;

allocated_chunk *alloc_list = NULL;

void track_allocation(void *addr, size_t size) {
    allocated_chunk *new_alloc = malloc(sizeof(allocated_chunk));
    new_alloc->addr = addr;
    new_alloc->size = size;
    new_alloc->is_used = 1;
    new_alloc->next = alloc_list;
    alloc_list = new_alloc;
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset){
    void* (*original_mmap)(void *addr, size_t length, int prot, int flags, int fd, off_t offset)=NULL;
    if (original_mmap == NULL) {
        original_mmap = dlsym(RTLD_NEXT, "mmap");
        if (!original_mmap) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }

    void *addr = original_mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (addr == MAP_FAILED) {
        switch (errno) {
            case EINVAL:
                fprintf(stderr, "mmap failed: Invalid argument (check addr, length, offset, or flags)\n");
                break;
            case EACCES:
                fprintf(stderr, "mmap failed: Permission denied (check file permissions or protections)\n");
                break;
            case ENOMEM:
                fprintf(stderr, "mmap failed: Out of memory (check system resources)\n");
                break;
            case EBADF:
                fprintf(stderr, "mmap failed: Invalid file descriptor\n");
                break;
            case ENODEV:
                fprintf(stderr, "mmap failed: Mapping not supported for this file\n");
                break;
            case ENXIO:
                fprintf(stderr, "mmap failed: No such device or address (check offset and file size)\n");
                break;
            case EOVERFLOW:
                fprintf(stderr, "mmap failed: Offset or length exceeds file size or addressable range\n");
                break;
            default:
                break;
        }
        return NULL;
    }
    track_allocation(addr, size);
    printf("Memory mapped at %p (size: %zu bytes)\n", addr, size);
    return addr;
}

void log_memory_usage() {
    allocated_chunk *current = alloc_list;
    size_t total_memory = 0;
    while (current) {
        if (current->is_used) {
            total_memory += current->size;
            printf("Chunk address: %d, Size allocated: %d\n", current->addr, current-> size);
        }
        current = current->next;
    }
    printf("Total memory allocated: %zu bytes\n", total_memory);
}
