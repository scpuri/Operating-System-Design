/*
* @Author: Jack
* @Date:   2016-10-15 15:46:38
* @Last Modified by:   Jack
* @Last Modified time: 2016-12-03 21:59:25
*/

/* SOURCE: http://flint.cs.yale.edu/cs422/doc/art-of-asm/pdf/APNDXC.PDF */

#include "keyboard.h"

/* Arrays for output from keyboard for different combinations of shift and caps lock */
static const unsigned char keyboard_map[NUM_KEYS] = {'X','X','1','2','3','4','5','6','7','8','9','0','-','=','X','X',
'q','w','e','r','t','y','u','i','o','p','[',']','\n','X','a','s','d','f','g','h','j',
'k','l',';','\'','`','X','\\','z','x','c','v','b','n','m',',','.','/','X','*','X',' '
};

static const unsigned char caps_keyboard_map[NUM_KEYS] = {'X','X','1','2','3','4','5','6','7','8','9','0','-','=','X','X',
'Q','W','E','R','T','Y','U','I','O','P','[',']','\n','X','A','S','D','F','G','H','J',
'K','L',';','\'','`','X','\\','Z','X','C','V','B','N','M',',','.','/','X','*','X',' '
};

static const unsigned char shift_keyboard_map[NUM_KEYS] = {'X','X','!','@','#','$','%','^','&','*','(',')','_','+','X','X',
'Q','W','E','R','T','Y','U','I','O','P','{','}','\n','X','A','S','D','F','G','H','J',
'K','L',':','\"','~','X','|','Z','X','C','V','B','N','M','<','>','?','X','*','X',' '
};

static const unsigned char caps_shift_keyboard_map[NUM_KEYS] = {'X','X','!','@','#','$','%','^','&','*','(',')','_','+','X','X',
'q','w','e','r','t','y','u','i','o','p','{','}','\n','X','a','s','d','f','g','h','j',
'k','l',':','\"','~','X','|','z','x','c','v','b','n','m','<','>','?','X','*','X',' '
};

/* Flags to tell which key modifiers are active */
static int CTL_FLAG;
static int SHIFT_FLAG;
static int SHIFTR_FLAG;
static int CAPS_FLAG;
static int ENTER_FLAG[NUM_TERMS];
static int ALT_FLAG;

/* Holds the input string to help utilize line-buffered input */
static char command_line[NUM_TERMS][BUFFER_SIZE];
static int cl_index[NUM_TERMS];
static int active_term;

/* 
 * keyboard_init()
 *   DESCRIPTION: Initialize all the information necessary for the keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes file-scope values in keyboard.c
 */
void keyboard_init(void)
{
	int i, j;
	enable_irq(LOC_OF_KEYBOARD);
	
	/* Populate the buffer with null characters */
	for(i = 0; i < NUM_TERMS; i++){
		for(j = 0; j < BUFFER_SIZE; j++){
			command_line[i][j] = '\0';
		}
		cl_index[i] = 0;
		ENTER_FLAG[i] = 0;
	}

	/* Start writing in zeroth position */
	active_term = 0;

	/* Clear all flags */
	CTL_FLAG = 0;
	SHIFT_FLAG = 0;
	SHIFTR_FLAG = 0;
	CAPS_FLAG = 0;
	ALT_FLAG = 0;
}

/* 
 * key_valid(keycode)
 *   DESCRIPTION: Checks to see if a given keycode is supported by the driver 
 *   INPUTS: keycode -- the keycode to check
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if valid, 0 otherwise
 *   SIDE EFFECTS: none
 */
int key_valid(keycode)
{
	if((keycode > 0x01 && keycode < 0x0F) || (keycode > 0x0F && keycode < 0x37) || 
		(keycode > 0x37 && keycode < 0x3B) || keycode == 0x9D || keycode == 0xAA ||
		keycode == 0xB6 || keycode == 0xB8 || keycode == 0xBA)
	{
		return 1;
	}
	return 0;
}


/* 
 * keyboard_handler()
 *   DESCRIPTION: Receive an interrupt from the keyboard and write
 *				  the corresponding character to the screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Writes characters to the screen, sets flags
 */
