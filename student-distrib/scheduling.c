/*
* @Author: Jack, Charlie, Saurav, Rahul
* @Date:   2016-10-27 17:08:37
* @Last Modified by:   Jack
* @Last Modified time: 2016-12-03 22:32:26
*/

#include "scheduling.h"

int saved_x[NUM_TERMS];
int saved_y[NUM_TERMS];
static int FIRST_FLAG;

/* 
 * init_timer(void)
 *   DESCRIPTION: Set the rate of the PIT and the first time term flag
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: 
 */
void init_timer(void)
{
	set_rate(SCHEDULING_RATE);
	FIRST_FLAG = 1;	
}

/* 
 * irq_timer(uint32_t * esp)
 *   DESCRIPTION: Handler for the PIT that schedules tasks with a round robin algorithm
 *   INPUTS: esp - The esp to save from the PIT interrupt
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Changes scheduling/process structures
 */
uint32_t irq_timer(uint32_t* esp)
{
	int i;
	pcb_t * old_pcb = get_pcb();
	pcb_t * new_pcb;
	i = old_pcb->task_id;

	/* end PIT interrupt */
	send_eoi(0);

	/* if first process not running yet return */
	if(tasks_bitmap == NO_PROCESSES) return (uint32_t)esp;

	/* Determine the next child process to schedule */
	do {
		do {
			i++;
			if(i >= MAX_PROCESSES) i = 1;
		} while(tasks_bitmap & (0x1 << i));
		new_pcb = (pcb_t *)(EIGHT_MB - EIGHT_KB * i);
	} while(new_pcb->child != NULL);

	/* Do nothing if not switching tasks and not the first instance of an OG shell */
	if((old_pcb->task_id == new_pcb->task_id) && !FIRST_FLAG) return (uint32_t) esp;

	/* save old x,y and set new x,y */
	if(old_pcb->term != new_pcb->term)
	{
		saved_x[old_pcb->term] = screen_x;
		saved_y[old_pcb->term] = screen_y;
		screen_x = saved_x[new_pcb->term];
		screen_y = saved_y[new_pcb->term];
	}

	/* Set the the TSS esp0 */
	tss.esp0 = EIGHT_MB - (new_pcb->task_id-1)*EIGHT_KB - 1;
	
	/* stack swipswap */
	if(!FIRST_FLAG){
		old_pcb->ebp = esp[EBP_INDEX];
		old_pcb->esp = (uint32_t) esp;
	}
	FIRST_FLAG = 0;

	/* switch to new task's page directory and return */
	set_page_directory(new_pcb->task_id);
	return new_pcb->esp;
}

/* 
 * set_rate(int hz)
 *   DESCRIPTION: changes the rate of the PIT interrupts
 *   INPUTS: hz - the frequency of interrupts
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes register values for the PIT
 */
void set_rate(int hz)
{
	int divisor = BASECLOCKRATE / hz;// calculate the divider value with base clock rate
	outb(PIT_MODE3, PIT_CPORT);      // 0x36 for mode 3 (square wave), 0x43 command port
	outb(divisor & LSB_BYTE, PIT_DPORT); // send lower byte, then upper bytes
	outb(divisor >> BYTE_SHIFT, PIT_DPORT);   
}

void set_first_flag(int flag)
{
	FIRST_FLAG = flag;
}

