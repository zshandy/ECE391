/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */


/*
*   void i8259_init(void)
*   Inputs: void
*   Return Value: none
*   Function: Initialize the 8259 PIC
*/
void
i8259_init(void)
{

	//unsigned char a1, a2;
	// a1 = inb(MASTER_8259_PORT_DATA);
	// a2 = inb(SLAVE_8259_PORT_DATA);

	master_mask = MASTER_INIT_MASK;
	slave_mask = SLAVE_INIT_MASK;


	outb(ICW1, MASTER_8259_PORT);	//start the initialization sequence
	outb(ICW1, SLAVE_8259_PORT);
	outb(ICW2_MASTER, MASTER_8259_PORT_DATA);	//initialize the master pic with corresponding offset
	outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA);		//initialize the slave pic with coreesponding offset
	outb(ICW3_MASTER, MASTER_8259_PORT_DATA);	//tell master pic there is a slave pic at IRQ 2
	outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA);		//tell Slave PIC its cascade identity (0000 0010)
	outb(ICW4, MASTER_8259_PORT_DATA);
	outb(ICW4, SLAVE_8259_PORT_DATA);

	outb(master_mask, MASTER_8259_PORT_DATA);	//restore the master pic 
	outb(slave_mask, SLAVE_8259_PORT_DATA);		//resoter the slave pic


}


/*
*   enable_irq(uint32_t irq_num)
*   Inputs: irq_num - the irq number to be enable on slave/master PIC
*   Return Value: none
*   Function: Enable (unmask) the specified IRQ 
*/
void
enable_irq(uint32_t irq_num)
{
	int bit_mask = 0x1;		//bitmask to help get the correct value
	uint16_t port;
	uint8_t val; 
	
	if(irq_num < MS_OFFSET){    // check if the irq_num in slave PIC
		port = MASTER_8259_PORT_DATA;   // store port data from master PIC
		master_mask = master_mask & (~(bit_mask<<irq_num));   // set up mask for pin state of master PIC
		val = master_mask;
	}
	else{   // else, the irq_num is in master PIC
		master_mask = master_mask & (~(bit_mask << 2));  // set up mask for pin state of master PIC
		outb(master_mask, MASTER_8259_PORT_DATA);    

		irq_num = irq_num - MS_OFFSET; 
		port = SLAVE_8259_PORT_DATA;
		bit_mask = bit_mask << irq_num;   // set up mask for pin state of slave PIC
		bit_mask = ~bit_mask;
		val = slave_mask & bit_mask;
		slave_mask = val;
	}

		outb(val, port);	//write the output to the corresponding port
	
}


/*
*   disable_irq(uint32_t irq_num)
*   Inputs: irq_num - the irq number to be disable on slave/master PIC
*   Return Value: none
*   Function: Disable (mask) the specified IRQ 
*/
void
disable_irq(uint32_t irq_num)
{
	int bit_mask = 0x1;		//bitmask to help get the correct value
	uint16_t port;
	uint8_t val;
	
	//if IRQ < 8, disable the master data
	if(irq_num < MS_OFFSET){
		port = MASTER_8259_PORT_DATA;
	}
	else{
		//if IRQ >= 8, disable the slave data
		irq_num = irq_num - MS_OFFSET; 
		port = SLAVE_8259_PORT_DATA;
	}
	//using the bitmask shift to help get the correct value
	bit_mask = bit_mask << irq_num;
	val = inb(port) | bit_mask;
	outb(val, port);	//write the output to the corresponding port
	
}



/*
*   send_eoi(uint32_t irq_num)
*   Inputs: irq_num - the irq number to be sent EOI to
*   Return Value: none
*   Function: Send end-of-interrupt signal for the specified IRQ 
*/
void
send_eoi(uint32_t irq_num)
{
	uint32_t EOI_slave;
	uint32_t EOI_master;
	//process of sending EOI according to IRQ number
	if(irq_num >= MS_OFFSET){
		//if IRQ >= 8, sending EOI both to slave and master
		EOI_slave = EOI | (irq_num - MS_OFFSET);
		outb(EOI_slave, SLAVE_8259_PORT);
		EOI_master = EOI | MS_PIN;
		outb(EOI_master, MASTER_8259_PORT);
		}
	else{
		//if IRB < 8, send EOI only to master
		EOI_master = EOI | irq_num;
		outb(EOI_master, MASTER_8259_PORT);
		}
}

