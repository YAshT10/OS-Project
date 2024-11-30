#include "buffer.h"
#include "control_permission.h"
#include "logger.h"

// Initialize the buffer
void buffer_init(Buffer *buffer , int fd) {
    buffer->size = 0;
    buffer->fd = fd;
    buffer->data[0] = '\0';
}

// Add data to the buffer
void buffer_add(Buffer *buffer, const char *data, size_t data_size) {
    if(buffer->fd == -1) {
        return;
    }
    if (buffer->size + data_size > BUFFER_SIZE) {
        // Flush the buffer if it exceeds the size limit
        buffer_flush(buffer);
    }
    memcpy(buffer->data + buffer->size, data, data_size);
    buffer->size += data_size;
    log_message("Wrote to buffer");
}

// Flush the buffer
void buffer_flush(Buffer *buffer) {
    if(buffer->fd == -1) {
        return;
    }
    if (buffer->size > 0) {
        //Output the buffer data to file using a file descriptor passed as an argument
        buffer_logger(buffer->fd, buffer->data, buffer->size);
        buffer->size = 0;
    }
}