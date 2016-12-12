/*
* @Author: Jack
* @Date:   2016-10-15 17:17:13
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-15 17:17:13
*/

#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "lib.h"
#include "syscall.h"

#define DIVIDE_ERROR 0
#define DEBUG 1
#define NMI 2
#define INT3 3
#define OVERFLOW 4
#define BOUNDS 5
#define INVALID_OP 6
#define DEVICE_NOT_AVAILABLE 7
#define DOUBLEFAULT_FN 8
#define COPROCESSOR_SEGMENT_OVERRUN 9
#define INVALID_TSS 10
#define SEGMENT_NOT_PRESENT 11
#define STACK_SEGMENT 12
#define GENERAL_PROTECTION 13
#define PAGE_FAULT 14
#define COPROCESSOR_ERROR 16
#define ALIGNMENT_CHECK 17
#define MACHINE_CHECK 18
#define SIMD_COPROCESSOR_ERROR 19

/* Exception handlers placed in the IDT by interrupt.c */
extern void divide_error(void);
extern void debug(void);
extern void nmi(void);
extern void int3(void);
extern void overflow(void);
extern void bounds(void);
extern void invalid_op(void);
extern void device_not_available(void);
extern void doublefault_fn(void);
extern void coprocessor_segment_overrun(void);
extern void invalid_TSS(void);
extern void segment_not_present(void);
extern void stack_segment(void);
extern void general_protection(void);
extern void page_fault(void);
extern void coprocessor_error(void);
extern void alignment_check(void);
extern void machine_check(void);
extern void simd_coprocessor_error(void);

#endif
