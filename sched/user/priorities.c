#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int i;

	cprintf("Hello, I am environment %08x, cpu %d\n with %d tickets\n",
	        thisenv->env_id,
	        thisenv->env_cpunum,
	        thisenv->env_tickets);
	if (thisenv->env_id == 4096) {
		for (i = 0; i < 40; i++) {
			sys_env_set_priority();
		}
		cprintf("I am environment %08x and i lost tickets, now i have "
		        "%d tickets\n",
		        thisenv->env_id,
		        thisenv->env_tickets);
	}
	for (i = 0; i < 30; i++) {
		sys_yield();
		cprintf("Back in environment %08x, iteration %d, cpu %d, and "
		        "%d tickets\n",
		        thisenv->env_id,
		        i,
		        thisenv->env_cpunum,
		        thisenv->env_tickets);
	}
	cprintf("All done in environment %08x, cpu %d, %d tickets\n",
	        thisenv->env_id,
	        thisenv->env_cpunum,
	        thisenv->env_tickets);
}
