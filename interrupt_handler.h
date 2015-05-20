#ifndef _INTERRUPT_HANDLER_H
#define _INTERRUPT_HANDLER_H

#include "lib.h"

/* Hd controller regs. Ref: IBM AT Bios-listing */
#define HD_DATA		0x1f0	/* _CTL when writing */
#define HD_ERROR	0x1f1	/* see err-bits */
#define HD_NSECTOR	0x1f2	/* nr of sectors to read/write */
#define HD_SECTOR	0x1f3	/* starting sector */
#define HD_LCYL		0x1f4	/* starting cylinder */
#define HD_HCYL		0x1f5	/* high byte of starting cyl */
#define HD_CURRENT	0x1f6	/* 101dhhhh , d=drive, hhhh=head */
#define HD_STATUS	0x1f7	/* see status-bits */
#define HD_PRECOMP HD_ERROR	/* same io address, read=error, write=precomp */
#define HD_COMMAND HD_STATUS	/* same io address, read=status, write=cmd */

#define HD_CMD		0x3f6

/* Bits of HD_STATUS */
#define ERR_STAT	0x01
#define INDEX_STAT	0x02
#define ECC_STAT	0x04	/* Corrected error */
#define DRQ_STAT	0x08
#define SEEK_STAT	0x10
#define WRERR_STAT	0x20
#define READY_STAT	0x40
#define BUSY_STAT	0x80

/* Values for HD_COMMAND */
#define WIN_RESTORE		0x10
#define WIN_READ		0x20
#define WIN_WRITE		0x30
#define WIN_VERIFY		0x40
#define WIN_FORMAT		0x50
#define WIN_INIT		0x60
#define WIN_SEEK 		0x70
#define WIN_DIAGNOSE		0x90
#define WIN_SPECIFY		0x91

//RTC handler
void rtc_handler(void);

//Keyboard handler
void keybrd_handler(void);

void mouse_handler(void);

//index for the displaying terminal
extern uint32_t display_index;

void hd_init(void *BIOS);
void hd_read_user(char *hd_buffer, int ecx, int bl);
void hd_write_user(char *hd_buffer, int ecx, int bl);
extern void hd_handler(void);
extern char hd_buffer[513];
extern char password[513];
extern int TIME_OUT;
extern int time_out;
typedef struct hd_i_struct{
	unsigned int head,sect,cyl,wpcom,lzone,ctl;
}hd_i_struct;


#define MOUSE_IRQ  12
#define MAGIC_CMD 0x20
#define MOUSE_CMD 0x64
#define MOUSE_DISABLE	0xa7		/* disable mouse */
#define ENABLE_MOUSE	0xa8		/* enable mouse */
#define MOUSE_WRITE_VAL1 0x60		/* value to write to controller */
#define MOUSE_WRITE_VAL2	0xd4		/* value to send mouse device data */
#define MOUSE_ENABLE_INTERRUPT	0x47		/* enable controller interrupts */
#define MOUSE_DEV_ENABLE	0xf4		/* enable mouse device */
#define MOUSE_INPUT_PORT	0x60		/* mouse device output buffer */
#define MOUSE_OUTPUT_PORT	0x60		/* mouse device input buffer */
#define MOUSE_COMMAND	0x64		/* mouse device command buffer */
#define MOUSE_STATUS	0x64		/* mouse device status reg */

/* mouse controller status bits */
#define MOUSE_OBUF_FULL	0x01		/* output buffer (from device) full */
#define MOUSE_IBUF_FULL	0x02		/* input buffer (to device) full */

/* mouse controller commands */
#define MOUSE_CMD_WRITE	0x60		/* value to write to controller */
#define MOUSE_MAGIC_WRITE	0xd4		/* value to send mouse device data */

#define MOUSE_INTS_ON	0x47		/* enable controller interrupts */
#define MOUSE_INTS_OFF	0x65		/* disable controller interrupts */

#define MOUSE_DISABLE	0xa7		/* disable mouse */
#define MOUSE_ENABLE	0xa8		/* enable mouse */

/* mouse device commands */
#define MOUSE_SET_RES	0xe8		/* set resolution */
#define MOUSE_SET_SCALE	0xe9		/* set scaling factor */
#define MOUSE_SET_STREAM	0xea		/* set stream mode */
#define MOUSE_SET_SAMPLE	0xf3		/* set sample rate */
#define MOUSE_ENABLE_DEV	0xf4		/* enable mouse device */
#define MOUSE_DISABLE_DEV	0xf5		/* disable mouse device */
#define MOUSE_RESET	0xff		/* reset mouse device */
#define VIDEO 0xB8000
#define NUM_COLS 80
#define NUM_ROWS 25
#define ATTRIB 0x7

void init_mouse(void);

/*
int32_t mouse_read(int32_t fd, void* buf, int32_t nbytes);
int32_t mouse_write(int32_t fd, void* buf, int32_t nbytes);
int32_t mouse_open(const uint8_t * command);
int32_t mouse_close(int32_t fd);*/



#endif
