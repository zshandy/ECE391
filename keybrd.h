#ifndef _KEYBRD_H
#define _KEYBRD_H

#include "lib.h"
#include "i8259.h"


#define BUFFER_LIMIT 128
#define SCREEN_LIMIT 3
/* Keyboard handler */
// extern void keyboard_handler(void);
extern void keybrd_init();
extern int32_t terminal_read (int32_t fd, uint8_t* buf, int32_t nbytes);
extern int32_t terminal_write(int32_t fd, const uint8_t* buf, int32_t nbytes);
extern int32_t terminal_open(const uint8_t * filename);
extern int32_t terminal_close(int32_t fd);

int can_read;
uint8_t key_buf[BUFFER_LIMIT][SCREEN_LIMIT];
uint8_t out_buf[BUFFER_LIMIT][SCREEN_LIMIT];


#endif
