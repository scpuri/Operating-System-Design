/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask = MASK_ALL; /* IRQs 0-7 */
uint8_t slave_mask = MASK_ALL; /* IRQs 8-15 */

/* 
 * i8259_init
 *   DESCRIPTION: initialize the PIC with the four 
 *			     control words and enable the slave IRQ line
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
i8259_init(void)
{
	/* Mask interrupts on the master and slave */
	outb(master_mask, MASTER_8259_PORT + 1);
	outb(slave_mask, SLAVE_8259_PORT + 1);

	/* Control words to master */
	outb(ICW1, MASTER_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_PORT + 1);
	outb(ICW3_MASTER, MASTER_8259_PORT + 1);
	outb(ICW4, MASTER_8259_PORT + 1);

	/* Control words to slave */
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);
	outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);
	outb(ICW4, SLAVE_8259_PORT + 1);

	/* IRQ 2 is the slave and therefore should
	   be enabled */
	enable_irq(SLAVE_IRQ);
}

/* 
 * enable_irq
 *   DESCRIPTION: enable an IRQ line on the PIC
 *   INPUTS: uint32_t irq_num - the irq number to enable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
enable_irq(uint32_t irq_num)
{

	uint16_t port;
	uint8_t value;

	/* Check to see if the desired interrupt to alter is the
	   master or the slave, and then set the port to correspond
	   to that */
	if(irq_num < NUM_IRQ_PER){
		port = MASTER_8259_PORT + 1;
	} else {
		port = SLAVE_8259_PORT + 1;
		irq_num -= NUM_IRQ_PER; //Slave is only indexed to 8
	}

	/* Get the current Interrupt Mask Register data and change
	one bit */
	value = inb(port) & ~(1 << irq_num);
	outb(value, port);
}

/* 
 * disable_irq
 *   DESCRIPTION: disable an IRQ line on the PIC
 *   INPUTS: uint32_t irq_num - the irq number to disable
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
disable_irq(uint32_t irq_num)
{
	uint16_t port;
	uint8_t value;

	/* Check to see if the desired interrupt to alter is the
	   master or the slave, and then set the port to correspond
	   to that */
	if(irq_num < NUM_IRQ_PER){
		port = MASTER_8259_PORT + 1;
	} else {
		port = SLAVE_8259_PORT + 1;
		irq_num -= NUM_IRQ_PER;//Slave is only indexed to 8
	}

	/* Get the current Interrupt Mask Register data and change
	one bit */
	value = inb(port) | (1 << irq_num);
	outb(value, port);
}

/* 
 * send_eoi
 *   DESCRIPTION: send the appropriate EOI message to the PIC
 *   INPUTS: uint32_t irq_num - the irq number of the handled interrupt
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void
send_eoi(uint32_t irq_num)
{
	/* Add the IRQ number to the EOI message */
	uint32_t EOI_ = EOI | (irq_num & MASK_ALL);

	/* If the interrupt affects the slave,
	   tell the slave to end the interrupt */
	if(irq_num >= NUM_IRQ_PER) {
		outb(EOI_-NUM_IRQ_PER, SLAVE_8259_PORT);
		outb((EOI | SLAVE_IRQ), MASTER_8259_PORT);
	} else {
		/* Always send EOI to the master */
		outb(EOI_, MASTER_8259_PORT);
	}
}

