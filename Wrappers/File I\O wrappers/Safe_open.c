#include "logger.h"
#include "Safe_open.h"
#define VALID_FLAGS (O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_EXCL | O_TRUNC | O_APPEND | O_NONBLOCK | O_SYNC | O_CLOEXEC)

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

int validate_flags(int flags) {
    return (flags & ~VALID_FLAGS) == 0; // Check if flags contain any unsupported bits
}

int safe_open(const char* filename, int flags , mode_t mode) {

    //Check if flags are valid
    if (!validate_flags(flags)) {
        fprintf(stderr, "Error: Invalid flags\n");
        exit(EXIT_FAILURE);
    }

    // If mode is "r", check if the file exists
    if (flags == O_RDONLY && access(filename, F_OK) == -1) {
        //call the access_path function
        check_path(filename);
        exit(EXIT_FAILURE);
    }

    if((flags == O_WRONLY || flags == O_RDWR) && access(filename, F_OK) == -1){
        printf("File %s doesn't exist ; Do you want to create a new file? Y/N \n", filename);
        char c;
        scanf(" %c", &c);
        if(c == 'Y' || c == 'y'){
            FILE *file = fopen(filename, "w");
            if (file == NULL) {
                perror("Error creating file");
                exit(EXIT_FAILURE);
            }
            fclose(file);
        }
        else{
            exit(EXIT_FAILURE);
        }
    }

    int file = open_logger(filename, flags , mode);
    if (file == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    return file;
}