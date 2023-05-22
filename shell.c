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
 *_strlen - counts string length
 *@string: string to be counted
 *
 * Return: length of the string
 */

int _strlen(char *string)
{
	int len = 0;

	if (string == NULL)
		return (len);
	for (; string[len] != '\0'; len++)
		;
	return (len);
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
 * tokenizer - Tokenizes input and stores it into an array
 * @input_string: The input string to be parsed
 * @delim: The delimiter to be used, must be a single character string
 *
 * This function tokenizes the input string based on the provided delimiter
 * and stores the tokens in an array. The resulting array of tokens is returned.
 *
 * Return: An array of tokens
 */
char **tokenizer(char *input_string, char *delim)
{
    int num_delim = 0;
    char **av = NULL;
    char *token = NULL;
    char *save_ptr = NULL;

    token = _strtok_r(input_string, delim, &save_ptr);

    while (token != NULL)
    {
        av = _realloc(av, sizeof(*av) * num_delim, sizeof(*av) * (num_delim + 1));
        av[num_delim] = token;
        token = _strtok_r(NULL, delim, &save_ptr);
        num_delim++;
    }

    av = _realloc(av, sizeof(*av) * num_delim, sizeof(*av) * (num_delim + 1));
    av[num_delim] = NULL;

    return (av);
}

/**
 * subtract - Subtracts one integer from another
 * @a: The integer to subtract from
 * @b: The integer to subtract
 *
 * Return: The result of subtracting b from a
 */
int subtract(int a, int b)
{
    return a - b;
}

/**
 * multiply - Multiplies two integers together
 * @a: The first integer to multiply
 * @b: The second integer to multiply
 *
 * Return: The product of a and b
 */
int multiply(int a, int b)
{
    return a * b;
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
 * parse_command - Determines the type of the command
 * @command: The command to be parsed
 *
 * This function analyzes the command to determine its type. It takes the command
 * as an argument and checks for specific conditions to identify the type of the
 * command. It checks for an external command by searching for the presence of
 * a '/' character. It checks for internal commands by comparing the command to
 * a list of predefined internal commands. If the command matches an internal
 * command, it is considered an internal command. If the command is not an internal
 * command, it checks if it exists in the PATH by using the confirm_loc() function.
 * If a valid path is found, it is considered a path command. Otherwise, it is
 * considered an invalid command.
 *
 * Return: Constant representing the type of the command
 */
int parse_command(char *command)
{
    int i;
    char *internal_command[] = {"env", "exit", NULL};
    char *path = NULL;

    /* Check for external command by searching for '/' */
    for (i = 0; command[i] != '\0'; i++)
    {
        if (command[i] == '/')
            return (EXTERNAL_COMMAND);
    }

    /* Check for internal commands by comparing with predefined list */
    for (i = 0; internal_command[i] != NULL; i++)
    {
        if (_strcmp(command, internal_command[i]) == 0)
            return (INTERNAL_COMMAND);
    }

    /* Check if command exists in the PATH */
    path = confirm_loc(command);
    if (path != NULL)
    {
        free(path);
        return (PATH_COMMAND);
    }

    /* Invalid command if not matched to any type */
    return (INVALID_COMMAND);
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

/**
 * display - Displays a string to stdout
 * @string: The string to be displayed
 * @stream: The stream to display the string to
 *
 * This function writes the characters of the string to the specified stream.
 * It iterates over the characters of the string until the null terminator is
 * reached and writes each character to the stream using the write system call.
 *
 * Return: void
 */
void display(char *string, int stream)
{
    int i = 0;

    for (; string[i] != '\0'; i++)
    {
        write(stream, &string[i], 1);
    }
}

/**
 * _line_rm - Removes new line from a string
 * @str: The string to be modified
 *
 * This function iterates over the characters of the string until it reaches
 * the null terminator or encounters a new line character ('\n'). If a new line
 * character is found, it replaces it with the null terminator, effectively
 * removing the new line from the string.
 *
 * Return: void
 */
void _line_rm(char *str)
{
    int i = 0;

    while (str[i] != '\0')
    {
        if (str[i] == '\n')
            break;
        i++;
    }

    str[i] = '\0';
}

/**
 * _strcpy - Copies a string to another buffer
 * @source: The source string to be copied
 * @dest: The destination buffer to copy to
 *
 * This function copies the characters from the source string to the destination
 * buffer until it reaches the null terminator. It assumes that the destination
 * buffer has enough space to accommodate the entire source string.
 *
 * Return: void
 */
void _strcpy(char *source, char *dest)
{
    int i = 0;

    for (; source[i] != '\0'; i++)
    {
        dest[i] = source[i];
    }
    dest[i] = '\0';
}

/**
 * env - Displays the current environment
 * @tokenized_command: The command entered
 *
 * This function is used to display the current environment. It takes the
 * tokenized command as an argument, although it is not used in this implementation.
 * The function iterates over the `environ` array, which contains the environment
 * variables, and displays each variable followed by a new line.
 *
 * Return: void
 */
void env(char **tokenized_command __attribute__((unused)))
{
    int i;

    /* Iterate over the `environ` array */
    for (i = 0; environ[i] != NULL; i++)
    {
        /* Display each environment variable */
        display(environ[i], STDOUT_FILENO);

        /* Add a new line after each variable */
        display("\n", STDOUT_FILENO);
    }
}

/**
 * quit - Exits the shell
 * @tokenized_command: The command entered
 *
 * This function is used to exit the shell. It takes the tokenized command as an
 * argument. If the command has no arguments, the function frees allocated memory
 * and exits the shell with the current status. If the command has one argument,
 * it is expected to be an exit status. The function converts the argument to an
 * integer and exits the shell with that status. If the command has more than one
 * argument, an error message is displayed.
 *
 * Return: void
 */
void quit(char **tokenized_command)
{
    int num_token = 0, arg;

    /* Count the number of tokens in the command */
    for (; tokenized_command[num_token] != NULL; num_token++)
        ;

    if (num_token == 1)
    {
        /* Free allocated memory and exit with current status */
        free(tokenized_command);
        free(line);
        free(commands);
        exit(status);
    }
    else if (num_token == 2)
    {
        /* Convert the argument to an integer */
        arg = _atoi(tokenized_command[1]);

        if (arg == -1)
        {
            /* Display an error message for an illegal number */
            display(shell_name, STDERR_FILENO);
            display(": 1: exit: Illegal number: ", STDERR_FILENO);
            display(tokenized_command[1], STDERR_FILENO);
            display("\n", STDERR_FILENO);
            status = 2;
        }
        else
        {
            /* Free allocated memory and exit with the specified status */
            free(line);
            free(tokenized_command);
            free(commands);
            exit(arg);
        }
    }
    else
    {
        /* Display an error message for too many arguments */
        display("$: exit doesn't take more than one argument\n", STDERR_FILENO);
    }
}

/**
 * execute_command - Executes a command based on its type
 * @tokenized_command: Tokenized form of the command (e.g., {ls, -l, NULL})
 * @command_type: Type of the command
 *
 * Return: void
 */
void execute_command(char **tokenized_command, int command_type)
{
    void (*func)(char **command);

    if (command_type == EXTERNAL_COMMAND)
    {
        // Execute external command using execve
        if (execve(tokenized_command[0], tokenized_command, NULL) == -1)
        {
            perror(_getenv("PWD"));
            exit(2);
        }
    }
    else if (command_type == PATH_COMMAND)
    {
        // Execute command using the confirmed location from PATH
        if (execve(confirm_loc(tokenized_command[0]), tokenized_command, NULL) == -1)
        {
            perror(_getenv("PWD"));
            exit(2);
        }
    }
    else if (command_type == INTERNAL_COMMAND)
    {
        // Execute internal command by getting the corresponding function pointer
        func = get_func(tokenized_command[0]);
        func(tokenized_command);
    }
    else if (command_type == INVALID_COMMAND)
    {
        // Display an error message for invalid command and set the status to 127
        display(shell_name, STDERR_FILENO);
        display(": 1: ", STDERR_FILENO);
        display(tokenized_command[0], STDERR_FILENO);
        display(": not found\n", STDERR_FILENO);
        status = 127;
    }
}

/**
 * confirm_loc - Checks if a command is found in the PATH
 * @command: Command to be checked
 *
 * Return: Path where the command is found, NULL if not found
 */
char *confirm_loc(char *command)
{
    char **path_array = NULL;
    char *temp, *temp2, *path_cpy;
    char *path = _getenv("PATH");
    int i;

    // Check if PATH environment variable is empty or not set
    if (path == NULL || _strlen(path) == 0)
        return (NULL);

    // Create a copy of the PATH string
    path_cpy = malloc(sizeof(*path_cpy) * (_strlen(path) + 1));
    _strcpy(path, path_cpy);

    // Tokenize the PATH string using ':' as the delimiter
    path_array = tokenizer(path_cpy, ":");

    // Iterate through each directory in the PATH
    for (i = 0; path_array[i] != NULL; i++)
    {
        // Create the full path by concatenating directory and command
        temp2 = _strcat(path_array[i], "/");
        temp = _strcat(temp2, command);

        // Check if the full path exists and is accessible
        if (access(temp, F_OK) == 0)
        {
            free(temp2);
            free(path_array);
            free(path_cpy);
            return (temp);
        }

        // Free the temporary strings for the next iteration
        free(temp);
        free(temp2);
    }

    // Free the allocated memory and return NULL if command is not found in any directory
    free(path_cpy);
    free(path_array);
    return (NULL);
}

/**
 * add - Adds two integers together
 * @a: The first integer to add
 * @b: The second integer to add
 * Return: The sum of a and b
 */
int add(int a, int b)
{
    return a + b;
}
