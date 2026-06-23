
# Submissions: Problem 1 Observations

## Part 1: Standard Execution Logs (10 Runs Data)

### 1. hello_thread
* **Expected Output:** 5 lines printing "Hello from thread X" (where X is 0, 1, 2, 3, 4), followed by "All threads done."
* **Actual Output Range:** The content is always correct, but the order of lines changes constantly across the 10 runs.
  * *Run 1:* 0, 1, 3, 2, 4
  * *Run 2:* 1, 0, 2, 4, 3
  * *Run 5:* 3, 2, 4, 0, 1
* **Frequency of Correctness:** 100% correct in content. The thread execution ordering is non-deterministic due to OS scheduling, but no data is corrupted because there is no shared mutable state.

### 2. race_counter (at default ITERS = 10,000,000)
* **Expected Output:** `Final count: 20000000`
* **Actual Output Range:** `10,134,520` to `18,567,511`
* **Frequency of Correctness:** 0% correct. It never reached 20,000,000 a single time out of 10 runs. Every run produced a wildly different, lower number.

### 3. parallel_sum
* **Expected Output:**
  text
  Computed sum: 10000000
  Expected:     10000000

* **Actual Output Range:** Computed sum ranged between `3,120,450` and `8,604,561`. The value `Off by X` fluctuated between 4 million and 9 million.
* **Frequency of Correctness:** 0% correct. Due to 4 parallel threads hammering the shared global `total` variable via non-atomic `LOAD-ADD-STORE` instructions, millions of updates were dropped on every single run.

### 4. bank_chaos

* **Expected Output:** `Final balance: 0` (with Alice and Bob successfully making a combined 10,000 withdrawals, and rejecting the remaining 190,000 attempts).
* **Actual Output Range:**
* Final balance was mostly 0 but sometimes -1.
* Total successful counts across both customers summed up to values like `11,254` instead of `10,000`.


* **Frequency of Correctness:** It was mostly correct in my case  balance was mostly 0 and total successful count went up in like 5 times in 10 attempts.



## Part 2: race_counter Specific Variations

### Variation 1: ITERS = 1000 (Very Small)

* **Observed Behavior:** The bug completely disappears. The program outputs exactly `Final count: 2000` on all 10 runs.
* **Why it does not show up:**
1. *Loop Execution Time vs. OS Timeslice:* Running a loop 1,000 times takes less than a fraction of a microsecond. The operating system's scheduler leaves a thread on a CPU core for a few milliseconds before swapping it out (a context switch). Thread 1 completes all 1,000 loops and exits before the OS can pause it mid-instruction, or before Thread 2 even fully finishes booting up.
2. *Minimal Overlap Window:* The mathematical probability of their execution steps colliding in RAM is virtually zero because the window of time they spend inside the critical section is too short.



### Variation 2: ITERS = 100,000,000 

* **Observed Behavior:** The bug shows up with absolute reliability. The final counts drop even lower relative to the target maximum (often being close to 100,000,000).
* **Why it happens:** The threads must grind away at memory for a significantly longer duration. This guarantees that the OS scheduler will forcefully pause and swap the threads hundreds of times mid-loop, ensuring maximum interleaving and hardware-level instruction collisions on the CPU cache bus.

### Variation 3: Compiling with -O2 Optimization

* **Command:** `gcc -Wall -O2 -pthread race_counter.c -o race_counter`
* **Observed Behavior:** The output suddenly becomes 100% correct (`20,000,000`) or jumps up drastically to an almost correct number, and the program runs nearly instantly.
* **Why the behavior changes (Compiler Optimization Mechanics):**
* When you don't pass an optimization flag (`-O0`), the compiler strictly translates your C code line-by-line: it reads and writes to RAM memory (`total`) on every single iteration of the loop.
* When you turn on `-O2`, the compiler's optimization unit analyzes the loop logic. It realizes that updating a global memory variable millions of times inside a loop is highly inefficient. To optimize this, it changes the machine instructions :
1. It loads the global `total` value once into a fast, private CPU hardware before the loop starts.
2. It runs the loop 10,000,000 times purely inside that private hardware. No other thread can see or corrupt this register.
3. After the loop completely finishes, it executes a single `STORE` instruction to copy the final register value back into the global RAM variable.


* Because memory updates are no longer interleaved millions of times inside RAM, the race condition disappears or changes completely.
