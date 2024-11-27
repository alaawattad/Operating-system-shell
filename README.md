# Operating-system-shell
# SMASH - Simple Shell

SMASH (Shell Made As Simple as Hope) is a basic shell program that supports built-in commands, background and foreground jobs, and job management, while allowing users to run external commands. This project is intended to help you understand how a shell works, process management, and job control.

## Features

### Built-in Commands

1. **chprompt [new-prompt]**
   - Changes the shell prompt to the provided string. If no argument is provided, it resets the prompt to "smash".
   - Example:
     ```bash
     smash> chprompt MyNewPrompt
     MyNewPrompt> chprompt
     smash>
     ```

2. **showpid**
   - Displays the process ID (PID) of the current shell process.
   - Example:
     ```bash
     smash> showpid
     smash pid is 12345
     smash>
     ```

3. **pwd**
   - Prints the current working directory.
   - Example:
     ```bash
     smash> pwd
     /home/user/directory
     smash>
     ```

4. **cd <path>**
   - Changes the current working directory. Special argument `-` changes to the previous directory.
   - Example:
     ```bash
     smash> cd /home/user
     smash> pwd
     /home/user
     smash> cd -
     smash> pwd
     /home/previous_directory
     ```

5. **jobs**
   - Lists the background jobs in the shell.
   - Example:
     ```bash
     smash> sleep 100 &
     smash> sleep 200 &
     smash> jobs
     [1] sleep 100 &
     [2] sleep 200 &
     smash>
     ```

6. **fg [job-id]**
   - Brings a job from the background to the foreground.
   - Example:
     ```bash
     smash> fg 1
     sleep 100 & 12345
     ```

7. **quit [kill]**
   - Exits the shell. If the "kill" argument is specified, it terminates all unfinished jobs before quitting.
   - Example:
     ```bash
     smash> quit kill
     ```

8. **kill -<signum> <job-id>**
   - Sends a signal to a specific job. The `signum` specifies the signal number (e.g., SIGKILL, SIGTERM).
   - Example:
     ```bash
     smash> kill -9 1
     signal number 9 was sent to pid 12345
     ```

9. **alias <name>='<command>'**
   - Creates a shortcut for a command. 
   - Example:
     ```bash
     smash> alias ll='ls -l'
     smash> ll
     total 0
     ```
   
10. **unalias <name_1> <name_2> …**
    - Removes the specified aliases.
    - Example:
      ```bash
      smash> alias ll='ls -l'
      smash> unalias ll
      ```

### Error Handling

Each built-in command has specific error handling:
- Invalid arguments
- Non-existent jobs
- Permissions issues (e.g., for `cd` or `kill`)

### Job Management

- **Foreground Jobs**: The shell waits for the foreground process to finish before accepting new commands.
- **Background Jobs**: The shell runs these commands in the background, allowing the user to continue typing new commands.

### Job List Management

- **Job IDs**: Assigned by the shell and unique within the session.
- **Job States**: Either foreground or background.
- **Finished Jobs**: Automatically removed from the job list before executing new commands.

## How to Run

1. Clone the repository:
   bash
   git clone https://github.com/alaawattad/Operating-system-shell.git
Compile the program:

make
Run the shell:

./smash
Execute commands like chprompt, pwd, cd, jobs, fg, kill, and others.

Requirements
Linux or Unix-like OS
GCC compiler
Make