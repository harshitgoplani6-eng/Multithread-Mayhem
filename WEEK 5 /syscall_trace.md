# Syscall Trace — getpid()

## Checkpoint 1 — User space stub
File: user/usys.pl (line 41) and user/usys.S (generated)
usys.pl contains entry("getpid") which tells the build system to generate
a stub. The generated stub in usys.S does two things before trapping:
loads the syscall number (SYS_getpid = 11) into register a7, then
executes ecall to trap into the kernel.

## Checkpoint 2 — The ecall instruction
Hardware behavior — no single source line.
When ecall executes four things happen atomically: (1) privilege switches
from U-mode to S-mode, (2) current PC is saved into sepc so we can return
later, (3) scause is set to 8 meaning "environment call from U-mode",
(4) CPU jumps to the address in stvec which always points to the
trampoline.

## Checkpoint 3 — uservec in trampoline.S
File: kernel/trampoline.S (line 22)
uservec is the first code that runs after ecall. It saves all 32 user
registers into the process's trapframe — a pre-allocated block of memory,
one per process. It temporarily uses sscratch to save a0 first (line 32),
then saves everything else into p->trapframe. After saving all registers
it switches the page table from user to kernel and jumps to usertrap().

## Checkpoint 4 — usertrap() deciding this is a syscall
File: kernel/trap.c (line 54)
usertrap() checks r_scause() == 8 to determine if this trap was a system
call. If true, it calls syscall() at line 68. If scause had been something
else (like 13 or 15 for page faults) it would route to a different handler
entirely.

## Checkpoint 5 — syscall() finding the right function
File: kernel/syscall.c (line 141)
syscall() reads the syscall number from p->trapframe->a7 (line 141) — the
number the stub loaded before ecall. It checks if the number is valid and
looks it up in the syscalls[] array (line 108) — an array of function
pointers indexed by syscall number. The result is stored back into
p->trapframe->a0 (line 145) so it is available as the return value when
the process resumes.

## Checkpoint 6 — sys_getpid() implementation
File: kernel/sysproc.c (line 20)
sys_getpid() is only 2 lines long. It calls myproc() to get the current
process's struct proc, then returns p->pid directly. The return value gets
stored in p->trapframe->a0 by syscall() back in kernel/syscall.c line 145
— that is the register the user program reads as the function return value.

## Checkpoint 7 — Return path back to user space
Files: kernel/trap.c (line 121) and kernel/trampoline.S (line 100)
The return path involves two functions. First usertrapret() in trap.c sets
up registers for the return journey — restoring the user page table and
preparing sepc to point to the instruction after ecall. Then it jumps to
userret in trampoline.S (line 101) which restores all 32 user registers
from the trapframe back into the CPU. Finally sret at line 149 drops back
to U-mode and resumes the user program exactly where it left off.
