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

int munmap(void *addr, size_t length){
    if (!addr){
        fprintf(stderr, "safe_munmap: NULL address provided.\n");
        return;
    }
    int (*original_munmap)(void *addr, size_t length)=NULL;
    if (original_munmap == NULL) {
        original_munmap = dlsym(RTLD_NEXT, "mmap");
        if (!original_munmap) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }
    allocated_chunk *prev = NULL;
    allocated_chunk *current = alloc_list;
    while (current){
        if (current->addr == addr){
            if (!current->is_used){
                fprintf(stderr, "safe_munmap: Address %p is already unmapped.\n", addr);
                return;
            }
            if (original_munmap(addr, current->size) == -1){
                switch (errno){
                    case EINVAL:
                        fprintf(stderr, "munmap failed: Invalid address or size.\n");
                        break;
                    case ENOMEM:
                        fprintf(stderr, "munmap failed: Address is outside the address space of the process.\n");
                        break;
                    default:
                        fprintf(stderr, "munmap failed: Unknown error (errno: %d).\n", errno);
                        break;
                }
                return;
            }
            printf("Memory unmapped at %p (size: %zu bytes)\n", addr, current->size);
            current->is_used = 0;
            return;
        }
        prev = current;
        current = current->next;
    }
    fprintf(stderr, "safe_munmap: Address %p not found in allocation list.\n", addr);
}
