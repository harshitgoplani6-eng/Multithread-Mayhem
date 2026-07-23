## Part-1 File: kernel/vm.c (lines 298-326)

uvmcopy() loops through every page from address 0 to sz.
#For each page it does 3 things:

### 1. Allocates a new physical page using kalloc() (line ~310)
   — this is where the child's copy of the page will live

### 2. Copies the bytes using memmove() (line ~311)
   — copies exactly 4096 bytes from the parent's physical page
   into the new page

### 3. Installs a new PTE in the child's page table using mappages() (line ~312)
   — this tells the child's page table "virtual address X
   points to this new physical page"

After uvmcopy returns, parent and child have identical data
but stored in completely separate physical memory locations.

## Part-2 File: kernel/proc.c (lines 272-277)

If uvmcopy returns a negative value, fork() calls freeproc(np)
to delete the half-created child, releases its lock, and
returns -1 to signal failure.

This shows that the caller (fork) is responsible for cleaning
up whatever was set up before the failure. uvmcopy also cleans
up internally — if mappages fails partway through, it calls
uvmunmap to undo all the page mappings it already made, then
returns -1. So both functions follow the same rule: if you
fail halfway, clean up everything you already did before
returning the error.
## Part 3 — Design: sharing instead of copying

Point A — where uvmcopy is called:
Instead of calling uvmcopy(), we would just assign:
np->pagetable = p->pagetable
This makes the child point at the exact same page table
as the parent. No new physical pages are allocated, no
bytes are copied, no new PTEs are installed. The two
struct proc entries share one page table in memory.

Point B — when is it safe to free the page table:
We cannot free the page table when just one thread exits
because the other is still using it. The solution is a
reference counter ; a number stored in memory that both
threads can access. When a thread starts sharing the page
table, increment it. When a thread exits, decrement it.
Only call proc_freepagetable() when the counter reaches
zero — meaning the last thread using it has exited.
This counter would be stored in a small block allocated
with kalloc().

Point C — p->sz and p->pagetable values:
p->pagetable should be the EXACT SAME pointer as the
parent's — not a copy, literally the same address in
memory.
p->sz should start as the same value as the parent's.
But since sz is used for memory safety checks in copyin
and copyout, all sharing threads need to see updates
to sz — which leads directly into Part 4.

## Part 4 — Problem with each thread having its own p->sz

p->sz is a number that says how big this process's memory is.
The kernel uses it to check if a pointer is valid — if the
address is bigger than p->sz it gets rejected.

Suppose Thread A grows memory, its p->sz becomes 12000.
Thread B still has p->sz = 8000. Thread B tries to use
address 10000 which IS mapped and valid, but the kernel
checks Thread B's p->sz and rejects it because 10000 > 8000.

Fix: all threads sharing a page table must also share p->sz
so the fence is always correct for everyone.
