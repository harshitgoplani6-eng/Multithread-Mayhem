#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int balance = 10000;     /* SHARED account balance, in dollars */
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
#define ATTEMPTS_PER_CUSTOMER 100000
#define WITHDRAWAL_AMOUNT     1

void *customer(void *arg) {
    const char *name = (const char *)arg;
    int successful = 0;
    int rejected   = 0;

    for (int i = 0; i < ATTEMPTS_PER_CUSTOMER; i++) {
    pthread_mutex_lock(&lock);
        if (balance >= WITHDRAWAL_AMOUNT) {
            balance -= WITHDRAWAL_AMOUNT;
            successful++; 
        } else {
            rejected++;
        }
    pthread_mutex_unlock(&lock);
        
    }

    printf("Customer %s: %d successful, %d rejected\n",
           name, successful, rejected);
    return NULL;
}

int main(void) {
    pthread_t alice, bob;

    pthread_create(&alice, NULL, customer, (void *)"Alice");
    pthread_create(&bob,   NULL, customer, (void *)"Bob");

    pthread_join(alice, NULL);
    pthread_join(bob,   NULL);

    printf("Final balance: %d\n", balance);

    if (balance < 0) {
        printf("The account went NEGATIVE. The bank is now suing somebody.\n");
    } else if (balance > 0) {
        printf("Some money is left over. That should not be possible if both\n");
        printf("customers were determined to drain it — unless something\n");
        printf("went wrong with the accounting.\n");
    } else {
        printf("Balance is exactly zero — but check the success counts above:\n");
        printf("do successful_alice + successful_bob = 10000? If not, race.\n");
    }

    return 0;
}