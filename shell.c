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



/*constants*/
#define EXTERNAL_COMMAND 1
#define INTERNAL_COMMAND 2
#define PATH_COMMAND 3
#define INVALID_COMMAND -1

#define minimum(x, y) (((x) < (y)) ? (x) : (y))

typedef struct map
{
	char *command_name;
	void (*func)(char **command);
} mapping_func;

extern char **environ;
extern char *line;
extern char *shell_name;
extern char **commands;
extern int status;


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


char **commands = NULL;
char *line = NULL;
char *shell_name = NULL;
int status = 0;

/**
 * main - the main shell code
 * @argc: number of arguments passed
 * @argv: program arguments to be parsed
 *
 * applies the functions in utils and helpers
 * implements EOF
 * prints error on Failure
 * Return: 0 on success
 */


int main(int argc __attribute__((unused)), char **argv)
{
	char **current_command = NULL;
	int i, type_command = 0;
	size_t n = 0;

	signal(SIGINT, ctrl_c_handler);
	shell_name = argv[0];
	while (1)
	{
		handle_senario_a();
		display(" ($) ", STDOUT_FILENO);
		if (getline(&line, &n, stdin) == -1)
		{
			free(line);
			exit(status);
		}
			_line_rm(line);
			remove_comment(line);
			commands = tokenizer(line, ";");

		for (i = 0; commands[i] != NULL; i++)
		{
			current_command = tokenizer(commands[i], " ");
			if (current_command[0] == NULL)
			{
				free(current_command);
				break;
			}
			type_command = parse_command(current_command[0]);

			/* init -   */
			init(current_command, type_command);
			free(current_command);
		}
		free(commands);
	}
	free(line);

	return (status);
}

/**
 * handle_senario_a - handles a mode that is not interactive
 *
 * Return: void
 */

