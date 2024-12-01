#include "control_permission.h"
#include "logger.h"
// Function to check if the user has permission to access the file
int check_permission(const char *pathname) {
    // Get the current user's UID
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    if (pw == NULL) {
        return -1; // Failed to get user information
    }

    // Example: Only allow user "authorized_user" to access the file
    // Can change this to check for a specific group or other criteria
    // Depending on the access control policy and file system implementation
    // Files may also maintain some passwords in their metadata which can be matched with the user's password

    //put your user name here instead of "authorized_user" like mine is bruh
    if (strcmp(pw->pw_name, "bruh") != 0) {
        return -1; // Permission denied
    }

    return 0; // Permission granted
}

int check_read_fd(int fd) {
    ssize_t (*orig_read)(int, void *, size_t) = dlsym(RTLD_NEXT, "read");
    if (orig_read(fd , NULL , -1) == -1) {
        if (errno == EBADF) {
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Invalid file descriptor for reading ; fd : %d", fd);
            log_message(log_msg);
            return 0;
        }
    }
    return 1; // FD is valid
}

int check_write_fd(int fd) {
    ssize_t (*orig_write)(int, const void *, size_t) = dlsym(RTLD_NEXT, "write");
    if (orig_write(fd, NULL, -1) == -1) {
        if (errno == EBADF) {
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Invalid file descriptor for writing ; fd : %d", fd);
            log_message(log_msg);
            return 0;
        }
    }
    return 1; // FD is valid
}

// Wrapped open function with access control
int controlled_open(const char *pathname, int flags) {
    if (check_permission(pathname) == -1) {
        errno = EACCES; // Permission denied
        return -1;
    }
    return 0; // Success
}   