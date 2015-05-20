/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "rtc.h"
#include "i8259.h"
#include "lib.h"

#define REG_B 0x8B
#define NMI_PORT 0x70
#define RTC_RAM 0x71
#define REG_B_ON 0x40
#define REG_A 0x8A

//global variable to check the next interrupt
volatile int32_t interrupt_flag;

/* Initialize the 8259 PIC */
void
rtc_init(void)
{
	// printf("RTC INIT WORK!! \n");	
	// disable_irq(RTC_IRQ);	//disable the interrupts
	outb(REG_B, NMI_PORT);	//select Register B, and disable NMI
	char prev = inb(RTC_RAM);	//read the current value of Register B
	outb(REG_B, NMI_PORT);	//reset the index again
	outb(prev | REG_B_ON, RTC_RAM);	//write the previous value ORed with 0x40. This turns on bit 6 of Register B
	enable_irq(RTC_IRQ);	//enable the RTC
	int rate = 15;
	outb(REG_A, NMI_PORT);
	prev = inb(RTC_RAM);
	outb(REG_A, NMI_PORT);
	outb((prev & 0xF0) | rate, RTC_RAM);	//write the previous value ORed with rate to set new frequency

	// while(1){
	//test_interrupts();
	//printf("RTC INIT WORKS!! \n");

	// }
}

/* set RTC to 2Hz */
int32_t
rtc_open(const uint8_t* filename)
{
	/*
	uint32_t rate = Rate_2;	//seting the last four bits of Register A to 0x0F so the frequency is 2Hz
	rate &= 0x0F;	
	outb(REG_A, NMI_PORT);	//read the current value of Register A
	char prev = inb(RTC_RAM);	//read the current value 
	outb(REG_A, NMI_PORT);	//reset the index again
	outb((prev & 0xF0) | rate, RTC_RAM);	//write the previous value ORed with 0x0F to set 2Hz
	*/
	rtc_init();
	// rtc_write(2);	//set the frequency to be 2Hz

	return 0;

}

//wait until next interrupt
int32_t
rtc_read(int32_t fd, uint8_t* buf, int32_t nbytes)
{
	interrupt_flag = 1;	//reset flag for next interrupt
	while(interrupt_flag);	//wait until next interrupt
	return 0;
}

//write new frequency to RTC
int32_t
rtc_write(int32_t fd, const uint8_t* buf, int32_t nbytes)
{		
	uint32_t rate;	//rate value depends on different new_freq value
	switch(*buf){
		case Hz_2:	//new_freq = 2Hz
			rate = Rate_2;
			break;
		case Hz_4:	//new_freq = 4Hz
			rate = Rate_4;
			break;
		case Hz_8:	//new_freq = 8Hz
			rate = Rate_8;
			break;
		case Hz_16:	//new_freq = 16Hz
			rate = Rate_16;
			break;
		case Hz_32:	//new_freq = 32Hz
			rate = Rate_32;
			break;
		case Hz_64:	//new_freq = 64Hz
			rate = Rate_64;
			break;
		case Hz_128:	//new_freq = 128Hz
			rate = Rate_128;
			break;
		// case Hz_256:	//new_freq = 256Hz
		// 	rate = Rate_256;
		// 	break;
		// case Hz_512:	//new_freq = 512Hz
		// 	rate = Rate_512;
		// 	break;
		// case Hz_1024:	//new_freq = 1024Hz
		// 	rate = Rate_1024;
		// 	break;
		default:	//other value would return -1, which means to be set 1024Hz by default
			return -1;
	}
	//process of writing new frequency to RTC, which is similar to rtc_open
	rate &= 0x0F;
	outb(REG_A, NMI_PORT);
	char prev = inb(RTC_RAM);
	outb(REG_A, NMI_PORT);
	outb((prev & 0xF0) | rate, RTC_RAM);	//write the previous value ORed with rate to set new frequency

	return 0;
}

//directly return 0
int32_t
rtc_close(int32_t fd){
	return 0;
}

