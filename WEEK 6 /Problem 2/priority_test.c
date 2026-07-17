#include "kernel/types.h"
#include "user/user.h"

int main(void) {
    int pid1, pid2;
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);

    printf("=== Priority Scheduler Test ===\n");

    pid1 = fork();
    if(pid1 == 0){
        close(pipe1[0]);
        set_priority(getpid(), 5);
        volatile uint64 count = 0;
        int deadline = uptime() + 50;
        while(uptime() < deadline){
            count++;
            __sync_synchronize();
        }
        write(pipe1[1], (char*)&count, sizeof(count));
        close(pipe1[1]);
        exit(0);
    }

    pid2 = fork();
    if(pid2 == 0){
        close(pipe2[0]);
        set_priority(getpid(), 50);
        volatile uint64 count = 0;
        int deadline = uptime() + 50;
        while(uptime() < deadline){
            count++;
            __sync_synchronize();
        }
        write(pipe2[1], (char*)&count, sizeof(count));
        close(pipe2[1]);
        exit(0);
    }

    close(pipe1[1]);
    close(pipe2[1]);

    printf("Parent: set child %d to priority 5, child %d to priority 50\n",
           pid1, pid2);

    uint64 count1 = 0, count2 = 0;
    read(pipe1[0], (char*)&count1, sizeof(count1));
    read(pipe2[0], (char*)&count2, sizeof(count2));

    printf("Child %d (prio 5):  %lu iterations\n", pid1, count1);
    printf("Child %d (prio 50): %lu iterations\n", pid2, count2);

    if(count2 > 0)
        printf("Child %d ran about %lux more than child %d\n",
               pid1, count1/count2, pid2);

    wait(0);
    wait(0);
    exit(0);
}
