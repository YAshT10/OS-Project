#include "buffer.h"

// Initialize the buffer
void buffer_init(Buffer *buffer , int fd) {
    buffer->size = 0;
    buffer->fd = fd;
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