#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/types.h>

int main(){
	int fd[2];
	pipe(fd);

	pid_t child_id = fork();

	if(child_id < 0) exit(EXIT_FAILURE);5
    if(child_id > 0) { 

        dup2(fd[1], STDOUT_FILENO); 
        close(fd[0]); 
        
        char *arg[] = {"ls", NULL};
        execv("/bin/ls", arg); 
    } 
    else{ 
        dup2(fd[0], STDIN_FILENO); 
        close(fd[1]); 
        
        char *arg[] = {"wc", "-l", NULL};
        execv("/usr/bin/wc", arg); 
    }
}