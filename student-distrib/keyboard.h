/*
* @Author: Jack
* @Date:   2016-10-15 15:46:38
* @Last Modified by:   Jack
* @Last Modified time: 2016-10-15 15:49:03
*/

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "i8259.h"
#include "lib.h"
#include "terminal.h"

/* Info for accessing keyboard */
#define DATA_PORT 0x60
#define CMD_PORT  0x64
#define LOC_OF_KEYBOARD 1
#define NUM_KEYS 0xD8
#define BUFFER_SIZE 128
#define TYPED	  0x3D
#define BUF_SIZE 128
#define VIDEO 0xB8000
#define SCREEN_WIDTH 80
#define NUM_TERMS 3

/* Different scancodes */
#define CTL_DOWN   0x1D
#define CTL_UP     0x9D
#define CAPS_DOWN  0x3A
#define SHIFT_DOWN 0x2A
#define SHIFT_UP   0xAA
#define SHIFTR_DOWN 0x36
#define SHIFTR_UP  0xB6
#define L_DOWN 	   0x26
#define BACKSPACE  0x0E
#define ENTER 	   0x1C
#define ALT_DOWN   0x38
#define ALT_UP	   0xB8
#define F1_DOWN    0x3B


void keyboard_init(void);
int key_valid(int keycode);
void keyboard_handler(void);
void keyboard_put(void);
int32_t key_read(int32_t fd, void* buf, int32_t nbytes);
int32_t key_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t key_open(const uint8_t* filename);
int32_t key_close(int32_t fd);
void clear_keyboard(void);
void key_set_screen(int x, int y);
void set_active_term(int new_term);
int get_active_term(void);

#endif /* _KEYBOARD_H */
