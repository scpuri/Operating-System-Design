/*
* asm_syscall.h - header file for asm_syscall.S
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-24 14:43:00
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-24 20:43:00
*/

#ifndef _ASM_SYSCALL_H
#define _ASM_SYSCALL_H

#include "types.h"

#define NUM_SYSCALLS 9

#ifndef ASM

/* Function declarations */
extern void syscall_handler(void);

#endif

#endif


