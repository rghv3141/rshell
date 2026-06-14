# rshell

A simple Unix shell written in C.

## Features 

- Execute external commands using `fork()` and `execvp()`
- Single pipe support (`cmd1 | cmd2`)
- Output redirection (`>`)
- Input redirection (`<`)
- Background execution (`&`)
- SIGCHLD handling to reap background processes
- Built-in `exit` command  
## Compilation

```bash
gcc -Wall -Wextra rshell.c -o rshell
```

## Examples

```bash
rshell> ls
rshell> ls | grep .c
rshell> cat file.txt | wc -l
rshell> echo hello > hello.txt
rshell> wc -l < hello.txt
rshell> sleep 5 &
rshell> exit
