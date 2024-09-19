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

// 2. Function for Shell to tokenize the input by command and arguments
void parse_input (char *input, char **args) {
     char *token;
     int i = 0;

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

// 4. Function for child process to execute commands: exec()
void execute_command(char **args) {
     if (execvp(args[0], args) == 1)  {
          printf("Error executing command in child process\n");
          exit(1);
     }
}

// 5. Function for parrent process to wait for the child process termination: waitpid()
void wait_for_child(pid_t pid) {
     int status;
     waitpid(pid, &status, WUNTRACED);       
     // WUNTRACED tells waitpid to also return even if the child process stopped, not just exit
}
