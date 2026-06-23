#define _DEFAULT_SOURCE      

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define ITEMS_TO_PRODUCE  20

    int  count = 0;     
    int  in    = 0;    
    int  out   = 0; 
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t  not_full  = PTHREAD_COND_INITIALIZER;   /* room to put */
    pthread_cond_t  not_empty = PTHREAD_COND_INITIALIZER;   /* item to take */

    int *buffer = NULL;      
    int max_capacity = 0;    
    int consumer_counts[2] = {0, 0};
    long consumer_sums[2] = {0, 0};

void buffer_init(int capacity) {
    max_capacity = capacity;
    buffer = (int *)malloc(capacity * sizeof(int));
}

void buffer_put(int item) {
    pthread_mutex_lock(&lock);
    while (count == max_capacity) {
        printf("[producer] buffer full, waiting...\n");
        fflush(stdout); // Force instant print
        pthread_cond_wait(&not_full, &lock);
    }
        
    buffer[in] = item;
    in = (in + 1) % max_capacity;
    count++;
    printf("[producer] produced %d  (buffer now has %d)\n", item , count);
    fflush(stdout); // Force instant print while holding the lock

    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&lock);
}

int buffer_get(void) {
    pthread_mutex_lock(&lock);
    while (count == 0) {
        printf("[consumer] buffer empty, waiting...\n");
        fflush(stdout); // Force instant print
        pthread_cond_wait(&not_empty, &lock);
    }

    int item = buffer[out];
    out = (out + 1) % max_capacity;
    count--;
    printf("[consumer] consumed %d  (buffer now has %d)\n", item, count);
    fflush(stdout); // Force instant print while holding the lock

    pthread_cond_signal(&not_full);
    pthread_mutex_unlock(&lock);
    return item;
}

void buffer_destroy(void) {
  pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    for(int i=0 ; i< max_capacity ; i++){
        buffer[i] = 0;
    }
    count=0;
    free(buffer);
}
void *producer(void *arg) {
    long N=(long)arg;
    int range_start= N*1000;
    int range_end = N*1000 + 999 ;
    int item = range_start;
    for(int i=0 ; i<1000 ; i++){
        buffer_put(item);
        item++;
    }
    return NULL;
}
void *consumer(void *arg){
    long id = (long)arg;
    int counter_consumed=0;
    for(int i =0 ; i<1500 ; i++){
        int get =buffer_get();
        consumer_counts[id]++;
        consumer_sums[id] += get;
    }
    return NULL;
}



int main(void) {
    pthread_t producers[3];
    pthread_t consumers[2];
    buffer_init(10);
     for (long i = 0; i < 3; i++) {
        pthread_create(&producers[i], NULL, producer, (void *)i);
    }
    for (long i = 0; i < 2; i++) {
        pthread_create(&consumers[i], NULL, consumer, (void *)i);
    }
   
    
    for (int i = 0; i < 3; i++) {
        pthread_join(producers[i], NULL);
    }
    
    
    for (int i = 0; i < 2; i++) {
        pthread_join(consumers[i], NULL);
    }
    int total_items_consumed = consumer_counts[0] + consumer_counts[1];
    long total_sum_consumed = consumer_sums[0] + consumer_sums[1];
    printf("\n--- VERIFICATION REPORT ---\n");
    printf("Total Items Processed: %d (Expected: 3000)\n", total_items_consumed);
    printf("Total Combined Sum:    %ld (Expected: 4498500)\n", total_sum_consumed);
    buffer_destroy();
    return 0;
}