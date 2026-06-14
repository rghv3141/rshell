# rshell

A simple Unix shell written in C.

## Features 

- Execute external commands using `fork()` and `execvp()`
- Single pipe support (`cmd1 | cmd2`)
- Output redirection (`>`)
- Input redirection (`<`)
- Background execution (`&`)
- SIGCHLD handling to reap background processes

## Compilation

```bash
gcc -Wall -Wextra rshell.c -o rshell

