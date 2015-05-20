#include "pit.h"

#define PIT_DEF_FREQ 1193180
#define PIT_KN_HZ 100

/*
*   void pit_init()
*   Inputs: void
*   Return Value: void
*	Function: enable and setting up the pit handler
*/
void pit_init(){

	int divisor = PIT_DEF_FREQ / PIT_KN_HZ;       /* Calculate our divisor */
    outb(0x36, 0x43);             /* Set our command byte 0x36 */
    int l_byte = divisor & 0xFF;
    outb(l_byte, 0x40);   /* Set low byte of divisor */
    int h_byte = divisor >> 8;
    outb(h_byte, 0x40);     /* Set high byte of divisor */
}

/*
*   void send_pit_eoi()
*   Inputs: void
*   Return Value: void
*	Function: send eoi when pit is done
*/
void send_pit_eoi(){

	send_eoi(0);
}

