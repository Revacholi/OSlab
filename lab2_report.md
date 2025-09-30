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
An operating system kernel is a complex, multi-threaded program, in which
synchronizing multiple threads can be difficult. That is why we want you to explain explicitly
how you chose to synchronize this particular type of activity

## Time and space complexity 
This is where you state the time and space complexity of your
implementation. Note that you can do an informal complexity analysis (formal language or
proofs are unnecessary).
