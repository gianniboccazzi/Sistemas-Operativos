#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	// Your code here
	for (int i = 0; i < eargc; i++) {
		char *clave = strdup(eargv[i]);
		char *valor = strdup(eargv[i]);
		char *idx = strdup(eargv[i]);
		get_environ_key(clave, clave);
		get_environ_value(valor, valor, block_contains(idx, '='));
		setenv(clave, valor, WRITE);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd;

	if (flags & O_CREAT) {
		fd = open(file, flags, S_IWUSR | S_IRUSR);
	} else {
		fd = open(file, flags);
	}
	if (fd < 0) {
		perror("Error en open_redir_fd");
		_exit(-1);
	}
	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC: {
		e = (struct execcmd *) cmd;
		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);
		perror("execvp");
		_exit(-1);
		break;
	}


	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;
		e = (struct execcmd *) b->c;
		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);
		perror("execvp");
		_exit(-1);
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		r = (struct execcmd *) cmd;

		set_environ_vars(r->eargv, r->eargc);
		if (r->out_file[0] != '\0') {
			int fd_out = open_redir_fd(r->out_file,
			                           O_TRUNC | O_CREAT | O_RDWR);
			int new_out = dup2(fd_out, STDOUT_FILENO);
			close(fd_out);
			if (new_out < 0) {
				perror("Error en dup2 out");
				_exit(-1);
			}
		}

		if (r->in_file[0] != '\0') {
			int fd_in = open_redir_fd(r->in_file, O_RDONLY);
			int new_in = dup2(fd_in, STDIN_FILENO);
			close(fd_in);
			if (new_in < 0) {
				perror("Error en dup2 in");
				_exit(-1);
			}
		}

		if (r->err_file[0] != '\0') {
			if (strcmp(r->err_file, "&1") == 0) {
				dup2(STDOUT_FILENO, STDERR_FILENO);
			} else {
				int fd_err = open_redir_fd(
				        r->err_file, O_CREAT | O_RDWR | O_TRUNC);
				int new_err = dup2(fd_err, STDERR_FILENO);
				close(fd_err);
				if (new_err < 0) {
					perror("Error en dup2 in");
					_exit(-1);
				}
			}
		}
		execvp(r->argv[0], r->argv);
		perror("execvp");
		_exit(-1);
		break;
	}

	case PIPE: {
		// pipes two commands
		//
		// Your code here
		p = (struct pipecmd *) cmd;
		int pipe_fds[2];
		if (pipe(pipe_fds) < 0) {
			printf("Error en pipe\n");
			_exit(-1);
		}
		pid_t hijo_izq = fork();
		if (hijo_izq < 0) {
			perror("Error en fork\n");
			_exit(-1);
		}
		if (hijo_izq == 0) {
			setpgid(0, 0);
			dup2(pipe_fds[WRITE], STDOUT_FILENO);
			close(pipe_fds[WRITE]);
			close(pipe_fds[READ]);
			exec_cmd(p->leftcmd);
		} else {
			pid_t hijo_der = fork();
			if (hijo_der < 0) {
				perror("Error en fork\n");
				exit(-1);
			}
			if (hijo_der == 0) {
				setpgid(0, 0);
				dup2(pipe_fds[READ], STDIN_FILENO);
				close(pipe_fds[WRITE]);
				close(pipe_fds[READ]);
				exec_cmd(p->rightcmd);
			} else {
				close(pipe_fds[WRITE]);
				close(pipe_fds[READ]);
				waitpid(hijo_izq, NULL, 0);
				waitpid(hijo_der, NULL, 0);
				free_command(parsed_pipe);
				exit(0);
			}
		}
	}

		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);
		break;
	}
}
