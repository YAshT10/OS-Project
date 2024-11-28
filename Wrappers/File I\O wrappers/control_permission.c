#include "control_permission.h"

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

    if (strcmp(pw->pw_name, "authorized_user") != 0) {
        return -1; // Permission denied
    }

    return 0; // Permission granted
}

// Wrapped open function with access control
int controlled_open(const char *pathname, int flags) {
    if (check_permission(pathname) == -1) {
        errno = EACCES; // Permission denied
        return -1;
    }
    return 0; // Success
}   