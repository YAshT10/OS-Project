/******************************************************
 * Project: Group 3 OS project
 * Description: Shared Memory Wrappers.
 * 
 * Author(s): Memory Sub Team
 * Date: 16/Nov/2024 last editted data
 *
 ******************************************************/

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SHM_KEY 1234 // Shared memory key
#define SHM_SIZE 1024 // Shared memory size

// Wrapper for shmget
int shmget_wrapper(key_t key, size_t size, int shmflg) {
    int shmid = shmget(key, size, shmflg);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

// Wrapper for shmat
void* shmat_wrapper(int shmid, const void *shmaddr, int shmflg) {
    void *shm_ptr = shmat(shmid, shmaddr, shmflg);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    return shm_ptr;
}

// Wrapper for shmdt
void shmdt_wrapper(void *shmaddr) {
    if (shmdt(shmaddr) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
}

// Wrapper for shmctl
void shmctl_wrapper(int shmid, int cmd, struct shmid_ds *buf) {
    if (shmctl(shmid, cmd, buf) == -1) {
        perror("shmctl failed");
        exit(EXIT_FAILURE);
    }
}

void set_permission(int shmid, int permissions) {
    struct shmid_ds shm_ds;
    
    // Get current shared memory info
    if (shmctl(shmid, IPC_STAT, &shm_ds) == -1) {
        perror("shmctl IPC_STAT");
        exit(1);
    }
    
    // Set the new permissions
    shm_ds.shm_perm.mode = permissions;

    // Set the new permission on the shared memory segment
    if (shmctl(shmid, IPC_SET, &shm_ds) == -1) {
        perror("shmctl IPC_SET");
        exit(1);
    }

    printf("Permissions updated successfully.\n");
}


// Example usage
int main() {
    int shmid;
    void *shm_ptr;
    struct shmid_ds shm_info;
    shmid = shmget_wrapper(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    shm_ptr = shmat_wrapper(shmid, NULL, 0);
    strcpy((char *)shm_ptr, "This is shared memory!");
    printf("Data in shared memory: %s\n", (char *)shm_ptr);
    shmdt_wrapper(shm_ptr);
    shmctl_wrapper(shmid, IPC_STAT, &shm_info);
    shmctl_wrapper(shmid, IPC_RMID, NULL);

    return 0;
}

