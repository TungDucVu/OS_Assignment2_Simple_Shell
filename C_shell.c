#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64
#define MAX_PIPES 10  // Maximum number of pipes supported

// 1. Read user input 
char* read_input() {
     char *input = NULL;
     size_t len = 0;
     ssize_t nread;

     printf("simple_shell> ");
     nread = getline(&input, &len, stdin);
     if (nread == -1) {
          printf("Error reading user input\n");
          exit(1);
     }

     // Remove the newline character from input
     input[nread - 1] = '\0';
     return input;
}

// 2. Handling pipe inputs (split input into commands separated by "|")
int parse_input(char *input, char **commands) {
     int i = 0;
     char *token = strtok(input, "|");

     // Split input by pipes and store each command in the commands array
     while (token != NULL && i < MAX_PIPES) {
     commands[i++] = token;
     token = strtok(NULL, "|");
     }
     commands[i] = NULL;  // Null-terminate to mark then end of the commands array

     return i;  // Return the number of commands 
}

// 2*. Handling commands (break down a single commands into its component arguments)
void parse_command(char *command, char **args, int *background) {
     int i = 0;
     char *token = strtok(command, " ");
     *background = 0;  // Default to foreground

     // Tokenize the command string by spaces to extract the command and its arguments
     while (token != NULL) {
     if (strcmp(token, "&") == 0) {
          *background = 1;  // Set background flag if '&' is found
          break;
     }
     args[i++] = token;
     token = strtok(NULL, " ");
     }
     args[i] = NULL;  // Null-terminate to mark the end of the arguments array
}

// 3. Forking new process
pid_t fork_process() {
     pid_t pid = fork();
     if (pid < 0) {
          printf("Error forking a child process\n");
          exit(1);
     }
     return pid;                        
}

// 4. Executing commands
void execute_command(char **args) {
     if (execvp(args[0], args) == 1)  {
          printf("Error executing command in child process\n");
          exit(1);
     }
}

// 4*. Executing the piped commands
void execute_piped_commands(char **commands, int num_commands) {
     int pipefds[2 * (num_commands - 1)];  // One pipe for each pair of commands
     pid_t pid;
     int i;

     // Create pipes for each command pair
     for (i = 0; i < num_commands - 1; i++) {
          if (pipe(pipefds + i * 2) == -1) {
               printf("Error creating pipes for each command pair");
               exit(1);
          }
     }

     for (i = 0; i < num_commands; i++) {
          char *args[MAX_ARGS];
          int background;
          parse_command(commands[i], args, &background);  // Parse each command into arguments

          pid = fork_process();

          if (pid == 0) {
               // Child process: Setup pipes for input/output redirection

               // If it's not the first command, set the input from the previous pipe
               if (i > 0) {
                    dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
               }

               // If it's not the last command, set the output to the next pipe
               if (i < num_commands - 1) {
                    dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
               }

               // Close all pipes in the child process
               for (int j = 0; j < 2 * (num_commands - 1); j++) {
                    close(pipefds[j]);
               }

               // Execute the command
               execute_command(*args);
          } else {
               // Parent process: If not background, wait for the child
               if (!background) {
                    wait(NULL);
               }
          }
          }

          // Parent process: Close all pipes
          for (i = 0; i < 2 * (num_commands - 1); i++) {
               close(pipefds[i]);
     }
}
