#Simple Shell - Command-Line Interpreter
#Description

This project is an implementation of a simple UNIX command-line interpreter, also known as a shell. It serves as a custom version of the sh shell, providing limited functionality compared to more advanced shell programs.

The shell is designed to handle various commands and operations, including:

    Execution of commands within the PATH environment variable.
    Support for built-in shell commands such as env, exit, and cd.
    Redirection of standard input and output for flexible command handling.
    Effective management of the PATH environment variable for locating executable program files.

Files

The shell program consists of the following files:

    main.c: This file serves as the entry point for the shell program. It manages the interaction between the user and the shell, handling input and command execution.
    helper_functions.c: This file contains the implementation of several helper functions used throughout the shell program.
    shell.h: This header file includes function prototypes, macros, and libraries required for the project.

Functionality

The core functionality of the shell program is organized as follows:

    initializer() function: This function is responsible for initiating the execution process. It receives the current command to execute and its type. It handles the execution of both external and built-in commands.

    Execution of external or path commands: If the command is an external or path command, the fork() system call is invoked to create a child process. The child process then calls the execute_command() function to execute the specified command, while the parent process waits for the child to complete execution.

    execute_command() function: This function is responsible for executing the commands passed to it as arguments. It manages the execution of built-in commands such as cd (change directory) and exit. For external commands, it utilizes execve() to execute them successfully.

Usage

To utilize the shell program, follow these steps:

    Compile the source code using the following command:

bash

$ gcc -Wall -Werror -Wextra -pedantic *.c -o hsh

    After successful compilation, run the program using the following command:

bash

$ ./hsh

    You can now input commands, and the shell program will execute them accordingly. The prompt displayed for command input is represented by the '$' symbol.

Example usage:

bash

$ /bin/ls README.md shell.c ...
$ pwd
/home/user/simple_shell
$ cd /
$ pwd
/
$ exit
$

Feel free to experiment with different commands and explore the functionality of the shell program.

Authers:
Maria and Phumlani