# Lab 2: re-implement timer_sleep() Report

## Data Structures 

Modified `struct thread`, added `uint64_t wake_time` to the thread structure:
 
**Purpose**: Stores the absolute tick value at which a sleeping thread should be woken up. 


## Algorithms 
### Sleep Mechanism  
When a thread needs to sleep, it calculates an absolute wake up time (current ticks plus sleep duration) and stores it in thread structure. The thread will block itself then, yielding the CPU to other threads. 

### Wake-up Mechanism

The timer interrupt handler is responsible for managing wake up. On each tick, it scans all threads in the system to identify those that are both blocked and have reached their designated wake time. For efficiency, the scanning logic is encapsulated into a helper function passed to `thread_foreach()`, which handles the iteration safely.

The helper function applies a three-part test to each thread: is it blocked, does it have a pending wake time, and has that time arrived? Only threads passing all three conditions are unblocked. The wake time is reset to prevent repeated wake-up attempts.

In this design, blocked threads are excluded from CPU time consumption. The timer interrupt provides a natural checkpoints. Absolute tick values and thread structure are helpful to simplifies the logic.


## Synchronization 
The only real synchronization work we did on this lab was in the timer_sleep function. We check to make sure that interrupts are enabled before disabling interrupts, setting the threads wake time, blocking the thread and then reenabling interrupts. During this transition to create an atomic operation encompassing wake-time assignment and thread blocking. Without this protection, a timer interrupt could have an inconsistent state when the wake time is set but the thread hasn't blocked yet, or attempt to unblock a thread that isn't fully blocked. 


## Time and space complexity 
Adding the wake time to the thread structure and making sure it's set while initalizing a thread both take O(N) time as they are just a single operation each run once per thread. Storing a variable for each thread is also O(N) space complexity.

Timer_sleep has time complexity O(1) for each thread, meaning O(N) in total if we assume we run it on the order of N threads. Here we similarily save the intr_level lv giving us O(1) for each thread and O(N) in the multi-thread case.

The timer_interrupt function loops through all threads using the thread_foreach function. This calls check_blocked_thread on all threads. The check_blocked_thread function takes O(1) time and so the entire function takes O(N) time.
