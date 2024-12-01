#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#define MAX_OPERATIONS 10
#define TIME_WINDOW 60 // in seconds

typedef struct {
    int operations;
    time_t start_time;
} RateLimiter;

// Initialize the rate limiter
void rate_limiter_init(RateLimiter *limiter) {
    limiter->operations = 0;
    limiter->start_time = time(NULL);
}

// Check if the rate limit is exceeded
int rate_limiter_check(RateLimiter *limiter) {
    time_t current_time = time(NULL);
    if (difftime(current_time, limiter->start_time) > TIME_WINDOW) {
        limiter->operations = 0;
        limiter->start_time = current_time;
    }
    if (limiter->operations >= MAX_OPERATIONS) {
        
        return -1; // Rate limit exceeded
    }
    limiter->operations++;
    return 0;
}

// Wrapped open function with rate limiting
int rate_limited_open(RateLimiter *limiter, const char *pathname, int flags) {
    if (rate_limiter_check(limiter) == -1) {
        errno = EAGAIN; // Resource temporarily unavailable
        return -1;
    }
    return open(pathname, flags);
}

// Wrapped read function with rate limiting
ssize_t rate_limited_read(RateLimiter *limiter, int fd, void *buf, size_t count) {
    if (rate_limiter_check(limiter) == -1) {
        errno = EAGAIN; // Resource temporarily unavailable
        fprintf(stderr, "Rate limit exceeded\n");
        return -1;
    }
    return read(fd, buf, count);
}

// Example usage
int main() {
    RateLimiter limiter;
    rate_limiter_init(&limiter);

    int fd = rate_limited_open(&limiter, "example.txt", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }
    for(int i =0 ;i<15;i++)
    {
    char buffer[2];
    ssize_t bytes_read = rate_limited_read(&limiter, fd, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return 1;
    }

    buffer[bytes_read] = '\0';
    printf("Read: %s\n", buffer);
    }
    close(fd);
    return 0;
}