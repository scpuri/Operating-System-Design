/*
* @Author: Jack
* @Date:   2016-10-15 17:17:13
* @Last Modified by:   Jack
* @Last Modified time: 2016-12-03 21:44:10
*/

#include "exceptions.h"

/* 
 * divide_error
 *   DESCRIPTION: handle Divide-by-zero exception #0
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void divide_error(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Divide-by-zero exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * debug
 *   DESCRIPTION: handle Debug exception #1
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void debug(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Debug exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * nmi
 *   DESCRIPTION: handle NMI exception #2
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void nmi(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("NMI exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * int3
 *   DESCRIPTION: handle Breakpoint exception #3
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void int3(void)
{	
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Breakpoint exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * overflow
 *   DESCRIPTION: handle Overflow exception #4
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void overflow(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Overflow exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * bounds
 *   DESCRIPTION: handle Bounds check exception #5
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void bounds(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Bounds check exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * invalid_op
 *   DESCRIPTION: handle Invalid Opcode exception #6
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void invalid_op(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Invalid opcode exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * device_not_available
 *   DESCRIPTION: handle Device not available exception #7
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void device_not_available(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Device not available exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * doublefault_fn
 *   DESCRIPTION: handle Double fault exception #8
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void doublefault_fn(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Double fault exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * coprocessor_segment_overrun
 *   DESCRIPTION: handle Coprocessor segment overrun exception #9
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void coprocessor_segment_overrun(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Coprocessor segment overrun exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * invalid_TSS
 *   DESCRIPTION: handle Invalid TSS exception #10
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void invalid_TSS(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Invalid TSS exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * segment_not_present
 *   DESCRIPTION: handle Segment not present exception #11
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void segment_not_present(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Segment not present exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * stack_segment
 *   DESCRIPTION: handle Stack segment fault exception #12
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void stack_segment(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Stack segment fault exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * general_protection
 *   DESCRIPTION: handle General protection exception #13
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void general_protection(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("General protection exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * page_fault
 *   DESCRIPTION: handle Page fault exception #14
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void page_fault(void)
{
	/* Mask interrupts and clear the screen */
	int err_code;

	cli();
	clear();
	/* Print the error message */
	asm volatile(
		 "movl %%cr2, %%ebx"
		:"=b"(err_code)
	);
	printf("Page fault exception by address: %x\n", err_code);
	sys_halt(0);
	while(1);
}

/* 
 * coprocessor_error
 *   DESCRIPTION: handle Floating-point error exception #16
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void coprocessor_error(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Floating-point exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * alignment_check
 *   DESCRIPTION: handle Alignment check exception #17
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void alignment_check(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("Alignment check exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * machine_check
 *   DESCRIPTION: handle Machine check exception #18
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void machine_check(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();	
	/* Print the error message */
	printf("Machine check exception\n");
	sys_halt(0);
	while(1);
}

/* 
 * simd_coprocessor_error
 *   DESCRIPTION: handle SIMD Floating point exception #19
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void simd_coprocessor_error(void)
{
	/* Mask interrupts and clear the screen */
	cli();
	clear();
	/* Print the error message */
	printf("SIMD Floating-point exception\n");
	sys_halt(0);
	while(1);
}

