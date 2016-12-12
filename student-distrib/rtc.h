/*
* rtc.h - header file for rtc.c
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-14 17:39:00
* @Last Modified by:   Charlie
* @Last Modified time: 2016-10-15 17:39:00
*/

#ifndef _RTC_H
#define _RTC_H

#include "i8259.h"
#include "lib.h"

/* Pre-processor definitions */
#define LOC_OF_RTC    8	   //The port on the PIC
#define REGISTER_PORT 0x70 //Changes selected register and also changes NMI
#define RW_PORT		  0x71 //Reads or writes to the selected register
#define PIE_BIT		  0x40 //Bitmask to change the periodic interrupt enable bit of register B
#define B_NMI_DIS	  0x8B //Write to REGISTER_PORT to select register B and disable NMI
#define REG_A		  0x8A //select register A
#define SELECT_C	  0x0C //Write to REGISTER_PORT to select register C
#define RATE 		  0xF  // F for 2 Hz
#define UNIB		  0xF0 // Mask upper nib of byte
#define DEFAULT_FREQ  2
#define SIZEOF_LONG   4
#define NUM_TERMS	  3

extern fops_t rtc_file_operations;

/* File scope variables */
uint8_t int_has_occurred[NUM_TERMS];

/* Function delcarations */
void rtc_init(void);
void rtc_handler(void);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);
uint8_t freq_to_rate(uint32_t freq);

#endif /* _RTC_H */
