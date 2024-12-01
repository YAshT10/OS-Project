/******************************************************
 * Project: Group 3 OS project
 * Description: File open and closing wrappers.
 * 
 * Author(s): Memory Sub Team
 * Date: 16/Nov/2024 wrote major code 
 * Date: 28/Nov/2024 Added hooks: last ediited data
 *
 ******************************************************/

#define _GNU_SOURCE
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHM_KEY 1234 
#define SHM_SIZE 1024 

int shmget(key_t key, size_t size, int shmflg) {
    int (*original_shmget)(key_t key, size_t size, int shmflg) = NULL;
    if (!original_shmget) {
        original_shmget = dlsym(RTLD_NEXT, "shmget");
        if (!original_shmget) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }
    int shmid = original_shmget(key, size, shmflg);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    return shmid;
}


void* shmat(int shmid, const void *shmaddr, int shmflg) {
    void* (*original_shmat)(int shmid, const void *shmaddr, int shmflg) = NULL;
    if (!original_shmat) {
        original_shmat = dlsym(RTLD_NEXT, "shmat");
        if (!original_shmat) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }
    void *shm_ptr = original_shmat(shmid, shmaddr, shmflg);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    return shm_ptr;
}

void shmdt(void *shmaddr) {
    void (*original_shmdt)(void *shmaddr) = NULL;
    if (!original_shmdt) {
        original_shmdt = dlsym(RTLD_NEXT, "shmdt");
        if (!original_shmdt) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }
    if (original_shmdt(shmaddr) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
}

void shmctl(int shmid, int cmd, struct shmid_ds *buf) {
    void (*original_shmctl)(int shmid, int cmd, struct shmid_ds *buf) = NULL;
    if (!original_shmctl) {
        original_shmctl = dlsym(RTLD_NEXT, "shmctl");
        if (!original_shmctl) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            return -1;
        }
    }
    if (original_shmctl(shmid, cmd, buf) == -1) {
        perror("shmctl failed");
        exit(EXIT_FAILURE);
    }
}

void set_permission(int shmid, int permissions) {
    struct shmid_ds shm_ds;
    
    if (shmctl(shmid, IPC_STAT, &shm_ds) == -1) {
        perror("shmctl IPC_STAT");
        exit(1);
    }
    
    shm_ds.shm_perm.mode = permissions;

    if (shmctl(shmid, IPC_SET, &shm_ds) == -1) {
        perror("shmctl IPC_SET");
        exit(1);
    }

    printf("Permissions updated successfully.\n");
}


int main() {
    int shmid;
    void *shm_ptr;
    struct shmid_ds shm_info;
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    shm_ptr = shmat(shmid, NULL, 0);
    strcpy((char *)shm_ptr, "This is shared memory!");
    printf("Data in shared memory: %s\n", (char *)shm_ptr);
    shmdt(shm_ptr);
    shmctl(shmid, IPC_STAT, &shm_info);
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}