/*
* asm_handler.S - assembly wrapper functions for interrupt service routines
* 				  and paging helper functions
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-14 14:43:00
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-16 23:22:23
*/

#define ASM 	1

#include "asm_handler.h"

.text

.globl asm_rtc_handler, asm_keyboard_handler, asm_int_ignore, asm_timer_handler

.align SIZEOF_LONG

/* 
 * asm_rtc_handler
 *   DESCRIPTION: Mask interrupts, save all regs, call the handler,
 *				  restore the regs, unmask interrupts, and iret
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
asm_rtc_handler:
	cli

/* Save all registers */
	pushl %es
	pushl %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx

/* Call the C part of the handler */
	call rtc_handler

/* Restore all registers */
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	popl %ds
	popl %es

/* Reenable interupts and iret */
	sti
	iret

/* 
 * asm_keyboard_handler
 *   DESCRIPTION: Mask interrupts, save all regs, call the handler,
 *				  restore the regs, unmask interrupts, and iret
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
asm_keyboard_handler:
	cli

/* Save all registers */
	pushl %es
	pushl %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx

/* Call the C part of the handler */
	call keyboard_handler

/* Restore all registers */
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	popl %ds
	popl %es

/* Reenable interupts and iret */
	sti
	iret

/* 
 * asm_int_ignore
 *   DESCRIPTION: Mask interrupts, save all regs, call the handler,
 *				  restore the regs, unmask interrupts, and iret
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
asm_int_ignore:
	cli

/* Save all registers */
	pushl %es
	pushl %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx

/* Call the C part of the handler */
	call int_ignore

/* Restore all registers */
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	popl %ds
	popl %es

/* Reenable interupts and iret */
	sti
	iret

/* 
 * asm_timer_ignore
 *   DESCRIPTION: Mask interrupts, save all regs, call the handler,
 *				  restore the regs, unmask interrupts, and iret
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
asm_timer_handler:
	cli

/* Save all registers */
	pushl %es
	pushl %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx

	pushl %esp

/* Call the C part of the handler */
	call irq_timer

/* The return value of irq_timer is the new esp that should be used */
	movl %eax, %esp

/* Restore all registers */
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	popl %ds
	popl %es

/* Reenable interupts and iret */
	sti
	iret

/* We'll never get back here, but we put in a hlt anyway. */
halt:
	hlt
	jmp     halt


	


