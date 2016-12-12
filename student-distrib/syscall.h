/*
* @Author: Jack, Saurav, Charlie, Rahul
* @Date:   2016-10-15 17:17:13
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-15 17:17:13
*/

#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "types.h"
#include "paging.h"
#include "lib.h"
#include "file_sys.h"
#include "rtc.h"
#include "terminal.h"
#include "x86_desc.h"
#include "keyboard.h"

#define V_PAGE 0x08000000 
#define V_ADDR 0x08048000 //Where the program image is set to execute
#define MG_1 0x7f
#define MG_2 0x45
#define MG_3 0x4c
#define MG_4 0x46
#define MNUM_SIZE 4
#define MNUM_OFFSET 24
#define NO_PROCESSES 0xFE
#define EIGHT_MB 0x800000
#define EIGHT_KB 0x2000
#define NUM_FILES 8
#define FNAME_SIZE 32
#define STDOUT 2
#define VIDEO_FLAGS 0x7 /* User, read/write, present */
#define USER_VMEM 0x8400000

extern uint8_t tasks_bitmap;

int32_t sys_halt(uint8_t status);
int32_t sys_execute(const uint8_t* command);
int32_t sys_fork (void);
int32_t load_program(int pd, void * p_addr, void * v_addr, uint8_t * file_name);
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t sys_open(const uint8_t* filename);
int32_t sys_close(int32_t fd);
int32_t sys_getargs(uint8_t* buf, int32_t nbytes);
int32_t sys_vidmap (uint8_t** screen_start);
int32_t sys_set_handler(int32_t signum, void* handler_address);
int32_t sys_sigreturn (void);

#endif /* _SYSCALL_H */
