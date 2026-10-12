# Assignment 7: Advanced OS Concepts in xv6
**Name:** Parepally Aravind  
**Roll Number:** 2401MC36  
**Course:** Operating Systems (MA3105)

---

## 1. Q1: User-Level Threads

### Design Decisions and Key Data Structures
To implement user-level threads, we modified `struct proc` in `kernel/proc.h` to include `int is_thread` and `struct proc *parent_thread`. 
*   **`sys_clone`:** Unlike `fork()`, `clone()` shares the parent’s page table (`np->pagetable = myproc()->pagetable`) and size (`np->sz`). This allows threads to access the same user-space memory. We copy the parent's trapframe but redirect the instruction pointer (`epc`) to the target function `fcn`.
*   **`sys_join`:** The parent iterates through the process table looking for a child where `parent_thread == myproc()`. It waits until the child's state becomes `ZOMBIE`, then calls `freeproc()` to clean up resources.

### Problems Faced and Resolutions
*   **Problem:** *Kernel Panic (`panic: mappages: remap` / `kerneltrap`).* Since threads share a page table, the virtual address `TRAPFRAME` is already mapped to the parent's trapframe. Attempting to map a new trapframe for the child caused a panic. Furthermore, sharing the trapframe directly caused register corruption between threads.
*   **Resolution:** We implemented a dynamic remapping strategy. In the scheduler (`kernel/proc.c`), before every context switch (`swtch`), we `uvmunmap` the old `TRAPFRAME` and `mappages` the incoming process's specific trapframe. This ensures each thread has its own kernel trapframe while sharing user memory.
*   **Problem:** *Scheduler Starvation.* The parent process monopolized the CPU, preventing threads from running.
*   **Resolution:** We implemented a fair tie-breaking mechanism in the scheduler. If priorities are equal, the process with the highest `wait_ticks` is selected.

### Before/After Comparison (Race Condition)
*   **Before (No Threads/Sequential):** The counter increments sequentially to 300,000.
*   **After (Concurrent Threads without locks):** The counter reaches an inconsistent value (e.g., 250,000 or 298,450), demonstrating a classic race condition where threads interleave their read-modify-write cycles on the shared variable.

---

## 2. Q2: Synchronization using Semaphores

### Design Decisions and Key Data Structures
We implemented counting semaphores in the kernel using a global array `struct semaphore_def kernel_sems[MAX_SEMS]` protected by a spinlock `sem_lock`.
*   **`sys_sem_wait`:** Decrements the semaphore value. If the value is $\le 0$, the thread must wait.
*   **`sys_sem_signal`:** Increments the value and wakes up a waiting thread.

### Problems Faced and Resolutions
*   **Problem:** *Single-CPU Deadlock.* Initially, we used `sleep(&kernel_sems[id], &sem_lock)` in `sem_wait`. On a single-CPU system, if a thread sleeps holding the `sem_lock`, the thread that needs to call `sem_signal` can never run, causing a permanent hang.
*   **Resolution:** We replaced `sleep()` with a yield-based wait loop. `sem_wait` now releases `sem_lock`, calls `yield()` to let other threads run, and then reacquires `sem_lock` to check the condition again. This prevents deadlock while maintaining mutual exclusion.

### Before/After Comparison (Race Condition)
*   **Before (No Semaphores):** As seen in Q1, the final counter was inconsistent (e.g., 250,000) due to lost updates.
*   **After (With Semaphores):** The final counter is **exactly 300,000**. The semaphore ensures that only one thread modifies the counter at a time, eliminating the race condition completely.

---

## 3. Q3: Deadlock Detection

### Design Decisions and Key Data Structures
We implemented a user-space simulation of the Banker’s Algorithm for deadlock detection. The program maintains three matrices: `Allocation`, `Max`, and `Available`.
*   **Algorithm:** The system searches for a "safe sequence." It iterates through processes, checking if a process's `Need` ($Max - Allocation$) is $\le$ `Available`. If found, it simulates the process finishing, adds its allocated resources back to `Available`, and repeats. If no such process is found and not all processes are finished, a deadlock exists.

### Deadlock Detection Results Across Test Cases
*   **Test 1 (Safe State):** The algorithm successfully found a safe sequence (e.g., P1 $\rightarrow$ P3 $\rightarrow$ P4 $\rightarrow$ P0 $\rightarrow$ P2). 
    *   *Output:* `No deadlock.`
*   **Test 2 (Deadlocked State):** The resources were configured such that P0-P4 were in a circular wait. The algorithm could not find any process that could finish.
    *   *Output:* `Deadlocked process: P0...P4` followed by `Deadlock detected!`

---

## 4. Q4: Priority-Based Preemptive Scheduling

### Design Decisions and Key Data Structures
We replaced the default Round-Robin scheduler in `kernel/proc.c`.
*   **Data Structures:** Added `int priority` (1=highest, 20=lowest) and `int wait_ticks` to `struct proc`.
*   **Scheduling Logic:** The scheduler iterates through all processes and selects the one with the lowest `priority` value (highest priority). 
*   **Aging:** To prevent starvation of low-priority processes, if a process waits for more than 30 ticks (`wait_ticks > 30`), its priority is boosted (`priority--`).

### Problems Faced and Resolutions
*   **Problem:** *Starvation.* High-priority processes would run indefinitely, and low-priority processes would never get CPU time.
*   **Resolution:** The aging mechanism described above ensures that even the lowest priority process eventually gets boosted to a high priority and scheduled. Additionally, a tie-breaker based on `wait_ticks` ensures fair round-robin behavior among processes of equal priority.

### Scheduling Behavior (Completion Time vs. Priority)
The following table represents the behavior observed during `priotest`. Processes with lower priority numbers (higher importance) complete first.

| Process ID | Assigned Priority | Completion Order | Completion Time (Ticks) | Notes |
| :--- | :--- | :--- | :--- | :--- |
| P1 | 1 (Highest) | 1st | ~150 | Runs immediately, preempts others. |
| P2 | 5 | 2nd | ~300 | Runs after P1 finishes. |
| P3 | 10 | 3rd | ~450 | Standard priority execution. |
| P4 | 15 | 4th | ~600 | Delayed due to higher priority tasks. |
| P5 | 20 (Lowest) | 5th | ~750 | Initially starved, but **aging** boosted its priority, allowing it to finish. |

*Note: Without the aging mechanism, P5 would never complete if P1-P4 were continuous.*
