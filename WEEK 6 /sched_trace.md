# Scheduler Trace — Week 6 Problem 1

## Checkpoint 1 — Process Creation (allocproc)
File: kernel/proc.c (around line 100)
allocproc() loops through the proc[] array looking for a slot with
state == UNUSED. When it finds one it sets state to USED, assigns a
new PID, allocates a trapframe, sets up the page table, and initializes
the context so the process will start executing at forkret(). After
allocproc() returns the process is in the USED state — not yet RUNNABLE.

## Checkpoint 2 — Becoming RUNNABLE
File: kernel/proc.c (around line 280, inside fork())
fork() calls allocproc() to get a new slot, copies the parent's memory
and trapframe into the child, then sets p->state = RUNNABLE while holding
p->lock. The lock must be held during this transition because another CPU
running the scheduler could be scanning the proc[] array at the same
moment — without the lock it might see a half-initialized process and try
to run it, causing corruption.

## Checkpoint 3 — Scheduler picks it up
File: kernel/proc.c (around line 450, scheduler())
scheduler() runs an infinite loop on each CPU. It scans the entire proc[]
array from proc[0] to proc[NPROC]. For each process it acquires p->lock,
checks if p->state == RUNNABLE, and if so sets state to RUNNING, records
the process in c->proc, and calls swtch() to context switch into it.
After swtch() eventually returns (when the process gives up the CPU),
it sets c->proc = 0 and releases p->lock before continuing the loop.

## Checkpoint 4 — The context switch
File: kernel/swtch.S
swtch() saves 14 callee-saved registers (ra, sp, s0-s11) from the current
context into the old context struct, then loads those same 14 registers
from the new context struct. The ret instruction at the end jumps to
wherever ra now points — which is the saved return address of the new
process. This is how swtch() ends up executing in a completely different
place than where it was called from.

## Checkpoint 5 — While the process runs
File: kernel/proc.c (scheduler())
While the process runs in user space, the scheduler is paused inside
swtch() — specifically it is sitting at the ret instruction waiting for
control to come back. c->context holds the scheduler's saved registers
for this CPU. The scheduler does nothing and uses no CPU time while a
process is executing — it only resumes when the process calls sched()
which switches back to c->context.

## Checkpoint 6 — Yielding the CPU
File: kernel/trap.c (usertrap(), around line 70) and kernel/proc.c (yield(), sched())
When a timer interrupt fires, usertrap() sees scause is a timer interrupt
and calls yield(). yield() acquires p->lock, sets p->state = RUNNABLE
(the process is ready to run again, just not right now), then calls
sched(). sched() calls swtch(&p->context, &c->context) which saves the
process's registers and restores the scheduler's registers. Control
returns to the scheduler loop right after where swtch() was called in
scheduler().

## Checkpoint 7 — Scheduler picks another process
File: kernel/proc.c (scheduler(), around line 470)
After swtch() returns in scheduler(), it sets c->proc = 0 (no process
running on this CPU right now) and releases p->lock. The loop then
continues scanning from where it left off — checking the next entries
in proc[] for a RUNNABLE process. When it finds one it acquires that
process's lock, sets it to RUNNING, and calls swtch() again to hand
over the CPU. This continues forever — the scheduler never stops.
