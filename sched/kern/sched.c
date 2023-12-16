#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>


void sched_halt(void);
void round_robin(void);
void lottery(void);

#define HIST_MAX_SIZE 10000
typedef struct hist_entry {
	envid_t env_id;
	int env_tickets;
	int env_runs;

} hist_entry_t;

hist_entry_t historial[HIST_MAX_SIZE];
int ultima_posicion = 0;
int max_historial_indx = 0;

static unsigned random_seed = 1;

#define RANDOM_MAX total_tickets
unsigned
lcg_parkmiller(unsigned *state)
{
	const unsigned N = 0x7fffffff;
	const unsigned G = 48271u;

	/*
	    Indirectly compute state*G%N.

	    Let:
	      div = state/(N/G)
	      rem = state%(N/G)

	    Then:
	      rem + div*(N/G) == state
	      rem*G + div*(N/G)*G == state*G

	    Now:
	      div*(N/G)*G == div*(N - N%G) === -div*(N%G)  (mod N)

	    Therefore:
	      rem*G - div*(N%G) === state*G  (mod N)

	    Add N if necessary so that the result is between 1 and N-1.
	*/
	unsigned div =
	        *state / (N / G); /* max : 2,147,483,646 / 44,488 = 48,271 */
	unsigned rem =
	        *state % (N / G); /* max : 2,147,483,646 % 44,488 = 44,487 */

	unsigned a = rem * G;       /* max : 44,487 * 48,271 = 2,147,431,977 */
	unsigned b = div * (N % G); /* max : 48,271 * 3,399 = 164,073,129 */

	return *state = (a > b) ? (a - b) : (a + (N - b));
}

unsigned
next_random()
{
	return lcg_parkmiller(&random_seed);
}

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *idle;

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here
	// Wihtout scheduler, keep runing the last environment while it exists

	// #define SCHED_RR
	// #define SCHED_LP

	// #ifdef SCHED_RR
	// round_robin();
	// #endif
	// 	// sched_halt never returns
	// 	sched_halt();
	// #ifdef SCHED_LP
	lottery();

	// #endif
	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;
	//  cprintf("sched_halt - entering\n");

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		cprintf("Historial de ejecucion:\n");
		for (int i = 0; i < max_historial_indx; i++) {
			cprintf("env_id: %d, env_tickets: %d, env_runs: %d\n",
			        historial[i].env_id,
			        historial[i].env_tickets,
			        historial[i].env_runs);
		}
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here


	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}

void
round_robin(void)
{
	int n = curenv ? (ENVX(curenv->env_id) + 1) : 0;

	for (int i = 0; i < NENV; i++) {
		int idx = (n + i) % NENV;
		if (envs[idx].env_status == ENV_RUNNABLE) {
			env_run(&envs[idx]);
		}
	}
	if (curenv != NULL && curenv->env_status == ENV_RUNNING) {
		env_run(&envs[ENVX(curenv->env_id)]);
	}
}

void
add_to_historial(struct Env *e)
{
	historial[ultima_posicion].env_id = e->env_id;
	historial[ultima_posicion].env_tickets = e->env_tickets;
	historial[ultima_posicion].env_runs = e->env_runs;
	ultima_posicion++;
	if (ultima_posicion > max_historial_indx) {
		max_historial_indx = ultima_posicion;
	}
	if (ultima_posicion == HIST_MAX_SIZE) {
		ultima_posicion = 0;
	}
}

void
lottery(void)
{
	int pool = 0;

	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNING ||
		    envs[i].env_status == ENV_RUNNABLE) {
			pool += envs[i].env_tickets;
		}
	}
	int winner = next_random() % (pool + 1);


	for (int i = 0, j = 0; i < NENV; i++) {
		if (envs[i].env_status != ENV_RUNNING &&
		    envs[i].env_status != ENV_RUNNABLE) {
			continue;
		}
		j += envs[i].env_tickets;
		if (j >= winner) {
			add_to_historial(&envs[i]);
			env_run(&envs[i]);
		}
	}
}