void keyboard_handler(void) 
{
	/* Get the scancode from the keyboard */
	int keycode = inb(DATA_PORT);

	/* Alt-fxn check */
	if(ALT_FLAG && keycode != ALT_UP && keycode != ALT_DOWN && keycode != CTL_DOWN && keycode != CTL_UP && \
		keycode != CAPS_DOWN && keycode != SHIFT_DOWN && keycode != SHIFT_UP && keycode != SHIFTR_DOWN && \
		keycode != SHIFTR_UP){
		if((F1_DOWN <= keycode) && (keycode < (F1_DOWN + NUM_TERMS))){
			term_switch(keycode - F1_DOWN);
		}
		send_eoi(LOC_OF_KEYBOARD);
		return;
	}

	/* If the key is not handled, just return */
	if(!key_valid(keycode))
	{
		send_eoi(LOC_OF_KEYBOARD);
		return;
	}

	/* Either set a flag or write a character to the screen */
	switch(keycode){
		case(CTL_DOWN):
			CTL_FLAG++;
			if(CTL_FLAG > 2) CTL_FLAG = 2;
			break;
		case(CTL_UP):
			CTL_FLAG--;
			if(CTL_FLAG < 0) CTL_FLAG = 0;
			break;
		case(ALT_DOWN):
			ALT_FLAG++;
			if(ALT_FLAG > 2) ALT_FLAG = 2;
			break;
		case(ALT_UP):
			ALT_FLAG--;
			if(ALT_FLAG < 0) ALT_FLAG = 0;
			break;
		case(CAPS_DOWN):
			CAPS_FLAG = 1 - CAPS_FLAG;
			break;
		case(SHIFT_DOWN):
			SHIFT_FLAG = 1;
			break;
		case(SHIFT_UP):
			SHIFT_FLAG = 0;
			break;
		case(SHIFTR_DOWN):
			SHIFTR_FLAG = 1;
			break;
		case(SHIFTR_UP):
			SHIFTR_FLAG = 0;
			break;
		case(BACKSPACE):
			if(cl_index[active_term] == 0)
				break;
			cl_index[active_term]--;	
			command_line[active_term][cl_index[active_term]] = '\0';
			break;
		case(ENTER):
			/* Do newline and then reset the command line */
			if(cl_index[active_term] >= BUFFER_SIZE)
				command_line[active_term][BUFFER_SIZE - 1] = '\n';
			else
				command_line[active_term][cl_index[active_term]] = '\n';
			if(ENTER_FLAG[active_term] == 0){
				/* Populate the buffer with null characters */
				for(cl_index[active_term] = 0; cl_index[active_term] < BUFFER_SIZE; cl_index[active_term]++){
					command_line[active_term][cl_index[active_term]] = '\0';
				}
				cl_index[active_term] = 0;
			}
			ENTER_FLAG[active_term] = 0;
			break;
		default:
			if(keycode > TYPED){
				break;
			}
			/* Control-L check */
			else if(CTL_FLAG){
				if(keycode == L_DOWN){
					clear_keyboard();
				}
			}
			/* Depending on flags, write a different set of capital or lowercase characters */
			else if (((SHIFT_FLAG || SHIFTR_FLAG) && CAPS_FLAG) && cl_index[active_term] < BUFFER_SIZE - 1){
				command_line[active_term][cl_index[active_term]] = caps_shift_keyboard_map[keycode];
				cl_index[active_term]++;
			}
			else if ((!(SHIFT_FLAG || SHIFTR_FLAG) && CAPS_FLAG) && cl_index[active_term] < BUFFER_SIZE - 1){
				command_line[active_term][cl_index[active_term]] = caps_keyboard_map[keycode];
				cl_index[active_term]++;
			}
			else if (((SHIFT_FLAG || SHIFTR_FLAG) && !CAPS_FLAG) && cl_index[active_term] < BUFFER_SIZE - 1){
				command_line[active_term][cl_index[active_term]] = shift_keyboard_map[keycode];
				cl_index[active_term]++;
			}
			else if (cl_index[active_term] < BUFFER_SIZE - 1) {
				command_line[active_term][cl_index[active_term]] = keyboard_map[keycode];
				cl_index[active_term]++;
			}
	}

	/* Output to the screen and exit */
	keyboard_put();
	send_eoi(LOC_OF_KEYBOARD);
}


/* 
 * keyboard_put()
 *   DESCRIPTION: output keyboard input to screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: modifies video memory
 */
void keyboard_put(void)
{
	/* Loop counter */
	int i, j, x;
	j = 0;

	/* Cast buf to a char ptr */
	char * addr = (char *)(command_line[active_term]);

	/* Get the process executing in the active term */
	pcb_t * pcb;
	pcb_t * old_pcb = get_pcb();
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

	/* Move to video memory */
	for(x = screen_x, i = 0; i < BUF_SIZE; x++, i++){
		/* Let buffer overflow to second line */
		if(x > SCREEN_WIDTH-1){
			j++;
			x = 0;
		}

		/* Check for enter(newline) or scroll */
		if(addr[i] == '\n'){
			*(uint8_t *)(VIDEO + ((NUM_COLS*(screen_y + j) + x) << 1)) = '\0';
	        *(uint8_t *)(VIDEO + ((NUM_COLS*(screen_y + j) + x) << 1) + 1) = ATTRIB; 
			screen_y = screen_y + j + 1;
			screen_x = 0;
			break;
		} else if(screen_y + j > NUM_ROWS-1){
			vert_scroll();
			screen_y = NUM_ROWS - j - 1;
		}

		/* Output the char to video memory */
		*(uint8_t *)(VIDEO + ((NUM_COLS*(screen_y + j) + x) << 1)) = addr[i];
        *(uint8_t *)(VIDEO + ((NUM_COLS*(screen_y + j) + x) << 1) + 1) = ATTRIB; 

        /* If null char stop writing to vidmem */
        if(addr[i] == '\0')
        {
        	*(uint8_t *)(VIDEO + ((NUM_COLS*(screen_y + j) + x) << 1)) = BLOCK_TXT;
	        *(uint8_t *)(VIDEO + ((NUM_COLS*(screen_y + j) + x) << 1) + 1) = ATTRIB; 
	        *(uint8_t *)(VIDEO + ((NUM_COLS*(screen_y + j) + x + 1) << 1)) = '\0';
	        *(uint8_t *)(VIDEO + ((NUM_COLS*(screen_y + j) + x + 1) << 1) + 1) = ATTRIB; 
        	break;
        }
	}

	/* Swipswap screenx/y values if curr process in inactive term */
	if(old_pcb->term != active_term){
		saved_x[active_term] = screen_x;
		saved_y[active_term] = screen_y;
		screen_x = saved_x[old_pcb->term];
		screen_y = saved_y[old_pcb->term];
	}

	/* Switch back addr spaces */
	set_page_directory(old_pcb->task_id);
}


