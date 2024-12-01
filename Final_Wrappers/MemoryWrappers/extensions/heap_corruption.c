#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

#define MAX_LOG_SIZE 200
#define HEAP_CORRUPTION_LOG "heap_corruption_log.txt"

static pthread_mutex_t heap_mutex = PTHREAD_MUTEX_INITIALIZER;

static size_t heap_size = 0;
static unsigned int heap_checksum = 0;

unsigned int calculate_heap_checksum()
{
    unsigned int checksum = 0;
    checksum ^= heap_size;
    return checksum;
}

void validate_heap_integrity()
{
    unsigned int expected_checksum = calculate_heap_checksum();
    if (expected_checksum != heap_checksum)
    {
        FILE *log_file = fopen(HEAP_CORRUPTION_LOG, "a");
        if (log_file)
        {
            fprintf(log_file, "Heap corruption detected! Expected checksum: %u, but got: %u\n", expected_checksum, heap_checksum);
            fclose(log_file);
        }
        printf("Heap corruption detected! Exiting...\n");
        exit(EXIT_FAILURE);
    }
}

void *malloc(size_t size)
{
    static void *(*original_malloc)(size_t) = NULL;
    if (original_malloc == NULL)
    {
        original_malloc = dlsym(RTLD_NEXT, "malloc");
    }

    void *ptr = original_malloc(size);
    if (ptr)
    {
        pthread_mutex_lock(&heap_mutex);
        heap_size += size;
        heap_checksum = calculate_heap_checksum();
        pthread_mutex_unlock(&heap_mutex);
    }
    validate_heap_integrity();
    return ptr;
}

void free(void *ptr)
{
    static void (*original_free)(void *) = NULL;
    if (original_free == NULL)
    {
        original_free = dlsym(RTLD_NEXT, "free");
    }

    if (ptr)
    {
        size_t size_to_free = malloc_usable_size(ptr);
        pthread_mutex_lock(&heap_mutex);
        heap_size -= size_to_free;
        pthread_mutex_unlock(&heap_mutex);
    }

    original_free(ptr);
    heap_checksum = calculate_heap_checksum();
    validate_heap_integrity();
}

void *calloc(size_t num, size_t size)
{
    static void *(*original_calloc)(size_t, size_t) = NULL;
    if (original_calloc == NULL)
    {
        original_calloc = dlsym(RTLD_NEXT, "calloc");
    }

    void *ptr = original_calloc(num, size);
    if (ptr)
    {
        pthread_mutex_lock(&heap_mutex);
        heap_size += num * size;
        heap_checksum = calculate_heap_checksum();
        pthread_mutex_unlock(&heap_mutex);
    }
    validate_heap_integrity();
    return ptr;
}

void *realloc(void *ptr, size_t size)
{
    static void *(*original_realloc)(void *, size_t) = NULL;
    if (original_realloc == NULL)
    {
        original_realloc = dlsym(RTLD_NEXT, "realloc");
    }

    if (ptr)
    {
        pthread_mutex_lock(&heap_mutex);
        heap_size -= malloc_usable_size(ptr);
        pthread_mutex_unlock(&heap_mutex);
    }

    void *new_ptr = original_realloc(ptr, size);
    if (new_ptr)
    {
        pthread_mutex_lock(&heap_mutex);
        heap_size += size;
        heap_checksum = calculate_heap_checksum();
        pthread_mutex_unlock(&heap_mutex);
    }
    validate_heap_integrity();
    return new_ptr;
}
