// ID: 2065388 Ling Li, ID:2090149 Yi Cheng Lu, ID:2088679 LinLin Zhang
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

// Simplifed xv6 shell.

#define MAXARGS 10

// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd
{
    int type; //  ' ' (exec), | (pipe), '<' or '>' for redirection
};

struct execcmd
{
    int type;            // ' '
    char *argv[MAXARGS]; // arguments to the command to be exec-ed
};

struct redircmd
{
    int type;        // < or >
    struct cmd *cmd; // the command to be run (e.g., an execcmd)
    char *file;      // the input/output file
    int mode;        // the mode to open the file with
    int fd;          // the file descriptor number to use for the file
};

struct pipecmd
{
    int type;          // |
    struct cmd *left;  // left side of pipe
    struct cmd *right; // right side of pipe
};

int fork1(void); // Fork but exits on failure.
struct cmd *parsecmd(char *);

// Execute cmd.  Never returns.
void runcmd(struct cmd *cmd)
{
    int p[2], r; // variables p for pipe, r for result of system calls
    struct execcmd *ecmd;
    struct pipecmd *pcmd;
    struct redircmd *rcmd;
    // exit if given command is null
    if (cmd == 0)
        exit(0);

    switch (cmd->type)
    {
    default:
        fprintf(stderr, "unknown runcmd\n");
        exit(-1);

    case ' ':
        ecmd = (struct execcmd *)cmd;
        if (ecmd->argv[0] == 0)
            exit(0);
        // Your code here ...
        // if execvp failed, it return -1, else it will replace the current process with new one
        if (execvp(ecmd->argv[0], ecmd->argv) < 0)
        {
            perror(ecmd->argv[0]);
            exit(1);
        }
        break;

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
            // duplicate fd to fd number 0(standard input fd),call fails when return negative
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
            // duplicate fd to fd number 1(standard output),call fails when return negative
            if (dup2(fd, 1) < 0)
            {
                fprintf(stderr, "dup2 error\n");
                exit(-1);
            }
        }

        close(fd);
        runcmd(rcmd->cmd);
        break;

    case '|':
        pcmd = (struct pipecmd *)cmd;
        int pipefd[2];
        pid_t pid; // declear process ID data type pid
        // create a pipe represented by fd in the pipefd array
        if (pipe(pipefd) < 0) // pipe() return -1 if it fails
        {
            fprintf(stderr, "pipe error\n");
            exit(-1);
        }

        if ((pid = fork()) < 0) // fork system call returns -1 if it fails
        {
            fprintf(stderr, "fork error\n");
            exit(-1);
        }
        else if (pid == 0) // child process
        {
            close(pipefd[1]);

            // duplicate pipefd[0] to file descriptor 0(stdin), and check if it returns -1
            if (dup2(pipefd[0], 0) < 0)
            {
                fprintf(stderr, "dup2 error\n");
                exit(-1);
            }

            close(pipefd[0]);
            runcmd(pcmd->right);
            exit(0);
        }
        else // parent process
        {
            close(pipefd[0]);

            if (dup2(pipefd[1], 1) < 0)
            {
                fprintf(stderr, "dup2 error\n");
                exit(-1);
            }

            close(pipefd[1]);
            runcmd(pcmd->left);
            wait(NULL); // wait for child process to finish
        }

        break;
    }
    exit(0);
}

// read command from the user and store it in buf
int getcmd(char *buf, int nbuf)
{
    // if stdin is a terminal, print '$ '
    if (isatty(fileno(stdin)))
        fprintf(stdout, "$ ");
    // clear the buffer 'buf' by filling it with null characters(\0)
    memset(buf, 0, nbuf);
    // fgets function read a line of text from standard input, store it in buf
    fgets(buf, nbuf, stdin);
    // check if buf is NULL(\0), if is, indiates the end of file(EOF)
    if (buf[0] == 0) // EOF
        return -1;
    return 0;
}

int main(void)
{
    static char buf[100]; // arrray 'buf' to store input command(max 100 char)
    int fd, r;

    // Read and run input commands.
    while (getcmd(buf, sizeof(buf)) >= 0)
    { // change directory when input command is 'cd'
        if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ')
        {
            // Clumsy but will have to do for now.
            // Chdir has no effect on the parent if run in the child.
            buf[strlen(buf) - 1] = 0; // chop \n
            if (chdir(buf + 3) < 0)
                fprintf(stderr, "cannot cd %s\n", buf + 3);
            continue;
        }
        // fork a child process while command is not 'cd'
        if (fork1() == 0)
            runcmd(parsecmd(buf)); // parse command then run it in child process
        wait(&r);                  // parent process hold and wait for child process finished
    }
    exit(0);
}

int fork1(void)
{
    int pid;

    pid = fork();
    if (pid == -1)
        perror("fork");
    return pid;
}

struct cmd *
execcmd(void)
{
    struct execcmd *cmd;

    cmd = malloc(sizeof(*cmd));   // allocate memory for a structure of struct execcmd
    memset(cmd, 0, sizeof(*cmd)); // initialize the memory block pointed to by 'cmd' with 0
    cmd->type = ' ';              // set type of cmd to ' '
    return (struct cmd *)cmd;
}

