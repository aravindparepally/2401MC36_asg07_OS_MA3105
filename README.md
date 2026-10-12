# Assignment 7: Advanced OS Concepts in xv6

**Name:** Parepally Aravind  
**Roll Number:** 2401MC36  
**Course:** Operating Systems (MA3105)

## Overview
This assignment implements advanced operating system concepts on the xv6-riscv kernel, including user-level threads, kernel-level synchronization primitives, deadlock detection, and priority-based preemptive scheduling.

## Q1: User-Level Threads
**Implementation:**
- Added `sys_clone()` and `sys_join()` system calls in `kernel/sysproc.c`.
- Threads share the parent process's page table (`np->pagetable = myproc()->pagetable`), enabling shared memory concurrency.
- **TRAPFRAME Remapping:** Since threads share a page table, the `TRAPFRAME` virtual address can only map to one physical page at a time. To prevent kernel panics, the scheduler in `kernel/proc.c` dynamically unmaps and remaps the incoming process's `TRAPFRAME` on every context switch.
- Threads are given a default priority of 10.

**Result:** 
Successfully runs 3 concurrent threads. The shared memory counter reaches exactly `300000`, proving correct memory sharing and thread execution.

## Q2: Synchronization using Semaphores
**Implementation:**
- Implemented counting semaphores (`sys_sem_init`, `sys_sem_wait`, `sys_sem_signal`) in the kernel.
- Semaphores use a global array and a spinlock for atomic operations.
- **Yield-based Waiting:** To prevent deadlocks on a single-CPU system (where `sleep()` would block the only CPU), `sem_wait` uses a `yield()` loop. It releases the semaphore lock, yields the CPU, and re-acquires the lock to check the condition, ensuring other threads get a chance to call `sem_signal`.

**Result:**
The synchronized counter reaches exactly `300000` (no lost updates due to race conditions). The Producer-Consumer test runs flawlessly, demonstrating correct blocking and signaling.

## Q3: Deadlock Detection
**Implementation:**
- Implemented a user-space simulation of deadlock detection in `user/deadlock.c`.
- Uses the Banker's algorithm logic to check for a safe sequence based on the Resource Allocation Graph (RAG) matrices (Allocation, Max, Available).
- Tests two scenarios: a safe state and a deadlocked state.

**Result:**
- Test 1 (Safe State): Correctly identifies that no deadlock exists.
- Test 2 (Deadlocked State): Correctly identifies the specific processes (P0, P1, P2, P3, P4) involved in the deadlock.

## Q4: Priority-Based Preemptive Scheduling
**Implementation:**
- Replaced the default Round-Robin scheduler in `kernel/proc.c` with a priority-based scheduler.
- Priority range: 1 (highest) to 20 (lowest).
- **Aging Mechanism:** Processes that wait too long (`wait_ticks > 30`) have their priority boosted (`priority--`) to prevent starvation.
- **Fair Tie-Breaking:** When multiple processes have the same priority, the scheduler selects the one that has waited the longest (`wait_ticks`), ensuring fair CPU distribution.
- Added `sys_set_priority` and `sys_get_priority` system calls to allow user processes to modify their priority dynamically.

**Result:**
High-priority processes finish before low-priority ones. The aging mechanism successfully prevents starvation for lower-priority processes over long execution times.

## How to Build and Run

1. Clone the repository and navigate to the directory:
   ```bash
   cd 2401MC36_asg07_OS_MA3105
