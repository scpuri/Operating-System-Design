/*
* interrupt.c - initializes the IDT, handles system calls, exceptions
* 				and interrupts
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-14 14:43:00
* @Last Modified by:   Jack
* @Last Modified time: 2016-12-03 21:49:43
*/

#include "interrupt.h"

/* 
 * init_idt
 *   DESCRIPTION: Initialize the IDT via the idt array declared
 * 				  in x86_desc.h
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes value of entries in IDT
 */
void init_idt(void)
{
	int i;		/* Loop counter through idt array */

	/* Set up a template entry in the IDT */
	idt_desc_t idt_entry;
	idt_entry.offset_15_00 = 0;
	idt_entry.seg_selector = KERNEL_CS;
	idt_entry.reserved4 = 0;
	idt_entry.reserved3 = 0;
	idt_entry.reserved2 = 1;
	idt_entry.reserved1 = 1;
	idt_entry.size = 1;
	idt_entry.reserved0 = 0;
	idt_entry.dpl = 0;    
	idt_entry.present = 1;
	idt_entry.offset_31_16 = 0;


	SET_IDT_ENTRY(idt_entry, asm_int_ignore);

	/* Initialize the idt array with template entries */
	for(i = 0; i < NUM_VEC; i++){
		/* The default entry is an interrupt gate */
		idt[i] = idt_entry;
		
		/* If the entry is an exception use trap gate */
		if(i < NUM_RESERVED) idt[i].reserved3 = 1;
	}

	/* In the case of the Double Fault exception use task gate */
	idt[DOUBLEFAULT_FN].reserved2 = 0;
	idt[DOUBLEFAULT_FN].size = 0;
	idt[DOUBLEFAULT_FN].seg_selector = KERNEL_TSS;

	/* The exceptions that may be called from user level 
	   should have their DPL set to 3 */
	idt[INT3].dpl = USER_PRIVILEGE;
	idt[OVERFLOW].dpl = USER_PRIVILEGE;
	idt[BOUNDS].dpl = USER_PRIVILEGE;
	idt[SYSCALL].dpl = USER_PRIVILEGE;

	/* Set up IDT exception entries */
	SET_IDT_ENTRY(idt[DIVIDE_ERROR],divide_error);
	SET_IDT_ENTRY(idt[DEBUG],debug);
	SET_IDT_ENTRY(idt[NMI],nmi);
	SET_IDT_ENTRY(idt[INT3],int3);
	SET_IDT_ENTRY(idt[OVERFLOW],overflow);
	SET_IDT_ENTRY(idt[BOUNDS],bounds);
	SET_IDT_ENTRY(idt[INVALID_OP],invalid_op);
	SET_IDT_ENTRY(idt[DEVICE_NOT_AVAILABLE],device_not_available);
	SET_IDT_ENTRY(idt[DOUBLEFAULT_FN],doublefault_fn);
	SET_IDT_ENTRY(idt[COPROCESSOR_SEGMENT_OVERRUN],coprocessor_segment_overrun);
	SET_IDT_ENTRY(idt[INVALID_TSS],invalid_TSS);
	SET_IDT_ENTRY(idt[SEGMENT_NOT_PRESENT],segment_not_present);
	SET_IDT_ENTRY(idt[STACK_SEGMENT],stack_segment);
	SET_IDT_ENTRY(idt[GENERAL_PROTECTION],general_protection);
	SET_IDT_ENTRY(idt[PAGE_FAULT],page_fault);
	SET_IDT_ENTRY(idt[COPROCESSOR_ERROR],coprocessor_error);
	SET_IDT_ENTRY(idt[ALIGNMENT_CHECK],alignment_check);
	SET_IDT_ENTRY(idt[MACHINE_CHECK],machine_check);
	SET_IDT_ENTRY(idt[SIMD_COPROCESSOR_ERROR],simd_coprocessor_error);

	/* Set other IDT entries with appropriate handler */
	SET_IDT_ENTRY(idt[SYSCALL],syscall_handler);
	SET_IDT_ENTRY(idt[KEYBOARD],asm_keyboard_handler);
	SET_IDT_ENTRY(idt[RTC],asm_rtc_handler);
	SET_IDT_ENTRY(idt[TIMER],asm_timer_handler);
}

/* 
 * int_ignore
 *   DESCRIPTION: Default handler for IDT entry, outputs
 *				  "unknown interrupt" to console and irets
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints Unknown Interrupt message
 */
void int_ignore(void)
{
	printf("Unknown Interrupt\n");
}

