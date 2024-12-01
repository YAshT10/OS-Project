#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

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
                fprintf(stderr, "Error: Directory %s doesn't exist\n", temp);
                exit(EXIT_FAILURE);
            }
            *p = '/';
        }
    }

    if (access(temp, F_OK) != 0) {
        fprintf(stderr, "Error: File %s doesn't exist\n", temp);
        exit(EXIT_FAILURE);
    }
}

FILE* safe_open(const char* filename, const char* mode) {
    // Check if the mode is valid
    if (mode == NULL || (strcmp(mode, "r") != 0 && strcmp(mode, "w") != 0 && strcmp(mode, "a") != 0)) {
        fprintf(stderr, "Error: Invalid mode %s\n", mode);
        exit(EXIT_FAILURE);
    }

    // If mode is "r", check if the file exists
    if (strcmp(mode, "r") == 0 && access(filename, F_OK) == -1) {
        //call teh access_path function
        check_path(filename);
        exit(EXIT_FAILURE);
    }

    FILE* file = fopen(filename, mode);
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    return file;
}

int main() {
    // Example usage of safe_open
    FILE* file = safe_open("dir_1/dir_3/example.txt", "r");
    
    // Perform file operations here

    fclose(file);
    return 0;
}