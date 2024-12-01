#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct AllocRecord {
    void *ptr;
    size_t size;
    struct AllocRecord *next;
} AllocRecord;

AllocRecord *alloc_head = NULL;

void track_allocation(void *ptr, size_t size) {
    AllocRecord *new_record = malloc(sizeof(AllocRecord));
    if (!new_record) {
        fprintf(stderr, "debug_malloc: Failed to track allocation. Out of memory.\n");
        return;
    }
    new_record->ptr = ptr;
    new_record->size = size;
    new_record->next = alloc_head;
    alloc_head = new_record;

    printf("Allocated %zu bytes at %p\n", size, ptr);
}

void untrack_allocation(void *ptr) {
    AllocRecord *prev = NULL, *current = alloc_head;

    while (current) {
        if (current->ptr == ptr) {
            if (prev) {
                prev->next = current->next;
            } else {
                alloc_head = current->next;
            }
            printf("Freed %zu bytes at %p\n", current->size, ptr);
            free(current); 
            return;
        }
        prev = current;
        current = current->next;
    }

    fprintf(stderr, "debug_free: Attempt to free untracked or already freed memory at %p\n", ptr);
}

void *debug_malloc(size_t size) {
    if (size == 0) {
        fprintf(stderr, "debug_malloc: Zero size allocation request ignored.\n");
        return NULL;
    }

    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "debug_malloc: Allocation failed. Out of memory.\n");
        return NULL;
    }

    track_allocation(ptr, size);
    return ptr;
}

void debug_free(void *ptr) {
    if (!ptr) {
        fprintf(stderr, "debug_free: Attempt to free a NULL pointer ignored.\n");
        return;
    }

    untrack_allocation(ptr);
    free(ptr);
}

void detect_memory_leaks() {
    AllocRecord *current = alloc_head;
    if (!current) {
        printf("No memory leaks detected.\n");
        return;
    }

    printf("Memory leaks detected:\n");
    while (current) {
        printf("  Leak: %zu bytes at %p\n", current->size, current->ptr);
        current = current->next;
    }
}
