#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define ITERS 10000000

/* A SHARED global counter. Both threads will modify this. */
long counter = 0;

void *increment(void *arg) {
    (void)arg;         /* unused — pthread signature requires it */
    for (long i = 0; i < ITERS; i++) {
         int a=counter;/* three machine instructions in disguise */
        a=a+1;
        usleep(2);
        counter=a;
            
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    pthread_create(&t1, NULL, increment, NULL);
    pthread_create(&t2, NULL, increment, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    long expected = 2L * ITERS;
    printf("Final counter:  %ld\n", counter);
    printf("Expected:       %ld\n", expected);
    printf("Difference:     %ld\n", expected - counter);

    if (counter == expected) {
        printf("Result: CORRECT (but try running it again, you might get unlucky)\n");
    } else {
        printf("Result: WRONG by %ld — race condition observed.\n", expected - counter);
    }

    return 0;
}