/* 
 * key_read(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: Block until the buffer has been terminated
 *				  by the line feed character (newline/enter)
 *				  then copy the local buffer into buf
 *   INPUTS: fd - ignored
 *			 void* buf - pointer to the user buffer
 *			 int32_t nbytes - the number of bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE: num bytes read
 *   SIDE EFFECTS: 
 */
int32_t key_read(int32_t fd, void* buf, int32_t nbytes)
{
	int i;
	int temp_index = 0;
	pcb_t * pcb = get_pcb();

	/* Null ptr check */
	if(buf == NULL) return -1;

	/* Block until the enter key has been pressed */
	ENTER_FLAG[pcb->term] = 1;
	sti();
	while(ENTER_FLAG[pcb->term]);
	cli();
	
	/* Copy the local buffer into parameter buf */
	for(i = 0; (i < BUFFER_SIZE && i < nbytes-1); i++){
		temp_index++;
		if(command_line[active_term][i] == '\n') break;
	}
	memcpy(buf, (void*)(command_line[active_term]), temp_index);

	if(i == nbytes-1) ((char*)buf)[i] = '\n';

	/* Populate the buffer with null characters */
	for(cl_index[active_term] = 0; cl_index[active_term] < BUFFER_SIZE; cl_index[active_term]++){
		command_line[active_term][cl_index[active_term]] = '\0';
	}
	cl_index[active_term] = 0;

	/* Return success */
	return temp_index;
}

/* 
 * key_write(int32_t fd, void* buf, int32_t nbytes)
 *   DESCRIPTION: 
 *   INPUTS: fd - file descriptor
 *			 const void* buf - buf to be copied to
 *			 int32_t nbytes - num bytes to copy
 *   OUTPUTS: none
 *   RETURN VALUE:
 *   SIDE EFFECTS:
 */
int32_t key_write(int32_t fd, const void* buf, int32_t nbytes)
{
	return 0;
}

/* 
 * key_open(const uint8_t filename)
 *   DESCRIPTION: initialze keyboard
 *   INPUTS: uint8_t* filename - name of file
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: initializes file scope variables
 */
int32_t key_open(const uint8_t* filename)
{
	keyboard_init();
	return 0;
}

/* 
 * key_close(int32_t fd)
 *   DESCRIPTION: 
 *   INPUTS: int32_t fd - file descriptor
 *   OUTPUTS: none
 *   RETURN VALUE: always 0 for success
 *   SIDE EFFECTS: none
 */
int32_t key_close(int32_t fd)
{
	return 0;
}

/* 
 * clear_keyboard(void)
 *   DESCRIPTION: clear the screen and set writing back to the top left
 *				  then put the buffer back on the screen
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clears the screen and resets cursor
 */
void clear_keyboard(void)
{
	/* Get the process executing in the active term */
	int i;
	pcb_t * pcb;
	pcb_t * old_pcb = get_pcb();
	cli();
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

	for(i=0; i<screen_y; i++)
	{
		vert_scroll();
	}
	screen_y = 0;

	/* Swipswap screenx/y values if curr process in inactive term */
	if(old_pcb->term != active_term){
		saved_x[active_term] = screen_x;
		saved_y[active_term] = screen_y;
		screen_x = saved_x[old_pcb->term];
		screen_y = saved_y[old_pcb->term];
	}

	/* Switch back addr spaces */
	set_page_directory(old_pcb->task_id);
	sti();
}

/* 
 * set_active_term(int new_term)
 *   DESCRIPTION: set a new term active
 *   INPUTS: int new_term - term to be active
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the value of active_term
 */
void set_active_term(int new_term){
	active_term = new_term;
}

/* 
 * get_active_term()
 *   DESCRIPTION: get value of active_term
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: value of active_term
 *   SIDE EFFECTS: none
 */
int get_active_term(void){
	return active_term;
}
