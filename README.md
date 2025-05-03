##Tung Duc Vu: 2023558
	Input reading and parsing commands
	Main method: Input handling
	Github

##Turbold Amarbat: 2023559
	Process forking and execution
	Command history
	Main method: Command execution, Command history

##Assignment description:
The simple shell program is basically an infinite while loop, which read user input, create a new process for execution and carry out the execution of user input command.

##The simple shell work flow with example:
User Input: ls -l
1. Shell reads input: "ls -l"
2. Shell tokenizes: Command = "ls", Args = ["-l"]
3. Shell forks a child process
4. Child process: execvp("ls", ["-l"])
5. Command output (if any): displayed in terminal
6. Shell waits for command to finish (waitpid())
7. Command completes, child process exits
8. Shell displays prompt again: simple_shell>

##Limitation of simple shell:
1. Lack of built-in commands (cd, export,...)
2. Cannot handle background processes (commands ending with &)
3. No command history (user cannot use up/down arrow keys to recall previous commands)
4. Fixed command length, number of pipes and argument length limit (although it can be changed directly in the code)
5. Does not support handling environment variable (user cannot cd ~)
6. Lack of signal handling (user cannot ^C or ^Z)



