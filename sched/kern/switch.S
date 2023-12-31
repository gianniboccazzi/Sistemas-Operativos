/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

.globl context_switch;
context_switch:
	movl 4(%esp), %eax
	movl %eax, %esp
	popal
	pop %es
	pop %ds
	add $8, %esp
	iret
