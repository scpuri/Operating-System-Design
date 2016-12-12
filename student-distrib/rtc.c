/*
* rtc.c - initializes the RTC, handles RTC interrupts
* @Author: Jack Weil, Charles Zega, Rahul Sharma, Saurav Puri
* @Date:   2016-10-14 17:39:00
* @Last Modified by:   Jack
* @Last Modified time: 2016-12-03 22:21:03
*/
#include "rtc.h"

/* File operations table */
fops_t rtc_file_operations = {
	.read = rtc_read,
	.write = rtc_write,
	.open = rtc_open,
	.close = rtc_close
};

/* 
 * rtc_init()
 *   DESCRIPTION: Initialize the rtc by unmasking
 *  			  the interrupt on the PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes value of entries in IDT
 */
void rtc_init(void)
{
	uint8_t regB, regA;
	/* Mask all interrupts while RTC is being initialized */

	/* Change the PIE bit of register B to turn on periodic interrupts */
	outb(B_NMI_DIS, REGISTER_PORT);
	regB = inb(RW_PORT);
	outb(B_NMI_DIS, REGISTER_PORT);
	outb(regB | PIE_BIT, RW_PORT);

	/* Set the rate of the interrupts */
	outb(REG_A, REGISTER_PORT);
	regA = inb(RW_PORT);
	outb(REG_A, REGISTER_PORT);
	outb((regA & UNIB) | RATE, RW_PORT);

	/* Turn interrupts back on, including for the RTC */	
	enable_irq(LOC_OF_RTC);
}


/* 
 * rtc_handler()
 *   DESCRIPTION: Calls test_interrupts and also opens register C
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Will call test_interrupts and open register C
 *				   to ensure that the interrupt will keep happening
 */
void rtc_handler(void) 
{
	/* Port C will hold the info about the interrupt, so check it
	   to ensure that the interrupt will keep happening, atomically */
	int i;
	cli();
	outb(SELECT_C, REGISTER_PORT);
	inb(RW_PORT);

	/* The interrupt is over, set flag atomically and acknowledge it */
	for(i = 0; i < NUM_TERMS; i++){
		int_has_occurred[i] = 0;
	}
	sti();
	send_eoi(LOC_OF_RTC);
}

/* 
 * rtc_read(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: Implements read system call functionality for RTC by waiting
 * 				  until an interrupt has occurred and then returning success
 *   INPUTS: all ignored
 *   OUTPUTS: none
 *   RETURN VALUE: always 0 for success
 *   SIDE EFFECTS: clears the file scope int_has_occurred flag
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
	/* Wait until an interrupt has occured then reset flag and return 0 */
	sti();
	int_has_occurred[get_pcb()->term] = 1;
	while(int_has_occurred[get_pcb()->term]);
	return 0;
}

/* 
 * rtc_write(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: Implements write system call functionality for RTC by setting
 * 				  the frequency of interrupts given by the buf ptr
 *   INPUTS: fd - ignored
 *			 const void* buf - ptr to the 4 byte frequency value
 *			 int32_t nbytes - number of bytes to at buf, must be 4
 *   OUTPUTS: none
 *   RETURN VALUE: -1 for failure, nbytes on success
 *   SIDE EFFECTS: sets the frequency of the RTC
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes)
{
	/* Make sure buf is valid ptr to 4 byte value */
	if(nbytes != SIZEOF_LONG || buf == NULL) return -1;

	/* Byte for saving old/new rate and new freq */
	uint8_t oldRate, newRate;
	uint32_t freq;

	/* Get the current rate and new freq */
	outb(REG_A, REGISTER_PORT);
	oldRate = inb(RW_PORT);
	freq = *((uint32_t*)buf);

	/* Calculate the new rate from the freq and check validity */
	newRate = freq_to_rate(freq);
	if(newRate == 1) return -1;

	/* Set the new rate and return num successful bytes */
	outb(REG_A, REGISTER_PORT);
	outb((oldRate & UNIB) | newRate, RW_PORT);
	return nbytes;
}

/* 
 * rtc_open(const uint8_t filename)
 *   DESCRIPTION: Implements open system call functionality for RTC 
 *   INPUTS: ignored
 *   OUTPUTS: none
 *   RETURN VALUE: always 0 for success
 *   SIDE EFFECTS: none
 */
int32_t rtc_open(const uint8_t* filename)
{
	/* Initialize the RTC to 2 Hz and return success */
	int freq = DEFAULT_FREQ;
	rtc_write(0,&freq,SIZEOF_LONG);
	return 0;
}

/* 
 * rtc_close(int32_t fd)
 *   DESCRIPTION: Implements close system call functionality for RTC 
 *   INPUTS: ignored
 *   OUTPUTS: none
 *   RETURN VALUE: always 0 for success
 *   SIDE EFFECTS: none
 */
int32_t rtc_close(int32_t fd)
{
	/* Return success */
	return 0;
}

/* 
 * freq_to_rate(uint32_t freq)
 *   DESCRIPTION: Implements a mapping from hz frequency to regA rate value
 *				  as given by the RTC documentation
 *   INPUTS: uint32_t freq - hz frequency to be converted to rate
 *   OUTPUTS: none
 *   RETURN VALUE: regA rate value equivalent or 1 for invalid param
 *   SIDE EFFECTS: none
 */
uint8_t freq_to_rate(uint32_t freq)
{
	switch(freq)
	{
		case 2:
			return 0xF;
		case 4:
			return 0xE;
		case 8:
			return 0xD;
		case 16:
			return 0xC;
		case 32:
			return 0xB;
		case 64:
			return 0xA;
		case 128:
			return 0x9;
		case 256:
			return 0x8;
		case 512:
			return 0x7;
		case 1024:
			return 0x6;
		default:
			return 1;
	}
}

