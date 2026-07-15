#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  getpid();
  uptime();
  sleep(0);

  int count = getsyscount();
  printf("syscall count: %d\n", count);

  exit(0);
}
