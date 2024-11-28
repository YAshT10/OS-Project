#include "Safe_open.h"
#include "logger.h"

void check_path(const char* path) {
    char temp[PATH_MAX];
    char* p = NULL;
    size_t len;

    snprintf(temp, sizeof(temp), "%s", path);
    len = strlen(temp);

    if (temp[len - 1] == '/') {
        temp[len - 1] = 0;
    }

    for (p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (access(temp, F_OK) != 0) {
                // fprintf(stderr, "Error: Directory %s doesn't exist\n", temp);
                exit(EXIT_FAILURE);
            }
            *p = '/';
        }
    }

    if (access(temp, F_OK) != 0) {
        // fprintf(stderr, "Error: File %s doesn't exist\n", temp);
        exit(EXIT_FAILURE);
    }
}

int safe_open(const char* filename, int flags , mode_t mode) {
    // Check if the mode is valid
    if (flags != O_RDONLY && flags != O_WRONLY && flags != O_RDWR) {
        perror("Error: Invalid mode\n");
        exit(EXIT_FAILURE);
    }

    // If mode is "r", check if the file exists
    if (flags == O_RDONLY && access(filename, F_OK) == -1) {
        //call the access_path function
        check_path(filename);
        exit(EXIT_FAILURE);
    }

    int file = open_logger(filename, flags , mode);
    if (file == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    return file;
}