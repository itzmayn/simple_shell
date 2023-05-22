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
 * _strcmp - Compares two strings
 * @first: First string to be compared
 * @second: Second string to be compared
 *
 * This function compares the characters of two strings lexicographically.
 *
 * Return: An integer value indicating the difference between the strings.
 *         - A negative value if the first string is less than the second.
 *         - Zero if the strings are equal.
 *         - A positive value if the first string is greater than the second.
 */
int _strcmp(char *first, char *second)
{
    int i = 0;

    while (first[i] != '\0')
    {
        if (first[i] != second[i])
        {
            break;
        }
        i++;
    }

    return (first[i] - second[i]);
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
 * _strspn - Gets the length of a prefix substring
 * @str1: String to be searched
 * @str2: String to be used
 *
 * This function calculates the length of the initial segment of str1
 * consisting of only the characters that appear in str2. It searches for
 * matches between str1 and str2 and returns the number of matching bytes.
 *
 * Return: Number of bytes in the initial segment of str1 that are part of str2.
 */
int _strspn(char *str1, char *str2)
{
    int i = 0;
    int match = 0;

    while (str1[i] != '\0')
    {
        if (_strchr(str2, str1[i]) == NULL)
            break;
        match++;
        i++;
    }

    return match;
}

/**
 * _strcspn - Computes the segment of str1 that consists of characters not in str2
 * @str1: String to be searched
 * @str2: String to be used
 *
 * This function calculates the index at which a character in str1 exists in str2.
 * It searches for matches between str1 and str2 and returns the length of the
 * segment of str1 that contains characters not present in str2.
 *
 * Return: Index at which a character in str1 exists in str2.
 */
int _strcspn(char *str1, char *str2)
{
    int len = 0;
    int i;

    for (i = 0; str1[i] != '\0'; i++)
    {
        if (_strchr(str2, str1[i]) != NULL)
            break;
        len++;
    }

    return len;
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
 * ctrl_c_handler - Handles the signal raised by CTRL-C
 * @signum: Signal number
 *
 * This function is the signal handler for the CTRL-C signal (SIGINT). It
 * displays a new prompt when the signal is received, indicating that the
 * shell is ready to accept a new command.
 *
 * Return: void
 */
void ctrl_c_handler(int signum)
{
    if (signum == SIGINT)
    {
        /* Display new prompt when CTRL-C signal is received */
        display("\n($) ", STDIN_FILENO);
    }
}

/**
 * remove_comment - Removes/ignores everything after a '#' character
 * @input: Input string to be modified
 *
 * This function removes any characters after the first '#' character in the
 * input string. It is used to remove comments from the input, as anything
 * after the '#' character is ignored.
 *
 * Return: void
 */
void remove_comment(char *input)
{
    int i = 0;

    if (input[i] == '#')
    {
        /* If '#' is the first character, remove the entire line */
        input[i] = '\0';
    }
    else
    {
        /* Find the first '#' character preceded by a space and remove the rest of the line */
        while (input[i] != '\0')
        {
            if (input[i] == '#' && input[i - 1] == ' ')
            {
                break;
            }
            i++;
        }
        input[i] = '\0';
    }
}

/**
 * _strcat - Concatenates two strings
 * @destination: Pointer to the string to be concatenated
 * @source: Pointer to the string to concatenate
 *
 * This function appends the characters of the source string to the
 * destination string, overwriting the terminating null byte ('\0') at
 * the end of the destination, and then adds a new terminating null
 * byte. The resulting concatenated string is returned.
 *
 * Return: Pointer to the resulting concatenated string.
 */
char *_strcat(char *destination, char *source)
{
    char *new_string = NULL;
    int len_dest = _strlen(destination);
    int len_source = _strlen(source);

    new_string = malloc(sizeof(*new_string) * (len_dest + len_source + 1));
    _strcpy(destination, new_string);
    _strcpy(source, new_string + len_dest);
    new_string[len_dest + len_source] = '\0';

    return (new_string);
}

/**
 * _strchr - Locates a character in a string
 * @s: The string to be searched
 * @c: The character to be checked
 *
 * This function searches for the first occurrence of the character c in the
 * string s. It returns a pointer to the location of the character if found,
 * or NULL if the character is not found in the string.
 *
 * Return: A pointer to the first occurrence of c in s, or NULL if c is not found
 */
char *_strchr(char *s, char c)
{
    int i = 0; /* Counter for iterating over string s */

    /* Iterate over s until the end of the string or until the character c is found */
    for (; s[i] != c && s[i] != '\0'; i++)
    {
        /* Empty body, just incrementing i */
    }

    /* If c is found in s, return a pointer to the location in s where c was found */
    if (s[i] == c)
    {
        return (s + i);
    }
    else
    {
        /* Otherwise, return NULL to indicate that c was not found in s */
        return (NULL);
    }
}

