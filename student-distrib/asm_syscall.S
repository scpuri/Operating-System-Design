/*
* asm_syscall.S - assembly wrapper for system calls via INT $0x80
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-26 14:43:00
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-24 23:22:23
*/

#define ASM 	1

#include "asm_syscall.h"

.text

.globl syscall_handler

.align SIZEOF_LONG

syscall_handler:
	/* Save all registers & place parameters on stack */
		pushl %es
		pushl %ds
		pushl %ebp
		pushl %edi
		pushl %esi
		pushl %edx
		pushl %ecx
		pushl %ebx

	/* Check validity of syscall number (EAX) */
		addl $-1, %eax
		cmpl $NUM_SYSCALLS, %eax
		ja invalid_syscall

	/* Use jump table to decide the system call type */
		call *syscall_table(,%eax,SIZEOF_LONG)
		jmp syscall_return

invalid_syscall:
		movl $-1, %eax

syscall_return:
	/* Restore all registers and iret */
		popl %ebx
		popl %ecx
		popl %edx
		popl %esi
		popl %edi
		popl %ebp
		popl %ds
		popl %es
		iret

    /* We'll never get back here, but we put in a hlt anyway. */
halt:
	hlt
	jmp     halt


syscall_table:
	.long sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn



