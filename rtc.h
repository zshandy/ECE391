// /* i8259.h - Defines used in interactions with the 8259 interrupt
//  * controller
//  * vim:ts=4 noexpandtab
//  */

// #ifndef _RTC_H
// #define _RTC_H

// #include "types.h"

// #define RTC_IRQ 8

// //initialize RTC
// void rtc_init(void);

// //open RTC helper function
// void rtc_open(uint32_t irq_num);

// //read RTC helper function
// void rtc_read(uint32_t irq_num);

// //write RTC helper function
// void rtc_write(uint32_t irq_num);

// #endif /* _RTC_H */


/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

#include "types.h"

#define RTC_IRQ 8

//Hz value
#define Hz_2 2
#define Hz_4 4
#define Hz_8 8
#define Hz_16 16
#define Hz_32 32
#define Hz_64 64
#define Hz_128 128
#define Hz_256 256
#define Hz_512 512
#define Hz_1024 1024

//Hz rate of last four bits of Register A
#define Rate_2 15
#define Rate_4 14
#define Rate_8 13
#define Rate_16 12
#define Rate_32 11
#define Rate_64 10
#define Rate_128 9
#define Rate_256 8
#define Rate_512 7
#define Rate_1024 6

//global variable to check the next interrupt
volatile int32_t interrupt_flag;	

//initialize RTC
void rtc_init(void);

//open RTC helper function
int32_t rtc_open(const uint8_t* filename);

//read RTC helper function
int32_t rtc_read(int32_t fd, uint8_t* buf, int32_t nbytes);

//write RTC helper function
int32_t rtc_write(int32_t fd, const uint8_t* buf, int32_t nbytes);

int32_t rtc_close(int32_t fd);

#endif /* _RTC_H */
