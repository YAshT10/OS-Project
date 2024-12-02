#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

// Wrapper for aligned memory allocation
void* aligned_mmap(size_t size, size_t alignment) {
    // Ensure alignment is a power of 2 and at least the size of a pointer
    if (alignment & (alignment - 1) || alignment < sizeof(void*)) {
        errno = EINVAL;
        return NULL;
    }

    // Calculate the total size needed (including space for alignment adjustments)
    size_t total_size = size + alignment - 1;

    // Use mmap to allocate the required memory
    void* raw_memory = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw_memory == MAP_FAILED) {
        return NULL;
    }

    // Calculate the aligned memory address
    uintptr_t raw_address = (uintptr_t)raw_memory;
    uintptr_t aligned_address = (raw_address + alignment - 1) & ~(alignment - 1);

    // Store the original base address just before the aligned address
    ((uintptr_t*)aligned_address)[-1] = raw_address;

    // Return the aligned memory pointer
    return (void*)aligned_address;
}

// Wrapper for aligned memory deallocation
int aligned_munmap(void* aligned_ptr, size_t size) {
    if (!aligned_ptr) {
        errno = EINVAL;
        return -1;
    }

    // Retrieve the original base address stored before the aligned memory
    uintptr_t raw_address = ((uintptr_t*)aligned_ptr)[-1];

    // Calculate the total size initially allocated
    size_t total_size = size + ((uintptr_t)aligned_ptr - raw_address);

    // Use munmap to deallocate the memory
    return munmap((void*)raw_address, total_size);
}

int main() {
    // Example usage
    size_t size = 1024;        // Allocate 1KB
    size_t alignment = 64;     // 64-byte alignment

    void* aligned_memory = aligned_mmap(size, alignment);
    if (aligned_memory == NULL) {
        perror("aligned_mmap failed");
        return EXIT_FAILURE;
    }

    printf("Aligned memory allocated at: %p\n", aligned_memory);

    // Deallocate the aligned memory
    if (aligned_munmap(aligned_memory, size) != 0) {
        perror("aligned_munmap failed");
        return EXIT_FAILURE;
    }

    printf("Aligned memory successfully deallocated.\n");
    return EXIT_SUCCESS;
}
