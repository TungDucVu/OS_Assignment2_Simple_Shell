#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64

// 1. Function for Shell to read user input
char * read_input() {
     char *input = NULL;
     size_t len = 0;
     size_t nread;

     printf("simple_shell> ");
     nread = getline(&input, &len, stdin);
     if (nread == -1) {
          printf("Error reading user input\n");
          exit(1);
     }

     input[nread - 1] = '\0';            
     // remove new line character
     return input;
}

// 2. Function for Shell to tokenize the input by command and arguments + handling pipes
void parse_input(char *input, char **args, char **pipe_args) {
     char *token;
     int i = 0;

     // pipe handling
     char * pipe_position = strchr(input, '|');                  
     // check if exists a '|' (a pipe)

     if (pipe_position != NULL) {
          *pipe_position = '\0';                                 
          // mark the end of the pipe -> split the input at the pipe
          pipe_position++;                                       
          // move pass the pipe

          // token nize the part of the pipe
          token = strtok(pipe_position, " ");
          while (token != NULL) {
               pipe_args[i++] = token;
               token = strtok(NULL, " ");
          }
          pipe_args[i] = NULL;
          // NULL terminate the arguments array
     }

     
     // tokenize input by spaces
     token = strtok(input, " "); 
     while (token != NULL) {
          args[i++] = token;
          token = strtok(NULL, " ");
     }
     args[i] = NULL;  
     // NULL terminate the arguments array to mark the end of the arguments
}

// 3. Function for Shell to fork a child process
pid_t fork_process() {
     pid_t pid = fork();
     if (pid < 0) {
          printf("Error forking a child process\n");
          exit(1);
     }
     return pid;                        
     // pid == 0 then child is created
     // pid > 0 then we are in parent process. In this case we should wait for the child to terminate
}

// 4. Function to execute commands: exec()
void execute_command(char **args) {
     if (execvp(args[0], args) == 1)  {
          printf("Error executing command in child process\n");
          exit(1);
     }
}

// 4*. Function to execute with pipes
void execute_with_pipes(char **args, char **pipe_args) {
     int pipefd[2];
     pid_t pid1, pid2;

     if (pipe(pipefd) == -1) {
          printf("Error executing pipe command\n");
          exit(1);
     }

     pid1 = fork_process();
     if (pid1 == 0) {
          // right side = write end -> dup2 + close unused end
          dup2(pipefd[1], STDOUT_FILENO);
          close(pipefd[0]);
          close(pipefd[1]);
          execute_command(args);
     }
     pid2 = fork_process();
     if (pid2 == 0) {
          // left side = read end -> dup2 + close unused end
          dup2(pipefd[0], STDIN_FILENO);
          close(pipefd[1]);
          close(pipefd[0]);
          execute_command(args);
     }

     // tell parent to wait for children termination:
     close(pipefd[0]);
     close(pipefd[1]);
     waitpid(pid1, NULL, 0);
     waitpid(pid2, NULL, 0);
}

// 5. Function for parrent process to wait for the child process termination: waitpid()
void wait_for_child(pid_t pid) {
     int status;
     waitpid(pid, &status, WUNTRACED);       
     // WUNTRACED tells waitpid to also return even if the child process stopped, not just exit
}
