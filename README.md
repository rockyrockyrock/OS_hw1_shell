# Simplified Shell

The simplified shell is constructed using the skeleton of xv6 shell sh.c, consisting of parsing shell commands and implementing their execution.
On top of that, it is a minimalistic implementation designed to demonstrate the basic functionalities of a Unix-like shell.
It supports executing commands, handling I/O redirection ('<', '>'), and piping ('|') between commands.


## Features
- Command Execution: Execute basic commands like 'ls, 'cat', 'rm', 'echo', etc.
- I/O Redirection:
    - Input redirection ('<') to read input from a file.
    - Output redirection ('>') to write output to a file, overwriting the file if it exists.
- Piping ('|'): Allows chaining commands together, where the output of one command serves as the input to the next.
- Error Handling: Basic error handling for system call failures and command parsing errors.


## Testing Process
The shell has been tested using a series of commands that utilize its core functionalities. Below are the tests performed.

### Test1: Output Redirection and File Operations
1. List directory contents and redirect output:
```bash
ls > y
```
This command lists the current directory contents, redirecting the output to a file named 'y'.

2. Sort, remove duplicates, and count unique entries from the file:
```bash
cat y | sort | uniq | wc < y1
```
Read the contents of 'y, sort it, remove duplicates, and count the lines, words, and characters, writing the output to 'y1'.

3. Display the count of unique entries:
```bash
cat y1
```
Display the contents of 'y1', which contains the counts of unique lines, words, and characters from 'y'.

4. Clean up by removing the file 'y1':
```bash
rm y1
```

### Test2: Piping and Sorting
1. List, sort, remove duplicates, and count unique directory contents:
```bash
ls | sort | uniq | wc
```
List the current directory contents, sort them, remove duplicates, and count the unique entries, displaying the counts directly in the shell.
2. Clean up by removing the file 'y':
```bash
rm y
```


## Conclusion
This simplified shell demonstrates basic shell functionalities with a focus on process creation, I/O redirection, and inter-process communication through pipes.
Future improvements could include support for background processes, signal handling, and built-in commands like 'cd'.
For further testing or development, additional commands and scenarios can be added to the testing process to ensure robustness and reliability.
