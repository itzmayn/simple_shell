#include <stdio.h>

/**
 * main - Entry point of the program
 *
 * Return: Always 0 (Success)
 */
int main(void)
{
    char message[] = "Hello, world!\n"; // Define the message to be printed
    int i;

    for (i = 0; message[i] != '\0'; i++)
    {
        putchar(message[i]); // Print each character using putchar
    }

    return 0; // Exit the program with a success status
}

