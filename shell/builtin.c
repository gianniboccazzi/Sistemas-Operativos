#include "builtin.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	// Your code here
	if (strcmp(cmd, "exit") == 0) {
		return 1;
	}
	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	// Your code here
	if (cmd[0] != 'c' || cmd[1] != 'd') {
		return 0;
	}
	if (cmd[2] == '\0' || (cmd[2] == ' ' && strlen(cmd) == 4)) {
		char *home = getenv("HOME");
		if (chdir(home) < 0) {
			exit(-1);
		}
		snprintf(prompt, sizeof prompt, "(%s)", home);
		return 1;
	}
	if (cmd[2] == ' ' && strlen(cmd) > 4) {
		char *izq = strdup(cmd);
		char *der = split_line(izq, ' ');
		if (chdir(der) < 0) {
			printf("cannot cd to %s ", der);
			return 0;
		} else {
			char *path_actual = NULL;
			size_t size = 1024;
			path_actual = (char *) malloc(size);
			if (!getcwd(path_actual, size)) {
				exit(-1);
			}
			snprintf(prompt, sizeof prompt, "(%s)", path_actual);
			free(path_actual);
		}
		return 1;
	}
	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") != 0) {
		return 0;
	}
	// Your code here
	char *path_actual = NULL;
	size_t size = 1024;
	path_actual = (char *) malloc(size);
	if (getcwd(path_actual, size) != NULL) {
		printf("%s\n", path_actual);
		return 1;
	} else {
		perror("Error al obtener el directorio actual");
	}
	free(path_actual);
	return 0;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
