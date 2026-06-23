#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#define t 2
long counter = 0; 

typedef struct {
    long initial;
    long final;
} thread_arg_t;

void *partial_sum(void *arg) {
    thread_arg_t *bounds = (thread_arg_t *)arg;
    int in_word = 0, ch;
    
   
    FILE *local_fp = fopen("example.txt", "r");
    if (local_fp == NULL) {
        return NULL; 
    }
    
    
    fseek(local_fp, bounds->initial, SEEK_SET);
    
    
    while (ftell(local_fp) < bounds->final && (ch = fgetc(local_fp)) != EOF) {
        if (isspace(ch)) {
            in_word = 0; 
        } 
        else if (!in_word) {
            in_word = 1; 
            
            long temp = counter;
            usleep(2);
            counter = temp + 1; 
            
        }
    }
    
    fclose(local_fp);
    return NULL;
}

int main(void) {
    
    FILE *fp = fopen("example.txt", "r");
    if (fp == NULL) {
        perror("Failed to open file");
        return 1;
    }
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fclose(fp);

    
    long position = file_size / t; 
    pthread_t threads[t];
    thread_arg_t arg[t];

    for(int i = 0; i < t; i++) {
        arg[i].initial = position * i;
        arg[i].final = (i == t - 1) ? file_size : position * (i + 1);
        pthread_create(&threads[i], NULL, partial_sum, &arg[i]);
    }
    
    for (int i = 0; i < t; i++) {
        pthread_join(threads[i], NULL);
    }
    
    printf("Computed word count: %ld\n", counter);
     
    return 0;
}