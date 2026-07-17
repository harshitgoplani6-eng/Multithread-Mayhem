# Week 6 Problem 2 - Priority Scheduler Changes

## What I did

I added priority based scheduling to xv6. The idea is simple - every process gets a priority number between 0 and 63. Lower number means higher priority. The scheduler always picks the process with the lowest priority number to run next instead of just picking the first runnable one it finds.


## kernel/proc.h
I added one field to struct proc to store the priority of each process.

```
int priority;
```


## kernel/proc.c

### Change 1 - allocproc()

When a new process is created I set its default priority to 31 which is the middle of the 0-63 range. This means all processes start equal unless someone changes their priority.

Added after p->pid = allocpid():
```
p->priority = 31;
```

### Change 2 - scheduler()

This is the main change. The original scheduler just picked the first RUNNABLE process it found by going through the proc array in order. I changed it so it goes through ALL processes first and then picks whichever RUNNABLE one has the lowest priority number.

The way it works is I keep track of the best process found so far in a variable called chosen. For each process I check if it is RUNNABLE and if its priority is lower than the current chosen one. If yes I update chosen. At the end I run whichever process ended up in chosen.

Replaced scheduler() with:
```
void
scheduler(void)
{
  struct proc *p;
  struct proc *chosen;
  struct cpu *c = mycpu();

  for(;;){
    intr_on();
    chosen = 0;

    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE){
        if(chosen == 0 || p->priority < chosen->priority){
          if(chosen != 0)
            release(&chosen->lock);
          chosen = p;
        } else {
          release(&p->lock);
        }
      } else {
        release(&p->lock);
      }
    }

    if(chosen != 0){
      chosen->state = RUNNING;
      c->proc = chosen;
      swtch(&c->context, &chosen->context);
      c->proc = 0;
      release(&chosen->lock);
    }
  }
}
```


## kernel/syscall.h

Added two new syscall numbers at the bottom of the file after SYS_hello:

```
#define SYS_set_priority 25
#define SYS_get_priority 26
```



## user/user.h

Added two function declarations so user programs can call these syscalls:

```
int set_priority(int pid, int prio);
int get_priority(int pid);
```


## user/usys.pl

Added two entries so the build system generates the assembly stubs:

```
entry("set_priority");
entry("get_priority");
```



## kernel/syscall.c

Added extern declarations:
```
extern uint64 sys_set_priority(void);
extern uint64 sys_get_priority(void);
```

Added to syscalls array:
```
[SYS_set_priority] sys_set_priority,
[SYS_get_priority] sys_get_priority,
```



## kernel/sysproc.c

Added extern at top so we can access the proc array:
```
extern struct proc proc[];
```

Added two functions at the bottom of the file.

set_priority takes a pid and a new priority value. It searches through all processes to find the one with matching pid, saves the old priority, sets the new one and returns the old value. Returns -1 if no process found.

get_priority takes a pid and just returns the priority of that process. Returns -1 if not found.

```
uint64
sys_set_priority(void)
{
  int pid, prio;
  argint(0, &pid);
  argint(1, &prio);
  struct proc *p;
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      int old = p->priority;
      p->priority = prio;
      release(&p->lock);
      return old;
    }
    release(&p->lock);
  }
  return -1;
}

uint64
sys_get_priority(void)
{
  int pid;
  argint(0, &pid);
  struct proc *p;
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      int prio = p->priority;
      release(&p->lock);
      return prio;
    }
    release(&p->lock);
  }
  return -1;
}
```
