/*
 * File: main.c
 * Auth: Gbenga Elegbede
 *       Ebenezer Sam-Oladapo
 */
#include "shell.h"
#include <errno.h>

void sig_handler(int sig);
int execute(char **args, char **front);

/**
 * sig_handler - Prints a new prompt upon a signal.
 * @sig: The signal.
 */
void sig_handler(int sig)
{
	char *new_prompt = "\n$ ";
	(void)sig;
	signal(SIGINT, sig_handler);
	write(STDIN_FILENO, new_prompt, 3);
}
/**
 * execute - Executes a command in a child process.
 * @args: An array of arguments.
 * @front: A double pointer to the beginning of args.
 *
 * Return: If an error occurs - a corresponding error code.
 *         O/w - The exit value of the last executed command.
 */
int execute(char **args, char **front)
{
	pid_t child_pid;
	int status, flag = 0, ret = 0;
	char *command = args[0];
	if (command[0] != '/' && command[0] != '.')
	{
		flag = 1;
		command = get_location(command);
	}
	child_pid = fork();
	if (child_pid == -1)
	{
		if (flag)
			free(command);
		perror("Error child:");
		return (1);
	}
	if (child_pid == 0)
	{
		if (!command || (access(command, F_OK) == -1))
		{
			if (errno == EACCES)
				ret = (create_error(args, 126));
			else
				ret = (create_error(args, 127));
			free_env();
			free_args(args, front);
			_exit(ret);
		}
	/*
	*	if (access(command, X_OK) == -1)
	*		return (create_error(argv[0], 126));
	*/
		execve(command, args, NULL);
		if (errno == EACCES)
			ret = (create_error(args, 126));
		free_env();
		free_args(args, front);
		_exit(ret);
	}
	else
	{
		wait(&status);
		ret = WEXITSTATUS(status);
	}
	if (flag)
		free(command);
	return (ret);
}
/**
 * main - Runs a simple UNIX command interpreter.
 * @argc: The number of arguments supplied to the program.
 * @argv: An array of pointers to the arguments.
 *
 * Return: The return value of the last executed command.
 */
int main(int argc, char *argv[])
{
	int ret = 0, retn;
	int *exe_ret = &retn;

	name = argv[0];
	hist = 1;

	signal(SIGINT, sig_handler);

	*exe_ret = 0;
	environ = _copyenv();
	if (!environ)
		exit(-100);

	if (argc != 1)
	{
		ret = execute(argv + 1, argv + 1);
		free_env();
		return (ret);
	}

	if (!isatty(STDIN_FILENO))
	{
		while (ret != END_OF_FILE && ret != EXIT)
			ret = handle_args(exe_ret);
		free_env();
		return (*exe_ret);
	}

	while (1)
	{
		printf("$ ");
		ret = handle_args(exe_ret);
		if (ret == END_OF_FILE || ret == EXIT)
		{
			if (ret == END_OF_FILE)
				printf("\n");
			free_env();
			exit(*exe_ret);
		}
	}
	free_env();
	return (*exe_ret);
}
