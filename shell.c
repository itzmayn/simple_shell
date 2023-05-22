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
 * _atoi - Converts a string to an integer.
 *
 * @s: The string to convert to an integer.
 *
 * Return: The integer value of the string.
 */
int _atoi(char *s)
{
	unsigned int result = 0; /* The resulting integer */
	unsigned int negative = 0; /* Flag to indicate if the number is negative */

	/* Loop through the string */
	while (*s)
	{
		/* Check for a negative sign at the beginning of the string */
		if (*s == '-')
		{
			negative = 1;
		}
		
		/* Check for a digit */
		if (*s >= '0' && *s <= '9')
		{
			/* Shift the current result to the left by one digit and add the current digit */
			result = result * 10 + (*s - '0');
		}

		/* Move to the next character in the string */
		s++;
	}

	/* Return the result with a negative sign if necessary */
	return negative ? -result : result;
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
        /* Create a child process to execute the command */
        PID = fork();
        if (PID == 0)
        {
            /* Child process executes the command */
            execute_command(current_command, type_command);
        }
        else
        {
            /* Parent process waits for the child to finish executing */
            waitpid(PID, &status, 0);
            status >>= 8;
        }
    }
    else
    {
        /* Execute the command directly */
        execute_command(current_command, type_command);
    }
}

/**
 * _strtok_r - Tokenizes a string
 * @string: The string to be tokenized
 * @delim: The delimiter used to tokenize the string
 * @save_ptr: Pointer to keep track of the next token
 *
 * This function takes a string and a delimiter and returns the next token
 * from the string. The `save_ptr` argument is used to keep track of the next
 * token, allowing the caller to tokenize multiple strings simultaneously.
 *
 * Return: The next available token, or NULL if there are no more tokens.
 */
char *_strtok_r(char *string, char *delim, char **save_ptr)
{
	char *finish;

	/* If string is NULL, use the save_ptr */
	if (string == NULL)
		string = *save_ptr;

	/* If the current position is at the end of the string, no more tokens */
	if (*string == '\0')
	{
		*save_ptr = string;
		return (NULL);
	}

	/* Skip leading delimiters */
	string += _strspn(string, delim);

	/* If the current position is at the end of the string, no more tokens */
	if (*string == '\0')
	{
		*save_ptr = string;
		return (NULL);
	}

	/* Find the end of the token */
	finish = string + _strcspn(string, delim);

	/* If the end of the string is reached, update save_ptr and return the token */
	if (*finish == '\0')
	{
		*save_ptr = finish;
		return (string);
	}

	/* Null-terminate the token, update save_ptr, and return the token */
	*finish = '\0';
	*save_ptr = finish + 1;
	return (string);
}

/**
 * _realloc - Reallocate memory block
 * @ptr: Pointer to the memory previously allocated with malloc
 * @old_size: Size of the old memory block
 * @new_size: Size of the new memory block to be allocated
 *
 * This function reallocates a memory block, pointed to by ptr, to a new size
 * specified by new_size. It preserves the data in the old memory block as much
 * as possible.
 *
 * Return: Pointer to the address of the new memory block
 */
void *_realloc(void *ptr, unsigned int old_size, unsigned int new_size)
{
    void *temp_block;
    unsigned int i;

    if (ptr == NULL)
    {
        /* If ptr is NULL, equivalent to malloc(new_size) */
        temp_block = malloc(new_size);
        return (temp_block);
    }
    else if (new_size == old_size)
    {
        /* If new_size is equal to old_size, return ptr without reallocation */
        return (ptr);
    }
    else if (new_size == 0 && ptr != NULL)
    {
        /* If new_size is 0 and ptr is not NULL, equivalent to free(ptr) */
        free(ptr);
        return (NULL);
    }
    else
    {
        /* Allocate a new memory block of size new_size */
        temp_block = malloc(new_size);
        if (temp_block != NULL)
        {
            /* Copy the data from the old memory block to the new memory block */
            for (i = 0; i < minimum(old_size, new_size); i++)
            {
                *((char *)temp_block + i) = *((char *)ptr + i);
            }
            free(ptr);
            return (temp_block);
        }
        else
        {
            /* Allocation failed, return NULL */
            return (NULL);
        }
    }
}
