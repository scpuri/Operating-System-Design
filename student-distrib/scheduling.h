/*
* @Author: Jack, Saurav, Charlie, Rahul
* @Date:   2016-10-27 17:17:13
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-27 17:17:13
*/

#ifndef _SCHEDULING_H
#define _SCHEDULING_H

#include "types.h"
#include "lib.h"
#include "interrupt.h"
#include "i8259.h"
#include "paging.h"
#include "syscall.h"
#include "keyboard.h"

#define NUM_TERMS 3
#define SCHEDULING_RATE 60
#define BASECLOCKRATE 1193180
#define PIT_DPORT 0x40
#define PIT_CPORT 0x43
#define PIT_MODE3 0x36
#define BYTE_SHIFT 8
#define LSB_BYTE 0xFF
#define EBP_INDEX 5

/* Local functions */
void init_timer(void);
uint32_t irq_timer(uint32_t* esp);
void set_rate(int hz);
void set_first_flag(int flag);

extern int saved_x[NUM_TERMS];
extern int saved_y[NUM_TERMS];

#endif /* _SCHEDULING_H */

