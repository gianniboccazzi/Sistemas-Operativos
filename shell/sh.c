#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"
#include <signal.h>

char prompt[PRMTLEN] = { 0 };

void sigchld_handler(int signum);

void
sigchld_handler(int signum)
{
	if (signum != SIGCHLD) {
		return;
	}

	int status;
	pid_t pid;
	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		printf("==> terminado: PID=%d\n", pid);
	}
}

// runs a shell command
static void
run_shell()
{
	char *cmd;

	struct sigaction sa = { 0 };
	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGCHLD, &sa, NULL);


	while ((cmd = read_line(prompt)) != NULL) {
		if (run_cmd(cmd) == EXIT_SHELL) {
			return;
		}
	}
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

int
main(void)
{
	init_shell();

	run_shell();

	return 0;
}
