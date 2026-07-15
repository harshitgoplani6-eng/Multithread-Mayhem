# Syscall Trace — getpid()

## Checkpoint 1 : User space stub
File: user/usys.pl (line 41) and user/usys.S (generated)

When your program calls getpid(), it first hits a small piece of assembly
code in usys.S. This code does two things:
1. Puts the number 11 (getpid's syscall number) into register a7
2. Runs the ecall instruction to enter the kernel

## Checkpoint 2 : The ecall instruction
Hardware behavior — no source file for this part.

The moment ecall runs, the CPU hardware automatically does 4 things:
1. Switches from U-mode to S-mode (user → kernel privilege)
2. Saves the current position into sepc (so we know where to return)
3. Sets scause to 8 (meaning "this was a system call")
4. Jumps to the address in stvec (which points to the trampoline)

## Checkpoint 3 : uservec in trampoline.S
File: kernel/trampoline.S (line 22)

This is the first code that runs inside the kernel. It saves all 32 CPU
registers into the trapframe (a saved-state structure, one per process).
Then it switches from the user page table to the kernel page table and
jumps to usertrap().

## Checkpoint 4 : usertrap() deciding this is a syscall
File: kernel/trap.c (line 54)

usertrap() reads scause. If it equals 8, it knows this was a system call.
The condition it checks is:
    if(r_scause() == 8)
If true, it calls syscall(). If scause was something else it would go to
a different handler.

## Checkpoint 5 : syscall() finding the right function
File: kernel/syscall.c (line 141)

syscall() reads the number from trapframe->a7 (which was 11 for getpid).
It looks up slot 11 in the syscalls[] array and calls whatever function
is there. The return value is stored into trapframe->a0 so the user
program gets it back later.

## Checkpoint 6 : sys_getpid() implementation
File: kernel/sysproc.c (line 20)

sys_getpid() is 2 lines long. It gets the current process using myproc()
and returns its pid field directly. That value goes into trapframe->a0.

## Checkpoint 7 : Return path back to user space
File: kernel/trap.c and kernel/trampoline.S (line 100, 149)

usertrapret() switches back to the user page table and sets up the return
address. Then userret() restores all 32 registers from the trapframe back
into the CPU. Finally sret drops back to U-mode and the program resumes
at the instruction after ecall, with the PID value waiting in a0.
