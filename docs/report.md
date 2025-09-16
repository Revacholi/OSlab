# Report lab 1 operating systems

## Completion of specifications

We split up the completion of the lab, each taking whatever specification we felt like implementing and then presented it to our colleagues whenever we were done or ran into problems.

### Order
We wrote the solutions basically in the same order as they were written in the README.md.

## Challenges

### Pipe issues
The biggest problem we had was with implementing pipes. We started doing it recursively which proved difficult. At one point we got the pipes working, but in reverse! E.g. instead of 
`cat foo | grep bar | wc -l` counting the number of occurences of `bar` in `foo` it would simply run `cat foo`. To do the piping as expected one had to write
`wc -l | grep bar | cat foo`. 

In the end, we totally rewrote the code with a better from the start.

### Zombie issues

## Feedback on the course

# Automated tests

# Missing test cases
