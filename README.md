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


## Command Execution
Below outlines the stpes involved in executing commands within this simple shell implementation.
In particular, we will be focusing on the commands of excuting programs, redirecting input/output, and creating pipelines between commands.
The shell processes commands based on their type, indicated by a leading character (' ', '>', '<', '|').

### Executing Commands (' ')
When a command prefixed with a space (' '), the shell attemps to execute the command specified by the user.

- Steps:
    1. Check if the first argument `(argv[0])` is `NULL`. If so, exit the shell.
    2. Use `execvp` to execute the command. `execvp` searches for the executable in the system's PATH.
    3. If `execvp` fails, print an error message and exit with status `1`.

```bash
case ' ':
        ecmd = (struct execcmd *)cmd;
        if (ecmd->argv[0] == 0)
            exit(0);
        if (execvp(ecmd->argv[0], ecmd->argv) < 0)
        {
            perror(ecmd->argv[0]);
            exit(1);
        }
        break;
```

### Redirecting Input/Output ('<', '>'):
For commands invovling redirection ('<' for input, '>' for output), the shell redirects the file descriptors accordingly.

- Steps:
    1. Open the specified file using `open`. The mode depends on the redirection type.
    2. If the file cannot be opened, print an error message and exit with status `-1`.
    3. Use `dup2` to duplicate the file descriptor to standard input (`stdin`) or standard output (`stdout`).
    4. Close the original file descriptor.
    5. Execute the command following the redirection.

```bash
case '>':
    case '<':
        rcmd = (struct redircmd *)cmd;
        int fd;

        if (rcmd->type == '<')
        {
            fd = open(rcmd->file, rcmd->mode);
            if (fd < 0)
            {
                fprintf(stderr, "cannot open %s\n", rcmd->file);
                exit(-1);
            }
            if (dup2(fd, 0) < 0)
            {
                fprintf(stderr, "dup2 error\n");
                exit(-1);
            }
        }
        else
        { // rcmd->type == '>'
            fd = open(rcmd->file, rcmd->mode, 0666);
            if (fd < 0)
            {
                fprintf(stderr, "cannot create %s\n", rcmd->file);
                exit(-1);
            }

            if (dup2(fd, 1) < 0)
            {
                fprintf(stderr, "dup2 error\n");
                exit(-1);
            }
        }

        close(fd);
        runcmd(rcmd->cmd);
        break;
```

### Creating Pipelines ('|')
Pipelines allow the output of one command to be used as the input for another.

- Steps:
    1. Create a pipe using `pipe`.
    2. Fork the process using `fork`.
    3. In the child process:
        - Close the write-end of the pipe.
        - Duplicate the read-end of the pipe to `stdin`.
        - Close the read-end of the pipe.
        - Recursively execute the command on the right side of the pipe.
    4. In the parent process:
        - Close the read-end of the pipe.
        - Duplicate the write-end of the pipe to `stdout`.
        - Close the write-end of the pipe.
        - Recursively execute the command on the left side of the pipe.
        - Wait for the child process to terminate.

```bash
case '|':
        pcmd = (struct pipecmd *)cmd;
        int pipefd[2];
        pid_t pid;

        if (pipe(pipefd) < 0)
        {
            fprintf(stderr, "pipe error\n");
            exit(-1);
        }

        if ((pid = fork()) < 0)
        {
            fprintf(stderr, "fork error\n");
            exit(-1);
        }
        else if (pid == 0)
        {
            close(pipefd[1]);

            if (dup2(pipefd[0], 0) < 0)
            {
                fprintf(stderr, "dup2 error\n");
                exit(-1);
            }

            close(pipefd[0]);
            runcmd(pcmd->right);
            exit(0);
        }
        else
        {
            close(pipefd[0]);

            if (dup2(pipefd[1], 1) < 0)
            {
                fprintf(stderr, "dup2 error\n");
                exit(-1);
            }

            close(pipefd[1]);
            runcmd(pcmd->left);
            wait(NULL);
        }

        break;
```

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
