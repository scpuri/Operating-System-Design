/*
* asm_handler.h - header file for asm_handler.S
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-14 14:43:00
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-16 20:43:00
*/

#ifndef _ASM_HANDLER_H
#define _ASM_HANDLER_H

#include "types.h"

#ifndef ASM

/* Function declarations */
extern void asm_rtc_handler(void);
extern void asm_keyboard_handler(void);
extern void asm_int_ignore(void);
extern void asm_timer_handler(void);

#endif

#endif
