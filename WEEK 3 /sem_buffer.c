#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

//BUFFFER STATE
int *buffer;
int capacity;
int in = 0;
int out = 0;

sem_t empty_slots; 
sem_t full_slots; 
pthread_mutex_t mutex; 
void buffer_init(int cap){
    capacity = cap;
    buffer = (int*)malloc(capacity*sizeof(int));
    //initializing semaphores
    sem_init(&empty_slots, 0, capacity); 
    sem_init(&full_slots, 0, 0); 

    pthread_mutex_init(&mutex, NULL);
}

void buffer_put(int item){
   
    sem_wait(&empty_slots);

    
    pthread_mutex_lock(&mutex);
    buffer[in] = item;
    in = (in + 1) % capacity;
    pthread_mutex_unlock(&mutex);

    
    sem_post(&full_slots);
}

int buffer_get(void) {
    
    sem_wait(&full_slots);
    
    
    pthread_mutex_lock(&mutex);
    int item = buffer[out];
    out = (out + 1) % capacity;
    pthread_mutex_unlock(&mutex);
    
   
    sem_post(&empty_slots);

    return item;
}

void buffer_destroy(void) {
    free(buffer);
    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    pthread_mutex_destroy(&mutex);
}

#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 3
#define ITEMS_PER_PRODUCER 10

void *producer(void *arg) {
    int id = *((int *)arg);
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = (id * 100) + i; // Generate a unique item
        buffer_put(item);
        printf("Producer %d put: %d\n", id, item);
        usleep(10000); // Simulate work
    }
    return NULL;
}

void *consumer(void *arg) {
    int id = *((int *)arg);
    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = buffer_get();
        printf("Consumer %d got: %d\n", id, item);
        usleep(15000); // Simulate work
    }
    return NULL;
}

int main() {
    printf("--- Testing Semaphore Bounded Buffer ---\n");
    
   buffer_init(5); 

    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int thread_ids[NUM_PRODUCERS > NUM_CONSUMERS ? NUM_PRODUCERS : NUM_CONSUMERS];

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer, &thread_ids[i]);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer, &thread_ids[i]);
    }

    // Join all threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    buffer_destroy();
    printf("--- Execution Complete ---\n");
    return 0;
}    
