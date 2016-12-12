/*
* @Author: Jack
* @Date:   2016-10-19 19:53:20
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-19 19:53:20
*/

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "lib.h"
#include "keyboard.h"
#include "paging.h"
#include "scheduling.h"

#define BUF_SIZE 128
#define VIDEO 0xB8000
#define NUM_COLS 80
#define NUM_ROWS 25
#define BLOCK_TXT 219

extern fops_t term_file_operations;

void term_init(void);
int32_t term_read(int32_t fd, void* buf, int32_t nbytes);
int32_t term_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t term_open(const uint8_t* filename);
int32_t term_close(int32_t fd);
void term_switch(int new_term);

#endif /* _TERMINAL_H */
