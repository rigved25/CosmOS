#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{

	//cprintf("CPU %d inside sched_yield()\n", cpunum());
	struct Env *idle = curenv;
    int idle_envid = (idle == NULL) ? -1 : ENVX(idle->env_id);
    int i;

    // search envs after idle
    for (i = idle_envid + 1; i < NENV; i++) {
        if (envs[i].env_status == ENV_RUNNABLE) {
            env_run(&envs[i]);
        }
    }

    // find from 1st env if not found
    for (i = 0; i < idle_envid; i++) {;
        if (envs[i].env_status == ENV_RUNNABLE) {
            env_run(&envs[i]);
        }
    }

    // if still not found, try idle
    if(idle != NULL && idle->env_status == ENV_RUNNING) {
        env_run(idle);
    }

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

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile (
		"movl $0, %%ebp\n"
		"movl %0, %%esp\n"
		"pushl $0\n"
		"pushl $0\n"
        // LAB 4:
		//Uncomment the following line after completing exercise 13
		"sti\n"
		"1:\n"
		"hlt\n"
		"jmp 1b\n"
	: : "a" (thiscpu->cpu_ts.ts_esp0));
}

