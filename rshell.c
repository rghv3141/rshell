#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char **rshell_split_line(char *);
int rshell_lauch(char **);
int rshell_execute(char **);
char *rshell_read_line();

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

	rshell_loop();

	return 0;	
}