struct cmd *
redircmd(struct cmd *subcmd, char *file, int type)
{
    struct redircmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = type; // pass the 'type'argument to 'type' member of cmd
    cmd->cmd = subcmd;
    cmd->file = file;
    // if type is <, set mode to read-only; if type is >, set mode to write-only, creating the file if it doesn't exist
    cmd->mode = (type == '<') ? O_RDONLY : O_WRONLY | O_CREAT | O_TRUNC;
    // set fd number for file, 0 for type == <, 1 for type == >
    cmd->fd = (type == '<') ? 0 : 1;
    return (struct cmd *)cmd;
}

struct cmd *
pipecmd(struct cmd *left, struct cmd *right)
{
    struct pipecmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = '|';
    cmd->left = left;
    cmd->right = right;
    return (struct cmd *)cmd;
}

// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";

// get token from input
int gettoken(char **ps, char *es, char **q, char **eq)
{
    char *s; // pointer used to traverse the input string
    int ret; // used to store the type of token encountered

    s = *ps; // s equals the value pointed to by 'ps'
    // strchr returns a pointer to the location of that character in whitespace if it is found
    while (s < es && strchr(whitespace, *s))
        s++;
    // if 'q' is not NULL, sets '*q' to point to the beginning of the token in the string 's'
    if (q)
        *q = s;
    ret = *s;
    switch (*s)
    {
    case 0:
        break;
    case '|':
    case '<':
        s++;
        break;
    case '>':
        s++;
        break;
    default:
        ret = 'a';
        while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
            s++;
        break;
    }
    if (eq) // if pointer 'eq' is not NULL
        *eq = s;

    while (s < es && strchr(whitespace, *s))
        s++;
    // updates pointer 'ps' to point to current position in input string
    *ps = s;
    return ret;
}

// Checks if token in input string matches specified tokens
int peek(char **ps, char *es, char *toks)
{
    char *s;
    // initializes the pointer s with the value pointed to by ps(current positon)
    s = *ps;
    while (s < es && strchr(whitespace, *s))
        s++;
    // update current position after any leading whitespace characters
    *ps = s;
    return *s && strchr(toks, *s);
}

struct cmd *parseline(char **, char *);
struct cmd *parsepipe(char **, char *);
struct cmd *parseexec(char **, char *);

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char *mkcopy(char *s, char *es) // creating copies of strings
{
    int n = es - s;
    char *c = malloc(n + 1);
    assert(c); // used to verify the validity of the pointer c
               // strncpy() function copies at most n characters from the source string s to the destination string c
    strncpy(c, s, n);
    // adds a null terminator '\0' at the end of copied string c
    c[n] = 0;
    return c;
}

// parse input command string into a command structure
struct cmd *
parsecmd(char *s)
{
    char *es;
    struct cmd *cmd;

    es = s + strlen(s);
    cmd = parseline(&s, es);
    peek(&s, es, "");
    if (s != es)
    {
        fprintf(stderr, "leftovers: %s\n", s);
        exit(-1);
    }
    return cmd;
}
// responsible for parsing a line of input, which may consist
// of one or more commands separated by pipe symbols
struct cmd *
parseline(char **ps, char *es)
{
    struct cmd *cmd;
    cmd = parsepipe(ps, es);
    return cmd;
}
// parses a command sequence separated by pipe symbols ('|')
// from the input string and constructs a pipeline of commands
struct cmd *
parsepipe(char **ps, char *es)
{
    struct cmd *cmd;

    cmd = parseexec(ps, es);
    if (peek(ps, es, "|"))
    {
        gettoken(ps, es, 0, 0);
        cmd = pipecmd(cmd, parsepipe(ps, es));
    }
    return cmd;
}

struct cmd *
parseredirs(struct cmd *cmd, char **ps, char *es)
{
    int tok;
    char *q, *eq;

    while (peek(ps, es, "<>"))
    {
        tok = gettoken(ps, es, 0, 0);
        if (gettoken(ps, es, &q, &eq) != 'a')
        {
            fprintf(stderr, "missing file for redirection\n");
            exit(-1);
        }
        switch (tok)
        {
        case '<':
            cmd = redircmd(cmd, mkcopy(q, eq), '<');
            break;
        case '>':
            cmd = redircmd(cmd, mkcopy(q, eq), '>');
            break;
        }
    }
    return cmd;
}

struct cmd *
parseexec(char **ps, char *es) // returns a pointer to a "struct cmd"
{
    char *q, *eq;
    int tok, argc;
    struct execcmd *cmd;
    struct cmd *ret; // used to return the parsed command

    ret = execcmd();             // create a new 'struct execcmd' object and assigns the result to 'ret'
    cmd = (struct execcmd *)ret; // Casts ret to a pointer of type struct execcmd and assigns it to cmd.

    argc = 0;                       // initializes argument_count = 0
    ret = parseredirs(ret, ps, es); // calls 'parseredirs' to parse any input/output redirections in command
    while (!peek(ps, es, "|"))
    {                                               // while input has no "|"
        if ((tok = gettoken(ps, es, &q, &eq)) == 0) // gettoken to retrieve the next token from the input string
            break;
        if (tok != 'a')
        {
            fprintf(stderr, "syntax error\n");
            exit(-1);
        }
        cmd->argv[argc] = mkcopy(q, eq); // copy the token (address from 'q' to 'eq')
        argc++;
        if (argc >= MAXARGS)
        {
            fprintf(stderr, "too many args\n");
            exit(-1);
        }
        ret = parseredirs(ret, ps, es);
    }
    cmd->argv[argc] = 0; // set the last element of argv to '0',end of the arguments
    return ret;
}