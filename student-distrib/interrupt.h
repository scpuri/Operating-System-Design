/*
* interrupt.h - header file for interrupt.c
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-14 14:43:00
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-14 14:43:00
*/

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "exceptions.h"
#include "keyboard.h"
#include "rtc.h"
#include "asm_handler.h"
#include "asm_syscall.h"
#include "i8259.h"

#define USER_PRIVILEGE 3
#define KERNEL_PRIVILEGE 0
#define SYSCALL 0x80
#define NUM_RESERVED 32

/* Interupt handlers */
#define KEYBOARD 0x21
#define RTC 	 0x28
#define TIMER    0x20
#define TIMER_IRQ 0

/* Function primitives */
extern void init_idt(void);
void int_ignore(void);

#endif
