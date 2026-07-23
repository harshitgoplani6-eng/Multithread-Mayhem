#include "kernel/types.h"
#include "user/user.h"

int main() {
    struct thread_info info;

    getthreadinfo(&info);
    printf("Before fork\n");
    printf("  PID- %d  TGID- %d  is_thread- %d\n",
           info.pid, info.tgid, info.is_thread);

    int pid = fork();

    if(pid == 0) {
        getthreadinfo(&info);
        printf("Child\n");
        printf("  PID- %d  TGID- %d  is_thread- %d\n",
               info.pid, info.tgid, info.is_thread);
        exit(0);
    } else {
        getthreadinfo(&info);
        printf("Parent\n");
        printf("  PID- %d  TGID- %d  is_thread- %d\n",
               info.pid, info.tgid, info.is_thread);
        wait(0);
    }

    exit(0);
}