void handle_senario_a(void)
{
	char **current_command = NULL;
	int i, type_command = 0;
	size_t n = 0;

	if (!(isatty(STDIN_FILENO)))
	{
		while (getline(&line, &n, stdin) != -1)
		{
			_line_rm(line);
			remove_comment(line);
			commands = tokenizer(line, ";");
			for (i = 0; commands[i] != NULL; i++)
			{
				current_command = tokenizer(commands[i], " ");
				if (current_command[0] == NULL)
				{
					free(current_command);
					break;
				}
				type_command = parse_command(current_command[0]);
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
 * power - Raises a number to a given power
 * @base: The base number
 * @exponent: The exponent to raise the base to
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
 * init - starts executing everything
 * @current_command: try to check current token
 * @type_command: parse token
 *
 * Return: void function
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

/**
 * _strtok_r - tokenizes a string
 * @string: string to be tokenized
 * @delim: delimiter to be used to tokenize the string
 * @save_ptr: pointer to be used to keep track of the next token
 *
 * This function takes a string and a delimiter, and returns the next token
 * from the string. The `save_ptr` argument is used to keep track of the next
 * token. This allows the caller to tokenize multiple strings at the same time.
 *
 * Return: The next available token, or NULL if there are no more tokens.
 */
char *_strtok_r(char *string, char *delim, char **save_ptr)
{
	char *finish;

	if (string == NULL)
		string = *save_ptr;

	if (*string == '\0')
	{
		*save_ptr = string;
		return (NULL);
	}

	string += _strspn(string, delim);
	if (*string == '\0')
	{
		*save_ptr = string;
		return (NULL);
	}

	finish = string + _strcspn(string, delim);
	if (*finish == '\0')
	{
		*save_ptr = finish;
		return (string);
	}

	*finish = '\0';
	*save_ptr = finish + 1;
	return (string);
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
			negative = 1;

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
 * _realloc - reallocates a memory block
 * @ptr: pointer to the memory previously allocated with a call to malloc
 * @old_size: size of ptr
 * @new_size: size of the new memory to be allocated
 *
 * Return: pointer to the address of the new memory block
 */
void *_realloc(void *ptr, unsigned int old_size, unsigned int new_size)
{
	void *temp_block;
	unsigned int i;

	if (ptr == NULL)
	{
		temp_block = malloc(new_size);
		return (temp_block);
	}
	else if (new_size == old_size)
		return (ptr);
	else if (new_size == 0 && ptr != NULL)
	{
		free(ptr);
		return (NULL);
	}
	else
	{
		temp_block = malloc(new_size);
		if (temp_block != NULL)
		{
			for (i = 0; i < minimum(old_size, new_size); i++)
				*((char *)temp_block + i) = *((char *)ptr + i);
			free(ptr);
			return (temp_block);
		}
		else
			return (NULL);

	}
}

/**
 * ctrl_c_handler - handles the signal raised by CTRL-C
 * @signum: signal number
 *
 * Return: void
 */
void ctrl_c_handler(int signum)
{
	if (signum == SIGINT)
		display("\n($) ", STDIN_FILENO);
}

/**
 * remove_comment - removes/ignores everything after a '#' char
 * @input: input to be used
 *
 * Return: void
 */
void remove_comment(char *input)
{
	int i = 0;

	if (input[i] == '#')
		input[i] = '\0';
	while (input[i] != '\0')
	{
		if (input[i] == '#' && input[i - 1] == ' ')
			break;
		i++;
	}
	input[i] = '\0';
}

/**
 *_strcmp - compare two strings
 *@first: first string to be compared
 *@second: second string to be compared
 *
 * Return: difference of the two strings
 */

int _strcmp(char *first, char *second)
{
	int i = 0;

	while (first[i] != '\0')
	{
		if (first[i] != second[i])
			break;
		i++;
	}
	return (first[i] - second[i]);
}

/**
 * _strcat - Concatenates two strings
 *
 * @destination: A pointer to the string to be concatenated
 * @source: A pointer to the string to concatenate
 *
 * Return: A pointer to the resulting concatenated string
 */
char *_strcat(char *destination, char *source)
{
	char *new_string =  NULL;
	int len_dest = _strlen(destination);
	int len_source = _strlen(source);

	new_string = malloc(sizeof(*new_string) * (len_dest + len_source + 1));
	_strcpy(destination, new_string);
	_strcpy(source, new_string + len_dest);
	new_string[len_dest + len_source] = '\0';
	return (new_string);
}

/**
 *_strspn - gets the length of a prefix substring
 *@str1: string to be searched
 *@str2: string to be used
 *
 *Return: number of bytes in the initial segment of 5 which are part of accept
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
	return (match);
}

/**
 *_strcspn - computes segment of str1 which consists of characters not in str2
 *@str1: string to be searched
 *@str2: string to be used
 *
 *Return: index at which a char in str1 exists in str2
 */


int _strcspn(char *str1, char *str2)
{
	int len = 0, i;

	for (i = 0; str1[i] != '\0'; i++)
	{
		if (_strchr(str2, str1[i]) != NULL)
			break;
		len++;
	}
	return (len);
}

/**
 * _strchr - Locates a character in a string
 * @s: The string to be searched
 * @c: The character to be checked
 *
 * Return: A pointer to the first occurrence of c in s, or NULL if c is not found
 */
char *_strchr(char *s, char c)
{
	int i = 0; /* Counter for iterating over string s */

	/* Iterate over s until the end of the string or until the character c is found */
	for (; s[i] != c && s[i] != '\0'; i++)
		;

	/* If c is found in s, return a pointer to the location in s where c was found */
	if (s[i] == c)
		return (s + i);
	else /* Otherwise, return NULL to indicate that c was not found in s */
		return (NULL);
}


/**
 * tokenizer - tokenizes input and stores it into an array
 *@input_string: input to be parsed
 *@delim: delimiter to be used, needs to be one character string
 *
 *Return: array of tokens
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
 *display - displays a string to stdout
 *@string: string to be displayed
 *@stream: stream to display out to
 *
 *Return: void, return nothing
 */
void display(char *string, int stream)
{
	int i = 0;

	for (; string[i] != '\0'; i++)
		write(stream, &string[i], 1);
}

/**
 *_line_rm - removes new line from a string
 *@str: string to be used
 *
 *
 *Return: void
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
 *_strcpy - copies a string to another buffer
 *@source: source to copy from
 *@dest: destination to copy to
 *
 * Return: void
 */

void _strcpy(char *source, char *dest)
{
	int i = 0;

	for (; source[i] != '\0'; i++)
		dest[i] = source[i];
	dest[i] = '\0';
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
 *env - displays the current_environnement
 *@tokenized_command: command entered
 *
 *Return: void
 */

void env(char **tokenized_command __attribute__((unused)))
{
	int i;

	for (i = 0; environ[i] != NULL; i++)
	{
		display(environ[i], STDOUT_FILENO);
		display("\n", STDOUT_FILENO);
	}
}

/**
 * quit - exits the shell
 * @tokenized_command: command entered
 *
 * Return: void
 */

void quit(char **tokenized_command)
{
	int num_token = 0, arg;

	for (; tokenized_command[num_token] != NULL; num_token++)
		;
	if (num_token == 1)
	{
		free(tokenized_command);
		free(line);
		free(commands);
		exit(status);
	}
	else if (num_token == 2)
	{
		arg = _atoi(tokenized_command[1]);
		if (arg == -1)
		{
			display(shell_name, STDERR_FILENO);
			display(": 1: exit: Illegal number: ", STDERR_FILENO);
			display(tokenized_command[1], STDERR_FILENO);
			display("\n", STDERR_FILENO);
			status = 2;
		}
		else
		{
			free(line);
			free(tokenized_command);
			free(commands);
			exit(arg);
		}
	}
	else
		display("$: exit doesn't take more than one argument\n", STDERR_FILENO);
}

/** parse_command - determines the type of the command
 * @command: command to be parsed
 *
 * Return: constant representing the type of the command
 */

int parse_command(char *command)
{
	int i;
	char *internal_command[] = {"env", "exit", NULL};
	char *path = NULL;

	for (i = 0; command[i] != '\0'; i++)
	{
		if (command[i] == '/')
			return (EXTERNAL_COMMAND);
	}
	for (i = 0; internal_command[i] != NULL; i++)
	{
		if (_strcmp(command, internal_command[i]) == 0)
			return (INTERNAL_COMMAND);
	}
	/* @confirm_loc - checks if a command is found in the PATH */
	path = confirm_loc(command);
	if (path != NULL)
	{
		free(path);
		return (PATH_COMMAND);
	}

	return (INVALID_COMMAND);
}

/**
 * execute_command - executes a command based on it's type
 * @tokenized_command: tokenized form of the command (ls -l == {ls, -l, NULL})
 * @command_type: type of the command
 *
 * Return: void
 */
void execute_command(char **tokenized_command, int command_type)
{
	void (*func)(char **command);

	if (command_type == EXTERNAL_COMMAND)
	{
		if (execve(tokenized_command[0], tokenized_command, NULL) == -1)
		{
			perror(_getenv("PWD"));
			exit(2);
		}
	}
	if (command_type == PATH_COMMAND)
	{
		if (execve(confirm_loc(tokenized_command[0]), tokenized_command, NULL) == -1)
		{
			perror(_getenv("PWD"));
			exit(2);
		}
	}
	if (command_type == INTERNAL_COMMAND)
	{
		func = get_func(tokenized_command[0]);
		func(tokenized_command);
	}
	if (command_type == INVALID_COMMAND)
	{
		display(shell_name, STDERR_FILENO);
		display(": 1: ", STDERR_FILENO);
		display(tokenized_command[0], STDERR_FILENO);
		display(": not found\n", STDERR_FILENO);
		status = 127;
	}
}

/**
 * confirm_loc - checks if a command is found in the PATH
 * @command: command to be used
 *
 * Return: path where the command is found in, NULL if not found
 */
char *confirm_loc(char *command)
{
	char **path_array = NULL;
	char *temp, *temp2, *path_cpy;
	char *path = _getenv("PATH");
	int i;

	if (path == NULL || _strlen(path) == 0)
		return (NULL);
	path_cpy = malloc(sizeof(*path_cpy) * (_strlen(path) + 1));
	_strcpy(path, path_cpy);
	path_array = tokenizer(path_cpy, ":");
	for (i = 0; path_array[i] != NULL; i++)
	{
		temp2 = _strcat(path_array[i], "/");
		temp = _strcat(temp2, command);
		if (access(temp, F_OK) == 0)
		{
			free(temp2);
			free(path_array);
			free(path_cpy);
			return (temp);
		}
		free(temp);
		free(temp2);
	}
	free(path_cpy);
	free(path_array);
	return (NULL);
}
/**
 * add - Adds two integers together
 * @a: The first integer to add
 * @b: The second integer to add
 *
 * Return: The sum of a and b
 */
int add(int a, int b)
{
    return a + b;
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
 * get_func - retrieves a function based on the command given and a mapping
 * @command: string to check against the mapping
 *
 * Return: pointer to the proper function, or null on fail
 */
void (*get_func(char *command))(char **)
{
	int i;
	mapping_func mapping[] = {
		{"env", env}, {"exit", quit}
	};

	for (i = 0; i < 2; i++)
	{
		if (_strcmp(command, mapping[i].command_name) == 0)
			return (mapping[i].func);
	}
	return (NULL);
}

/**
 * _getenv - gets the value of an environment variable
 * @name: name of the environment variable
 * Return: the value of the variable as a string
 */
char *_getenv(char *name)
{
	char **my_environ;
	char *pair_ptr;
	char *name_cpy;

	for (my_environ = environ; *my_environ != NULL; my_environ++)
	{
		for (pair_ptr = *my_environ, name_cpy = name;
		     *pair_ptr == *name_cpy; pair_ptr++, name_cpy++)
		{
			if (*pair_ptr == '=')
				break;
		}
		if ((*pair_ptr == '=') && (*name_cpy == '\0'))
			return (pair_ptr + 1);
	}
	return (NULL);
}