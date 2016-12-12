/*
* @Author: Jack
* @Date:   2016-10-19 19:53:20
* @Last Modified by:   Jack
* @Last Modified time: 2016-12-03 22:49:25
*/

#include "terminal.h"

static int TERM1_FLAG = 1;
static int TERM2_FLAG = 1;

/* File operations table */
fops_t term_file_operations = {
	.read = key_read,
	.write = term_write,
	.open = term_open,
	.close = term_close
};

/* 
 * term_init()
 *   DESCRIPTION: Initialize the terminal by resetting the cursor
 *				  and clear screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clears screen
 */
void term_init(void)
{
	screen_x = 0;
	screen_y = 0;
	clear();
}

/* 
 * term_read(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: 
 *   INPUTS: all ignored
 *   OUTPUTS: none
 *   RETURN VALUE: always 0 for success
 *   SIDE EFFECTS: 
 */
int32_t term_read(int32_t fd, void* buf, int32_t nbytes)
{
	return 0;
}

/* 
 * term_write(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: takes buffer and outputs nbytes to screen
 *   INPUTS: fd - file descriptor
 *			 const void* buf - buf to write from
 *			 int32_t nbytes - num bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *   SIDE EFFECTS: modifies video memory
 */
int32_t term_write(int32_t fd, const void* buf, int32_t nbytes)
{
	/* Loop counter */
	uint32_t i, x = screen_x, y = screen_y;


	/* Null ptr check */
	if(buf == NULL || nbytes < 0) return -1;

	/* Cast buf to a char ptr */
	char * addr = (char *)buf;


	/* Loops through the given buffer and writes to the screen */
	for(i = 0; i < nbytes; i++){
		/* Check to see if need to go to a new line */
		if((addr[i] == '\n') || (x > NUM_COLS - 1)){
			x = 0;
	    	y++;
		}

		/* If at the bottom, scroll the whole screen up */
	    if(y > NUM_ROWS - 1){
	    	y = NUM_ROWS - 1;
	    	x = 0;
	    	vert_scroll();
	    }

		/* Write to video memory */
		if(addr[i] != '\n'){
			*(uint8_t *)(VIDEO + ((NUM_COLS * y + x) << 1)) = addr[i];
	    	*(uint8_t *)(VIDEO + ((NUM_COLS * y + x) << 1) + 1) = ATTRIB;
	    	x++;
	    }
	}

	screen_y = y;
	screen_x = x;
	
	return 0;
}

/* 
 * term_open(const uint8_t filename)
 *   DESCRIPTION: intialize a terminal
 *   INPUTS: ignored
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: none
 */
int32_t term_open(const uint8_t* filename)
{
	/* Initialize the terminal and return success */
	term_init();
	return 0;
}

/* 
 * term_close(int32_t fd)
 *   DESCRIPTION: Returns success
 *   INPUTS: all ignored
 *   OUTPUTS: none
 *   RETURN VALUE: always 0 for success
 *   SIDE EFFECTS: none
 */
int32_t term_close(int32_t fd)
{
	/* Return success */
	return 0;
}

/* 
 * term_switch(int new_term)
 *   DESCRIPTION: Switch the active terminal
 *   INPUTS: int new_term - the terminal to switch to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the active terminal
 */
void term_switch(int new_term)
{
	int old_term = get_active_term();

	/* Get the process executing in the active term */
	int i, active_term = old_term;
	pcb_t * pcb;
	pcb_t * old_pcb = get_pcb();

	/* Print error message if no new tasks can be started, but the user is trying to open a new terminal */
	if(tasks_bitmap == 0x80 && (((new_term == 1) && TERM1_FLAG) || ((new_term == 2) && TERM2_FLAG))){
			
		for(i = 1; i < MAX_PROCESSES; i++){
			pcb = (pcb_t *)(EIGHT_MB - i*EIGHT_KB);
			if(pcb->term == active_term && pcb->child == NULL) break;
		}

		/* Swipswap screenx/y values if curr process in inactive term */
		if(old_pcb->term != active_term){
			saved_x[old_pcb->term] = screen_x;
			saved_y[old_pcb->term] = screen_y;
			screen_x = saved_x[active_term];
			screen_y = saved_y[active_term];
		}

		/* Switch to addr space of curr process */
		set_page_directory(pcb->task_id);

		term_write(0,(void*)err_proc,strlen((int8_t*)err_proc));
		
		/* Swipswap screenx/y values if curr process in inactive term */
		if(old_pcb->term != active_term){
			saved_x[active_term] = screen_x;
			saved_y[active_term] = screen_y;
			screen_x = saved_x[old_pcb->term];
			screen_y = saved_y[old_pcb->term];
		}

		/* Switch back addr spaces */
		set_page_directory(old_pcb->task_id);
		return;
	}

	/* Check validity of new terminal */
	if(new_term < 0 || new_term > 2) return;

	/* If it's not really a change leave, otherwise set a new active term */
	if(new_term == old_term) return;
	else set_active_term(new_term);

	/* Swap one of the video backups into the active video memory */
	switch_vidmem(old_term, new_term);

	/* If entering a new terminal without a shell....start dat shell */
	if(new_term == 1 && TERM1_FLAG){
		sys_fork();
		TERM1_FLAG = 0;
	}
	if(new_term == 2 && TERM2_FLAG){
		sys_fork();
		TERM2_FLAG = 0;
	}
}
