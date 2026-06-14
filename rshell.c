#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


char **rshell_split_line(char *);
int rshell_launch(char **);
int rshell_execute(char **);
char *rshell_read_line();
int rshell_pipe(char **, int);
int o_redirection(char **, int);
int rshell_background(char **, int);
void rshell_loop(void) 
{
	
	char *line;
	char **args; 
	int status;

	do {
		printf("rshell> ");
		line = rshell_read_line();
		args = rshell_split_line(line);
		status = rshell_execute(args);	

		free(line);
		free(args);
	} while (status);
}

char *rshell_read_line() 
{
	int pos = 0;
	int bufsize = 1024;
	char *buffer = malloc(bufsize * sizeof(char));
	int c;
	
	if(!buffer) {
		fprintf(stderr, "rshel: allocation error");
	}
	
	while (1) {
		c = getchar();
		
		if(c == EOF || c == '\n') {
			buffer[pos] = '\0';
			return buffer;
		} else {
			buffer[pos++] = c;
		}
		if (pos > bufsize) {
			bufsize += bufsize;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "rshell: allloc error");
				exit(EXIT_FAILURE);
			}
		}
	}
}

char **rshell_split_line(char *line) 
{
	int bufsize = 64;
	int posl = 0; // pos of line
	int i = 0;
	char **tokens = malloc(bufsize * sizeof(char *));
	char *token = malloc(256);

	while(line[posl] != '\0') {
		
		while(line[posl] == ' ') 
			posl++;
		if(line[posl] == '\0')
			break;

		int post = 0; // pos of temp token
		while (line[posl] != ' ' && line[posl] != '\0' ) {
			token[post] = line[posl];
			post++;
			posl++;
		}
		
		token[post] = '\0';
		tokens[i++] = strdup(token);

		if (i>bufsize) {
		bufsize += bufsize;
		tokens = realloc(tokens, bufsize * sizeof(char *));
		}	
	}
	tokens[i] = NULL;
	free(token);
	return tokens;
}

int rshell_launch(char **args) 
{

	//check for pipe
	int pipe_pos = -1;
        for(int i = 0; args[i] != NULL; i++) {
                if (strcmp(args[i], "|") == 0) {
                        pipe_pos = i;
                        break;
                }
        }
	if (-1 != pipe_pos) 
		return rshell_pipe(args, pipe_pos);
	

	// check for "&"
	int i;
        int background = 0;
        for(i = 0; args[i] != NULL; i++) {
                if (strcmp(args[i], "&") == 0) {
                        background = 1;
                        break;
                }

        }
        if (0 != background)
                return rshell_background(args, i);

	//check for ">"

	int outpos = -1;
	for(int i = 0; args[i] != NULL; i++) {
		if(strcmp(args[i], ">") == 0) {
			outpos = i;
			break;
		}
	}
	if (-1 != outpos) 
		return o_redirection(args, outpos);
	
	
	pid_t pid, wpid;
	int status;
	pid = fork();
	if (pid == 0) {
		if(execvp(args[0], args) == -1) {
			perror("rshell");
	
		}
		exit(EXIT_FAILURE);
	}
	else if(pid<0) {
		perror("rshell");	
	}
	else {
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int rshell_pipe(char **args, int pipe_pos) 
{	
	args[pipe_pos] = NULL;

	char **left_cmd = args;
	char **right_cmd = &args[pipe_pos + 1];

	int fd[2];

	if (pipe(fd) == -1) {
		perror("pipe");
		return 1;
	}
	
	pid_t pid1 = fork();

	if (pid1 < 0) {
		perror("forking");
		return 1;
	} 
	if (pid1 == 0) {
		dup2(fd[1], STDOUT_FILENO);
	
		close(fd[0]);
		close(fd[1]);
		
		if(execvp(left_cmd[0], left_cmd) == -1) {
			perror("rshell");
		}
		exit(EXIT_FAILURE);
	
	}

	pid_t pid2 = fork();

	if (pid2 < 0) {
		perror("fork");
		return 1;
	}

	if (pid2 == 0) {
		dup2(fd[0], STDIN_FILENO);
		
		close(fd[0]);
		close(fd[1]);

		if(execvp(right_cmd[0], right_cmd) == -1) {
                        perror("rshell");
		 }
                exit(EXIT_FAILURE);
	}

	close(fd[0]);
	close(fd[1]);

	waitpid(pid1, NULL, 0);
	waitpid(pid2, NULL, 0);
	
	return 1;
} 

int rshell_background(char **args, int i) {
	
	args[i] = NULL;
	pid_t pid = fork();
	if(pid == 0) {
		if(execvp(args[0], args) == -1) {
			perror("rshell");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0) {
		perror("forking");
	}
	
	return 1;
}

int o_redirection(char **args, int outpos) 
{
	
	args[outpos] = NULL;
	char **leftcmd = args;
	char **rightcmd = &args[outpos+1];
	
	pid_t pid;
	pid = fork();
	if (pid == 0) {
		int fd = open(rightcmd[0], O_WRONLY | O_CREAT | O_TRUNC, 0644 );

		dup2(fd, STDOUT_FILENO);
		close(fd);
		
		if(-1 == execvp(leftcmd[0], leftcmd)){
			perror("rshell");
		}
		exit(EXIT_FAILURE);
	}
	else if(pid < 0) 
		perror("fork");

	else {
		waitpid(pid, NULL, 0);
	}
	return 1;
}

void sigchld_handler(int sig) {
		while(waitpid(-1, NULL, WNOHANG) > 0)
		;
	}

int rshell_execute(char** args) 
{
	
	if(args[0] == NULL) {
		return 1;
	}
	if(strcmp(args[0], "exit" ) == 0){
		exit(EXIT_SUCCESS);
	}
	
	return rshell_launch(args);
}

int main(int argc, char **argv) {

	signal(SIGCHLD, sigchld_handler);

	rshell_loop();

	return 0;	
}



