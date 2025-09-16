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
The automated tests were extremely useful for several reasons. It covered all major specifications. When tests failed, the error messages provided clear guidance about what was expected versus what was received, making debugging much easier. Tests like `test_CTRL_C_with_fg_and_bg` helped identify complex scenarios we didn't have considered during manual testing.   
     
     
# Missing test cases
Tests combining pipes with both input and output redirection simultaneously (e.g., `cat < input.txt | grep pattern > output.txt`) would verify more complex scenarios. Also, test built-in Command Edge Cases like `cd` with no arguments or with invalid paths and proper error handling. Last, maybe it can verify all file descriptors are properly closed after complex pipe chains or background tasks. 
