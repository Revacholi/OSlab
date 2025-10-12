# Lab 3: Batch Scheduler Implementation Report

## How the Implementation Ensures Constraints

### Maximum of Three Tasks on the Bus

The implementation maintains a `slot_used` counter that tracks the number of tasks currently using the bus. Before any task can acquire a slot, the `check_bus()` helper function verifies that `slot_used` is less than the bus capacity of three. When a task successfully acquires a slot, the counter increments; when it releases the slot, the counter decrements. This mechanism guarantees that at most three tasks occupy the bus simultaneously, as any additional task will fail the capacity check and must wait.

### Only Tasks of the Same Direction Get on the Bus

Two variables enforce half-duplex behavior: `current_direction` stores the active bus direction, and `bus_in_use` indicates whether the bus is occupied. When a task attempts to acquire a slot, it checks if the bus is in use and whether its direction matches the current bus direction. If the directions differ, the task must wait. When all tasks release their slots and `slot_used` reaches zero, `bus_in_use` is set to false, allowing the next task to establish a new direction. This ensures all concurrent tasks always travel in the same direction.

### Priority Tasks Take Precedence Over Normal Tasks

The implementation uses a counter to track waiting priority tasks in each direction. When a priority task calls `get_slot()`, it increments the appropriate counter before waiting, and decrements it after acquiring a slot. Normal tasks check if any priority tasks are waiting in the opposite direction before proceeding. If priority tasks are waiting, normal tasks are blocked regardless of slot availability. This mechanism ensures priority tasks always take precedence over normal tasks.

## Unfairness in Task Scheduling

### What Makes the Implementation Unfair

The implementation is unfair because priority tasks of the same direction can continuously monopolize the bus, even when priority tasks of the opposite direction are waiting. Additionally, normal tasks can experience indefinite starvation if priority tasks keep arriving.

Consider `batch_scheduler(10, 5, 2, 2)` creating 10 priority SEND tasks, 5 priority RECEIVE tasks, and 2 normal tasks in each direction. The 10 priority SEND tasks will execute consecutively in batches of three, completely ignoring the 5 priority RECEIVE tasks waiting. Only after all SEND priority tasks complete will the bus switch direction. The normal tasks may never execute if priority tasks continue arriving.

This demonstrates two unfairness issues: priority tasks don't respect priority tasks of the opposite direction, and normal tasks can starve indefinitely.

### Modifications for Fairness
The design could be modified for fairness by adding a counter to track consecutive tasks in one direction and forcing a direction switch after a threshold is exceeded. To prevent normal task starvation, an aging mechanism could track each task's waiting time and temporarily boost normal tasks to priority status after a timeout. These changes would require new global variables and extended logic in `check_bus()` to enforce these fairness constraints alongside existing rules.