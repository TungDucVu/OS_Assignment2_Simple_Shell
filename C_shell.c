#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64
#define MAX_PIPES 10  

// 1. Read user input
char* read_input() {
    char *input = NULL;
    size_t len = 0;
    ssize_t nread;

    printf("simple-shell> ");
    nread = getline(&input, &len, stdin);
    if (nread == -1) {
        perror("getline");
        exit(EXIT_FAILURE);
    }

    // Remove the newline character 
    input[nread - 1] = '\0';
    return input;
}

// 2. Handling pipe inputs (split input into commands separated by "|")
int parse_input(char *input, char **commands) {
    int i = 0;
    char *token = strtok(input, "|");

    // Split input by "|" and store each command in the commands array
    while (token != NULL && i < MAX_PIPES) {
        commands[i++] = token;
        token = strtok(NULL, "|");
    }
    commands[i] = NULL;  // Null-terminate the commands array

    return i;  // Return the number of commands 
}

// 2*. Handling commands (break down a single command into its component arguments)
void parse_command(char *command, char **args) {
    int i = 0;
    char *token = strtok(command, " ");

    // Tokenize the command string by spaces to extract the command and its arguments
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;  // Null-terminate the arguments array
}

// 3. Forking a new process
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
    if (execvp(args[0], args) == -1) {  
        printf("Error executing command\n");
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
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < num_commands; i++) {
        char *args[MAX_ARGS];
        parse_command(commands[i], args);  // Parse each command into arguments

        pid = fork_process();  
        if (pid == 0) {

            // Not the first command: set the input from the previous pipe
            if (i > 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }

            // Not the last command: set the output to the next pipe
            if (i < num_commands - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }

            // Close all pipes in the child process
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefds[j]);
            }

            // Execute the command
            execute_command(args);  // Use the execute_command function for cleaner code
        }
    }

    // Parent process: Close all pipes and wait for all child processes
    for (i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefds[i]);
    }

    for (i = 0; i < num_commands; i++) {
        wait(NULL);  // Wait for each child process to finish
    }
}

// Main method
int main() {
    char *input;
    char *commands[MAX_PIPES + 1];  // To hold split commands (maximum 10 pipes)
    int num_commands;
    char *args[MAX_ARGS];
    pid_t pid;

    while (1) {
        // 1. Read user input
        input = read_input();

        // Check if command is "exit" to terminate the shell
        if (strcmp(input, "exit") == 0) {
            free(input);
            break;
        }

        // 2. Parse input into commands (split by pipes if it is a piped command)
        num_commands = parse_input(input, commands);
        if (num_commands > 1) {
            // Handle piped command case
            execute_piped_commands(commands, num_commands);
        } else {
            // Handle single command case
            parse_command(commands[0], args);  
            pid = fork();
            if (pid == 0) {
                execute_command(args);  
            } else {
                wait(NULL);  
            }
        }

        free(input);
    }

    return 0;
}
