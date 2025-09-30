# Lab 2: re-implement timer_sleep() Report

## Data Structures 

Modified `struct thread`, added `uint64_t wake_time` to the thread structure:
 
**Purpose**: Stores the absolute tick value at which a sleeping thread should be woken up. 


## Algorithms 
This is where you tell us how your code works. We might not be able to easily
figure it out from the code, because many creative solutions exist for most OS problems. Help
us out a little. Your report should be at a level below the high level description of requirements
given in the assignment. We have read the assignment too, so it is unnecessary to repeat or
rephrase what is stated there. On the other hand, your description should be at a level above
the low level of the code itself. Don’t give a line-by-line run-down of what your code does.
Instead, use your report to explain how your code works to implement the requirements.

## Synchronization 
The only real synchronization work we did on this lab was in the timer_sleep function. We check to make sure that interrupts are enabled before disabling interrupts, setting the threads wake time, blocking the thread and then reenabling interrupts.

## Time and space complexity 
Adding the wake time to the thread structure and making sure it's set while initalizing a thread both take O(N) time as they are just a single operation each run once per thread.
Storing a variable for each thread is also O(N) space complexity.

Timer_sleep has time complexity O(1) for each thread, meaning O(N) in total if we assume we run it on the order of N threads.
Here we similarily save the intr_level lv giving us O(1) for each thread and O(N) in the multi-thread case.

The timer_interrupt function loops through all threads using the thread_foreach function. This calls check_blocked_thread on all threads. The check_blocked_thread function takes O(1) time and so the entire function takes O(N) time.
