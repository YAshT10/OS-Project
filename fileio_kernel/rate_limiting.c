#include <linux/time.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "rate_limiting.h"
#include "Safe_open.h"
#include "Safe_read.h"

// Initialize the rate limiter
void rate_limiter_init(RateLimiter *limiter) {
    limiter->operations = 0;
    limiter->start_time = ktime_get_seconds();
}

// Check if the rate limit is exceeded
int rate_limiter_check(RateLimiter *limiter) {
    time64_t current_time = ktime_get_seconds();
    if (current_time - limiter->start_time > TIME_WINDOW) {
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
int rate_limited_open(RateLimiter *limiter, const char *pathname, int flags, umode_t mode) {
    if (rate_limiter_check(limiter) == -1) {
        printk(KERN_WARNING "Rate limit exceeded\n");
        return -EAGAIN; // Resource temporarily unavailable
    }
    return safe_open(pathname, flags, mode);
}

// Wrapped read function with rate limiting
ssize_t rate_limited_read(RateLimiter *limiter, int fd, void *buf, size_t count) {
    if (rate_limiter_check(limiter) == -1) {
        printk(KERN_WARNING "Rate limit exceeded\n");
        return -EAGAIN; // Resource temporarily unavailable
    }
    return safe_read(fd, buf, count);
}