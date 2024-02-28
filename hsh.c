#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 1024

/**
 * _getline - custom getline function
 * @buffer: buffer to store the line
 * @size: size of the buffer
 * @stream: file stream to read from
 *
 * Return: number of bytes read, 0 on EOF, -1 on failure
 */
ssize_t _getline(char **buffer, size_t *size, FILE *stream)
{
    size_t i = 0;
    int c;

    if (!buffer || !size || !stream)
        return -1;

    if (*buffer == NULL || *size == 0)
    {
        *size = BUFFER_SIZE;
        *buffer = malloc(*size);
        if (*buffer == NULL)
            return -1;
    }

    while ((c = getc(stream)) != EOF)
    {
        if (i >= *size - 1)
        {
            *size *= 2;
            *buffer = realloc(*buffer, *size);
            if (*buffer == NULL)
                return -1;
        }
        (*buffer)[i++] = c;
        if (c == '\n')
            break;
    }

    if (i == 0 && c == EOF)
        return 0;

    (*buffer)[i] = '\0';
    return i;
}

/**
 * tokenize - tokenize a string
 * @line: input string
 * @delim: delimiter
 *
 * Return: array of tokens
 */
char **tokenize(char *line, const char *delim)
{
    int i = 0;
    char **tokens = malloc(sizeof(char *) * (BUFFER_SIZE / 2));
    char *token;

    if (!tokens)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, delim);
    while (token)
    {
        tokens[i++] = token;
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL;

    return tokens;
}

/**
 * execute_command - execute a command
 * @args: arguments to the command
 */
void execute_command(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        if (execve(args[0], args, NULL) == -1)
        {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        do
        {
            if (waitpid(pid, &status, WUNTRACED) == -1)
            {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

int main(int argc, char **argv)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char **args;

    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (isatty(STDIN_FILENO))
        printf("($) ");

    while ((read = _getline(&line, &len, stdin)) != -1)
    {
        if (read > 0 && line[read - 1] == '\n')
            line[read - 1] = '\0';

        args = tokenize(line, " ");
        if (args[0] != NULL)
        {
            if (strcmp(args[0], "exit") == 0)
            {
                free(line);
                free(args);
                exit(EXIT_SUCCESS);
            }
            execute_command(args);
        }

        if (isatty(STDIN_FILENO))
            printf("($) ");
        free(args);
    }

    free(line);
    return EXIT_SUCCESS;
}
