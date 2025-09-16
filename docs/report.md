# Report lab 1 operating Systems
In this report we will outline our process for completing lab 1 of the course DIT401 - Operating Systems. We will discuss the general path we took to completing the coruse, as well as some troubles we ran into and a few things we will take away from the lab. Over all most things went very smoothly and we enjoyed doing the lab as well as look forward to the future labs!
## Completion of specifications
We completed most of the specifications without much issue. For the most part we didn't run into problems with the automated tester either, when we got the solution working with manual testing it generally worked for the automated tester as well. 
One notable excetion to this was **Ctrl-C Handling** where we didn't realize we left some child processes behind. We will get more into this under the rubric **Ctrl-C issues**.

### Order
We split up the completion of the lab, each taking whatever specification we felt like implementing and then presented it to our colleagues whenever we were done or ran into problems.
For the sake of simplicity, we wrote the solutions basically in the same order as they were written in the README.md.
As an exceptio, we wrote 3 - Background execution last. This was for no particular reason but it didn't cause any problems for us.

## Challenges

### Pipe issues
The biggest problem we had was with implementing pipes. We started doing it recursively which proved difficult. At one point we got the pipes working, but in reverse! E.g. instead of 
`cat foo | grep bar | wc -l` counting the number of occurences of `bar` in `foo` it would simply run `cat foo`. To do the piping as expected one had to write
`wc -l | grep bar | cat foo`. 

In the end, we totally rewrote the code with a better from the start.

### Zombie issues

### Ctrl-C issues

## Feedback on the labs
This was a fun lab that gave us some insights into the inner workings of the unix shell. It is fascinating to learn how much forking happens, even while just running regular commands and especially when using pipes. We also found the specifications were written in a helpful way giving enough details and hints to really help us get started without giving so much information as to totally give the solution for how to implement each specification away for free.


# Automated tests
The automated tests were extremely useful for several reasons. It covered all major specifications. When tests failed, the error messages provided clear guidance about what was expected versus what was received, making debugging much easier. Tests like `test_CTRL_C_with_fg_and_bg` helped identify complex scenarios we didn't have considered during manual testing.   
     
     
# Missing test cases
Tests combining pipes with both input and output redirection simultaneously (e.g., `cat < input.txt | grep pattern > output.txt`) would verify more complex scenarios. Also, test built-in Command Edge Cases like `cd` with no arguments or with invalid paths and proper error handling. Last, maybe it can verify all file descriptors are properly closed after complex pipe chains or background tasks. 