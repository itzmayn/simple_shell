#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

/* Constants */
#define EXTERNAL_COMMAND 1
#define INTERNAL_COMMAND 2
#define PATH_COMMAND 3
#define INVALID_COMMAND -1

/* Function Prototypes */
void display(char *, int);
char **tokenizer(char *, char *);
int _strlen(char *);
void _strcpy(char *, char *);
void _line_rm(char *);
int _strcmp(char *, char *);
char *_strcat(char *, char *);
int _strspn(char *, char *);
int _strcspn(char *, char *);
char *_strchr(char *, char);
void ctrl_c_handler(int);
void remove_comment(char *);
int parse_command(char *);
char *_strtok_r(char *, char *, char **);
int power(int base, int exponent);
int _atoi(char *);
int multiply(int a, int b);
int subtract(int a, int b);
int add(int a, int b);
void *_realloc(void *ptr, unsigned int old_size, unsigned int new_size);

void execute_command(char **, int);
char *confirm_loc(char *);
void (*get_func(char *))(char **);
char *_getenv(char *);
void env(char **);
void quit(char **);
extern void handle_senario_a(void);
extern void init(char **current_command, int type_command);

/* Global Variables */
extern char **environ;
extern char *line;
extern char *shell_name;
extern char **commands;
extern int status;

/* Definitions */
char **commands = NULL;
char *line = NULL;
char *shell_name = NULL;
int status = 0;

/**
 * main - Entry point of the shell program
 * @argc: The number of command-line arguments (unused)
 * @argv: Array of command-line arguments
 *
 * Return: The status code of the shell program
 */
int main(int argc __attribute__((unused)), char **argv)
{
	char **current_command = NULL;
	int i, type_command = 0;
	size_t n = 0;

	/* Register signal handler for Ctrl+C */
	signal(SIGINT, ctrl_c_handler);

	/* Set the shell name */
	shell_name = argv[0];

	while (1)
	{
		/* Handle scenario A */
		handle_senario_a();

		/* Display the shell prompt */
		display(" ($) ", STDOUT_FILENO);

		/* Read the command from the user */
		if (getline(&line, &n, stdin) == -1)
		{
			free(line);
			exit(status);
		}

		/* Remove leading/trailing whitespaces and remove comments */
		_line_rm(line);
		remove_comment(line);

		/* Tokenize the command by semicolon */
		commands = tokenizer(line, ";");

		for (i = 0; commands[i] != NULL; i++)
		{
			/* Tokenize each individual command */
			current_command = tokenizer(commands[i], " ");

			if (current_command[0] == NULL)
			{
				free(current_command);
				break;
			}

			/* Determine the type of command */
			type_command = parse_command(current_command[0]);

			/* Initialize and execute the command */
			init(current_command, type_command);

			free(current_command);
		}

		free(commands);
	}

	free(line);

	return (status);
}

/**
 * handle_senario_a - Handles a non-interactive mode
 *
 * This function is responsible for handling a scenario where the shell
 * is not running in interactive mode. It reads commands from the standard input
 * and executes them.
 *
 * Return: void
 */
void handle_senario_a(void)
{
	char **current_command = NULL;
	int i, type_command = 0;
	size_t n = 0;

	/* Check if the standard input is not a terminal */
	if (!(isatty(STDIN_FILENO)))
	{
		/* Read commands from the standard input */
		while (getline(&line, &n, stdin) != -1)
		{
			/* Remove leading/trailing whitespaces and remove comments */
			_line_rm(line);
			remove_comment(line);

			/* Tokenize the command by semicolon */
			commands = tokenizer(line, ";");

			for (i = 0; commands[i] != NULL; i++)
			{
				/* Tokenize each individual command */
				current_command = tokenizer(commands[i], " ");

				if (current_command[0] == NULL)
				{
					free(current_command);
					break;
				}

				/* Determine the type of command */
				type_command = parse_command(current_command[0]);

				/* Initialize and execute the command */
				init(current_command, type_command);

				free(current_command);
			}

			free(commands);
		}

		free(line);
		exit(status);
	}
}

/**
 * power - Calculates the power of a number
 * @base: The base number
 * @exponent: The exponent
 *
 * This function calculates the result of raising the base number to the given exponent.
 *
 * Return: The result of raising the base to the exponent
 */
int power(int base, int exponent)
{
    int result = 1;

    while (exponent > 0)
    {
        result *= base;
        exponent--;
    }

    return result;
}

/**
 * init - Start executing the command
 * @current_command: The current command to execute
 * @type_command: The type of command
 *
 * This function is responsible for initiating the execution of the command.
 * If the command is an external command or a command in the PATH, it creates
 * a child process to execute the command. Otherwise, it directly executes
 * the command.
 *
 * Return: void
 */
void init(char **current_command, int type_command)
{
    pid_t PID;

    if (type_command == EXTERNAL_COMMAND || type_command == PATH_COMMAND)
    {
        PID = fork();
        if (PID == 0)
            execute_command(current_command, type_command);
        else
        {
            waitpid(PID, &status, 0);
            status >>= 8;
        }
    }
    else
        execute_command(current_command, type_command);
}
