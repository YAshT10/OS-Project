#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

sem_t sem1;
sem_t sem2;

// Mutex deadlock testing function
void *test_mutex_deadlock(void *arg) {
    printf("Thread 1: Trying to lock Mutex 1\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 1: Mutex 1 locked\n");
    sleep(1);  // Simulate some work

    printf("Thread 1: Trying to lock Mutex 2\n");
    pthread_mutex_lock(&mutex2);
    printf("Thread 1: Mutex 2 locked\n");

    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

void *test_mutex_deadlock_reverse(void *arg) {
    printf("Thread 2: Trying to lock Mutex 2\n");
    pthread_mutex_lock(&mutex2);
    printf("Thread 2: Mutex 2 locked\n");
    sleep(1);  // Simulate some work

    printf("Thread 2: Trying to lock Mutex 1\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 2: Mutex 1 locked\n");

    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    return NULL;
}

// Semaphore deadlock testing function
void *test_semaphore_deadlock(void *arg) {
    printf("Thread 1: Trying to wait on Semaphore 1\n");
    sem_wait(&sem1);
    printf("Thread 1: Semaphore 1 locked\n");
    sleep(1);  // Simulate some work

    printf("Thread 1: Trying to wait on Semaphore 2\n");
    sem_wait(&sem2);
    printf("Thread 1: Semaphore 2 locked\n");

    sem_post(&sem2);
    sem_post(&sem1);
    return NULL;
}

void *test_semaphore_deadlock_reverse(void *arg) {
    printf("Thread 2: Trying to wait on Semaphore 2\n");
    sem_wait(&sem2);
    printf("Thread 2: Semaphore 2 locked\n");
    sleep(1);  // Simulate some work

    printf("Thread 2: Trying to wait on Semaphore 1\n");
    sem_wait(&sem1);
    printf("Thread 2: Semaphore 1 locked\n");

    sem_post(&sem1);
    sem_post(&sem2);
    return NULL;
}

int main() {
    pthread_t thread1, thread2;

    // Initialize the semaphores
    sem_init(&sem1, 0, 1);  // Semaphore initialized to 1 (binary semaphore)
    sem_init(&sem2, 0, 1);  // Semaphore initialized to 1 (binary semaphore)

    // Test Mutex Deadlock
    printf("Testing Mutex Deadlock...\n");
    pthread_create(&thread1, NULL, test_mutex_deadlock, NULL);
    pthread_create(&thread2, NULL, test_mutex_deadlock_reverse, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Test Semaphore Deadlock
    printf("Testing Semaphore Deadlock...\n");
    pthread_create(&thread1, NULL, test_semaphore_deadlock, NULL);
    pthread_create(&thread2, NULL, test_semaphore_deadlock_reverse, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Destroy semaphores
    sem_destroy(&sem1);
    sem_destroy(&sem2);

    return 0;
}
