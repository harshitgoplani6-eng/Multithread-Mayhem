
## kernel/proc.h

Added inside struct proc after char name[16]:

```c
int is_thread;
int tgid;
```



## kernel/proc.c

Added in allocproc() after p->pid = allocpid():

```c
p->is_thread = 0;
p->tgid = p->pid;
```


## kernel/syscall.h

Added at the bottom:

```c
#define SYS_getthreadinfo 28
```



## kernel/syscall.c

Added in the extern declarations block:

```c
extern uint64 sys_getthreadinfo(void);
```

Added in the syscalls[] array:

```c
[SYS_getthreadinfo] sys_getthreadinfo,
```



## kernel/sysproc.c

Added this struct at the top of the file after the existing includes:

```c
struct thread_info {
  int pid;
  int tgid;
  int is_thread;
};
```

Added this function at the bottom of the file:

```c
uint64
sys_getthreadinfo(void)
{
  struct proc *p = myproc();
  struct thread_info info;
  uint64 addr;
  argaddr(0, &addr);
  info.pid = p->pid;
  info.tgid = p->tgid;
  info.is_thread = p->is_thread;
  if(copyout(p->pagetable, addr, (char*)&info, sizeof(info)) < 0)
    return -1;
  return 0;
}
```



## user/user.h

Add this struct and declaration:

```c
struct thread_info {
  int pid;
  int tgid;
  int is_thread;
};
int getthreadinfo(struct thread_info *info);
```



## user/usys.pl

Add this line:

```c
entry("getthreadinfo");
```

own process ID. The test output confirms this — both parent and
child show tgid equal to their own pid, and is_thread is 0 for
both since neither is a thread.
