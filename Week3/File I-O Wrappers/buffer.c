#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// Buffer size for storing data
#define BUFFER_SIZE 1024
extern void log_message(const char *message);

typedef struct {
    char data[BUFFER_SIZE];
    int fd;
    size_t size;
} Buffer;

// Initialize the buffer
void buffer_init(Buffer *buffer) {
    buffer->size = 0;
}

// Add data to the buffer
void buffer_add(Buffer *buffer, const char *data, size_t data_size) {
    if (buffer->size + data_size > BUFFER_SIZE) {
        // Flush the buffer if it exceeds the size limit
        buffer_flush(buffer);
    }
    memcpy(buffer->data + buffer->size, data, data_size);
    buffer->size += data_size;
}

// Flush the buffer
void buffer_flush(Buffer *buffer) {
    if (buffer->size > 0) {
        //Output the buffer data to file using a file descriptor passed as an argument
        write(buffer->fd, buffer->data, buffer->size);
        log_message("Flushed buffer data to file");
        buffer->size = 0;
    }
}

// Example usage
int main() {
    Buffer buffer;
    buffer_init(&buffer);
    buffer.fd = open("example.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    const char *data = "Hello, world!";
    buffer_add(&buffer, data, strlen(data));

    // Flush any remaining data in the buffer
    buffer_flush(&buffer);

    return 0;
}