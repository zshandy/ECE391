#include "rtc.h"
#include "keybrd.h"
#include "i8259.h"
#include "interrupt_handler.h"
#include "paging.h"
#include "pit.h"
#include "sche.h"
#include "syscall.h"
#include "lib.h"

#define REG_C_OFF 0x0C
#define NMI_PORT 0x70
#define RTC_RAM 0x71

#define TABLE_SIZE 50
#define KEYBRD_DATA_PORT 0x60
#define PR_OFFSET 0x80
#define END_OF_BUF '\0'
#define TAB_SPACE 4
#define BUFFER_LIMIT 128
#define CHAR_NO 26
#define KEY_LIMIT 127
#define buf_len 32
#define VIDEO_MEM 0xB8000

#define ENTER_PRESSED 0x1C
#define CTRL_PRESS 0x1D
#define CTRL_RELEASE 0x9D
#define LEFT_SHIFT_PRESS 0x2A
#define LEFT_SHIFT_RELEASE 0xAA
#define RIGHT_SHIFT_PRESS 0x36
#define RIGHT_SHIFT_RELEASE 0xB6
#define ALT_PRESS 0x38
#define ALT_RELEASE 0xB8
#define CAPSLOCK_PRESS 0x3A
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D
#define BS_ASCII 8

#define SCREEN_SIZE 4096
#define VGA_BASE1 0x3D4			// addr of VGA index register
#define VGA_BASE2 0x3D5
#define HB_OFF 0x0C
#define LB_OFF 0x0D
#define DISP_OFF 0xFF
uint32_t display_index = 0;
uint32_t keypressed_count[SCREEN_LIMIT] = {0,0,0};
static char* video_mem = (char *)VIDEO_MEM;
static const uint8_t attr[3] = {0x7, 0x10, 0x20};

static int32_t x_pos;
static int32_t y_pos;
static int32_t s;
static uint8_t temp;
static uint8_t pre_ATTRIB;
static uint8_t paint;
static int source;
#define ATTRIB 0x7

static uint8_t scan_code[TABLE_SIZE] = { 0x1E, 0x30, 0x2E, 0x20, 0x12,		//value for the port to be received when keys are pressed
						0x21, 0x22, 0x23, 0x17, 0x24,
						0x25, 0x26, 0x32, 0x31, 0x18,
						0x19, 0x10, 0x13, 0x1F, 0x14,
						0x16, 0x2F, 0x11, 0x2D, 0x15,
						0x2C, 0x02, 0x03, 0x04, 0x05,
						0x06, 0x07, 0x08, 0x09, 0x0A,
						0x0B, 0x0C, 0x0D, 0x1A, 0x1B,
						0x27, 0x28, 0x29, 0x2B, 0x33,
						0x34, 0x35, 0x39, 0x0E, 0x1C};

/* Buffer which stores all lower case value*/
static uint8_t lower_key[TABLE_SIZE] = { 'a', 'b', 'c', 'd', 'e',			//each key pressed conrrespoed to the key (lower case)
						'f', 'g', 'h', 'i', 'j',
						'k', 'l', 'm', 'n', 'o',
						'p', 'q', 'r', 's', 't',
						'u', 'v', 'w', 'x', 'y',
						'z', '1', '2', '3', '4',
						'5', '6', '7', '8', '9',
						'0', '-', '=', '[', ']',
						';', 39,  '`', 92,  ',', 							// 92 = /, 39 = '
						'.', '/', ' ', BS_ASCII,  '\n'};							//8 = backspace

static uint8_t upper_key[TABLE_SIZE] = { 'A', 'B', 'C', 'D', 'E',			//each key pressed corresponded to the key (upper case), or with shift
						'F', 'G', 'H', 'I', 'J',
						'K', 'L', 'M', 'N', 'O',
						'P', 'Q', 'R', 'S', 'T',
						'U', 'V', 'W', 'X', 'Y',
						'Z', '!', '@', '#', '$',
						'%', '^', '&', '*', '(',
						')', '_', '+', '{', '}',
						':', '"', '~', '|', '<',
						'>', '?', ' ', BS_ASCII, '\n'};

static uint32_t ctrl_pressed = 0;											//initialze value for the flags
static uint32_t left_shift_pressed = 0;
static uint32_t right_shift_pressed = 0;
static uint32_t capslock_press = 0;
static uint32_t alt_pressed = 0;


/*
*   void screen_switch (uint32_t target_screen, uint32_t screen_size)
*   Inputs: target_screen, scrren_size
*   Return Value: none
*   Function: print out button pushed on screen
*/
void screen_switch (uint32_t target_screen, uint32_t screen_size)
{
	//calculating the high byte and display to the screen
	uint8_t high_byte = (screen_size*target_screen >> 8) & DISP_OFF;
	outb(HB_OFF, VGA_BASE1);
	outb(high_byte, VGA_BASE2);
	//calculating the low byte and display to the screen
	uint8_t low_byte = (screen_size*target_screen) & DISP_OFF;
	outb(LB_OFF, VGA_BASE1);
	outb(low_byte, VGA_BASE2);

	*(uint8_t *)(video_mem + 2*SCREEN_SIZE*display_index + (s << 1)) = temp;
    *(uint8_t *)(video_mem + 2*SCREEN_SIZE*display_index + (s << 1) + 1) = pre_ATTRIB;
	display_index = target_screen;

	//call paging and set up new pages for video mem
	vid_new(VIDEO_MEM + 2 * SCREEN_SIZE * display_index, display_index);
	send_eoi(1);
}

/*
* void keyboard_handler(void);
*   Inputs: void
*   Return Value: none
*   Function: print out button pushed on screen
*/

void keyboard_handler(void)
{       
        uint32_t i;
        uint8_t key_pressed = inb(KEYBRD_DATA_PORT);/*data port of keyboard*/
        uint8_t key_translate; 						//translated key when it is pressed

        if (key_pressed == CTRL_PRESS)				//flags for ctrl, shift, alt capslock when each pressed or released
			ctrl_pressed = 1;
		if (key_pressed == CTRL_RELEASE)
			ctrl_pressed = 0;
		if (key_pressed == LEFT_SHIFT_PRESS)
			left_shift_pressed = 1;	
		if (key_pressed == LEFT_SHIFT_RELEASE)
			left_shift_pressed = 0;
		if (key_pressed == RIGHT_SHIFT_PRESS)
			right_shift_pressed = 1;
		if (key_pressed == RIGHT_SHIFT_RELEASE)
			right_shift_pressed = 0;
		if (key_pressed == ALT_PRESS)
			alt_pressed = 1;
		if (key_pressed == ALT_RELEASE)
			alt_pressed = 0;
		if (key_pressed == CAPSLOCK_PRESS)
			capslock_press = 1 - capslock_press;

		if ((key_pressed == F1) && (alt_pressed == 1))
			screen_switch(0, SCREEN_SIZE);
		if ((key_pressed == F2) && (alt_pressed == 1))
			screen_switch(1, SCREEN_SIZE);
		if ((key_pressed == F3) && (alt_pressed == 1))
			screen_switch(2, SCREEN_SIZE);

        if((key_pressed & PR_OFFSET) == 0){                         //to check if the key is pressed
        	for (i = 0; i < TABLE_SIZE; i++){
            	if (key_pressed == scan_code[i]){          			//go through the array to find the key entered
            		if ((left_shift_pressed | right_shift_pressed) == 0){
            			if ((i < CHAR_NO) && capslock_press == 1)
            				key_translate = upper_key[i];           //go through the letter array to find the correspond letter, either in lower or upper case
            			else 										//or if shift is pressed or capslock is on
            				key_translate = lower_key[i];
            		}
            		else{
            			if ((i < CHAR_NO) && capslock_press == 1)
            				key_translate = lower_key[i];
            			else 
            				key_translate = upper_key[i];
            		}
					
					if (((key_translate == 'R') || (key_translate == 'r')) && (ctrl_pressed == 1)){
						if(talk_init_flag == 0){
							password[0] = '\0';
							hd_write_user(password, 16, 2);
							puts((int8_t*)"You can set a new password at next startup\n", 1);
						}
						send_eoi(1);  	//send end of interrupt signal
						/*asm ("leave;\
							iret;");*/
						return;
					}
					
					if (((key_translate == 'Z') || (key_translate == 'z')) && (ctrl_pressed == 1)){
						int indexz;
						char trash[513];
						for(indexz = 0; indexz < 513; indexz++){
							trash[indexz] = 0;
						}
						for(indexz = 0; indexz < 9; indexz++){
							hd_write_user(trash, 16, 3+indexz);
						}
						puts((int8_t*)"Chat history are all cleared\n", 1);
						send_eoi(1);  	//send end of interrupt signal
						out_talk = 1;
						/*asm ("leave;\
							iret;");*/
						return;
					}
					
					if (((key_translate == 'C') || (key_translate == 'c')) && (ctrl_pressed == 1)){
						int indexc;
						char trash[513];
						for(indexc = 0; indexc < 513; indexc++){
							trash[indexc] = 0;
						}
						hd_write_user(trash, 16, 3+talk_port);
						puts((int8_t*)"You have quit the chatroom\n", 1);
						send_eoi(1);  	//send end of interrupt signal
						out_talk = 1;
						/*asm ("leave;\
							iret;");*/
						return;
					}
           
            		if (((key_translate == 'T') || (key_translate == 't')) && ctrl_pressed == 1){
            			talk_init_flag = 0;
						out_talk = 0;
						send_eoi(1);
            			return;
            		}
            		if (((key_translate == 'L') || (key_translate == 'l')) && ctrl_pressed == 1){
            			clear ();									//clear screen when ctrl+l or ctrl+L is pressed
            			send_eoi(1);
            			return;
            		}
            		if (key_translate != BS_ASCII && key_translate != '\n'){
            			if (keypressed_count[display_index] == KEY_LIMIT)			//break if limit for the buffer is reached
            				break;
            			else {
            				key_buf[keypressed_count[display_index]][display_index] = key_translate;
							keypressed_count[display_index] ++;					//store the keys pressed into the buffer and increment the count for the keys
            			}
            		}
            		if (key_translate == BS_ASCII){						
						if(keypressed_count[display_index] > 0){					//if delete is pressed and there are letters in the buffer
							keypressed_count[display_index] --;
							print_backspace();						//decrement count and print to the screen
							key_buf[keypressed_count[display_index]][display_index] = END_OF_BUF; 
							send_eoi(1);
							return;
						}
						else{
							send_eoi(1);							//if no letter, just send_eoi
							return;
						}
            		}
            		if(key_translate == '\n'){   					//if enter pressed, start a new line
            			if (keypressed_count[display_index] < KEY_LIMIT){
            				key_buf[keypressed_count[display_index]][display_index] = '\n';
            				key_buf[keypressed_count[display_index] + 1][display_index] = END_OF_BUF;
            				keypressed_count[display_index]++;
            			}
            			else
							key_buf[keypressed_count[display_index]][display_index] = END_OF_BUF;		//store to the buffer
						uint32_t ii;
						for(ii=0; ii<keypressed_count[display_index]+1; ii++)
							out_buf[ii][display_index] = key_buf[ii][display_index];					//move all key_buffer value to out_buffer
						for(ii=0; ii<keypressed_count[display_index]+1; ii++)
							key_buf[ii][display_index] = '\0';					//move all key_buffer value to out_buffer
						keypressed_count[display_index] = 0;						//retore count to 0
						can_read = 0;
					}
            		putc (key_translate, 1);							//print to the screen
                }
            }
        }
        send_eoi(1);    //send eoi when  done
        return;
}


/*
*   void rtc_handler(void);
*   Inputs: void
*   Return Value: none
*   Function: real time clock
*/
void rtc_handler(void){

	// test_interrupts();
	//printf("test rtc\n");
	outb(REG_C_OFF, NMI_PORT); 	//select Register C
	inb(RTC_RAM);	//throw away the content
	if(interrupt_flag) 
		interrupt_flag = 0;	//next interrupt occurred, set flag to 0
	send_eoi(8);	//send EOI with RTC IRQ number: 8

}

int TIME_OUT = 10000;
int time_out = 0;
/*
*   void pit_handler(void)
*   Inputs: void
*   Return Value: none
*   Function: call scheduling when setting up PIT
*/
void pit_handler(void){
	if(talk_init_flag == 0){
		int i = 0;
		for(i = 0; i < end_port; i++){
			if(i == talk_port){
				continue;
			}
			hd_read_user(hd_buffer, 16, i+3);
			hd_buffer[512] = '\0';
			if(other_ports[i] == 0 && hd_buffer[0] > 0){
				puts("User number ", 1);
				putc((uint8_t)(i+1+0x30), 1);
				puts(" has joined the chatroom!\n", 1);
				other_ports[i] = 1;
			}
			if((other_ports[i] == 9 && hd_buffer[0] > 1 && hd_buffer[0] != 9) || hd_buffer[0] > other_ports[i]){
				other_ports[i] = hd_buffer[0];
				puts("User number ", 1);
				putc((uint8_t)(i+1+0x30), 1);
				puts(" says: ", 1);
				puts(hd_buffer+1, 1);
			}
			if(hd_buffer[0] == 0){
				if(other_ports[i] != 0){
					puts("User number", 1);
					putc((uint8_t)(i+1+0x30), 1);
					puts(" has left the chatroom!\n", 1);
					other_ports[i] = 0;
				}
			}
		}
		//time_out++;
		send_eoi(0);
		return;
	}
	scheduling();
}


static inline unsigned char CMOS_READ(unsigned char addr);

void inline outb_p(char value, unsigned short port)
{
__asm__ volatile ("outb %0,%1\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80\n\t"      
		  "outb %0,$0x80"
		::"a" ((char) value),"d" ((unsigned short) port));
}

unsigned char inline inb_p(unsigned short port)
{
	unsigned char _v;
__asm__ volatile ("inb %1,%0\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80"
		:"=a" (_v):"d" ((unsigned short) port));
	return _v;
}


static inline void port_read(int port, char* buf, int nr){
__asm__ volatile ("cld;rep;insw"
				 :
				 :"d" (port),"D" (buf),"c" (nr)
				 );
}

static inline void port_write(int port, char* buf, int nr){
__asm__ volatile ("cld;rep;outsw"
					:
					:"d" (port),"S" (buf),"c" (nr)
					);
}

static inline unsigned char CMOS_READ(unsigned char addr)
{
	outb_p(0x80|addr,0x70);
	return inb_p(0x71);
}

void hd_handler(void){
	
}


hd_i_struct hd_info = {0,0,0,0,0,0};
//static int NR_HD = 0;
char hd_buffer[513];
char password[513];


void hd_read_user(char *hd_buffer, int ecx, int bl){
	int flags;
	cli_and_save(flags);
	__asm__ volatile (
		"pushl %%ebx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%edi\n\t"
		"movl %0, %%edi\n\t"
		"movl %1, %%ecx\n\t"
		"movl %2, %%ebx\n\t"
		"call hd_read\n\t"
		"popl %%edi\n\t"
		"popl %%ecx\n\t"
		"popl %%ebx\n\t"
		::"r" ((char*)&hd_buffer[0]), "r" (ecx), "r" (bl));
	restore_flags(flags);
}

void hd_write_user(char *hd_buffer, int ecx, int bl){
	//puts("here1\n", 1);
	int flags;
	cli_and_save(flags);
	__asm__ volatile (
		"pushl %%ebx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%esi\n\t"
		"movl %0, %%esi\n\t"
		"movl %1, %%ecx\n\t"
		"movl %2, %%ebx\n\t"
		"call hd_write\n\t"
		"popl %%esi\n\t"
		"popl %%ecx\n\t"
		"popl %%ebx\n\t"
		::"r" ((char*)&hd_buffer[0]), "r" (ecx), "r" (bl));
	restore_flags(flags);
	//puts("here2\n", 1);
}


void cursor()
{
	if(x_pos < 0) x_pos = 0;
	if(x_pos >= 8000) x_pos = 7999;
	if(y_pos < 0) y_pos = 0;
	if(y_pos >= 2500) y_pos = 2499;
	*(uint8_t *)(video_mem + 2*SCREEN_SIZE*display_index + (s << 1)) = temp;
    *(uint8_t *)(video_mem + 2*SCREEN_SIZE*display_index + (s << 1) + 1) = pre_ATTRIB;
	int32_t g = x_pos/10000 + (y_pos/10000)*80;
	temp = *(uint8_t *)(video_mem + 2*SCREEN_SIZE*display_index + (g << 1));
	pre_ATTRIB = *(uint8_t *)(video_mem + 2*SCREEN_SIZE*display_index + (g << 1) + 1);
	*(uint8_t *)(video_mem + 2*SCREEN_SIZE*display_index + (g << 1)) = 'X';
    *(uint8_t *)(video_mem + 2*SCREEN_SIZE*display_index + (g << 1) + 1) = attr[display_index];
	s = g;
}

/*
* void mouse_handler(void);
*   Inputs: void
*   Return Value: none
*	Function: invoke the PS/2 mouse
*/
void mouse_handler(void){
	//cli();
	source=inb(MOUSE_COMMAND);
	source &= 0x20;
	if (source!=0x20) 
	{
		send_eoi(MOUSE_IRQ);
		//sti();
		return;
	}
	
	int8_t input;
	int32_t tempx;
	int32_t tempy;
	input = inb(MOUSE_INPUT_PORT);
	tempx = inb(MOUSE_INPUT_PORT);
	tempy = inb(MOUSE_INPUT_PORT);
	if(input & 0xC0) {
		send_eoi(MOUSE_IRQ);
		//sti();
		return;
	}
	if(input & 0x10) tempx |= 0xFFFFFF00;
	if(input & 0x20) tempy |= 0xFFFFFF00;
	x_pos += tempx;
	y_pos -= tempy;
	//printf("%d %d\n", tempx, tempy);
	//update_cursor(y_pos/10, x_pos/10);
	cursor();
	//printf("%d %d\n", x_pos, y_pos);
	//puts("here2\n", 1);
	send_eoi(MOUSE_IRQ);
	//sti();
	return;
}

/*
* void init_mouse(void);
*   Inputs: void
*   Return Value: none
*	Function: initialize the PS/2 mouse
*/
void init_mouse(void){
	int8_t status;
	enable_irq(MOUSE_IRQ);
	outb(MOUSE_ENABLE, MOUSE_COMMAND);
	outb(0x20, MOUSE_COMMAND);
	status = inb(MOUSE_INPUT_PORT);
	status |= 0x02;
	outb(MOUSE_CMD_WRITE, MOUSE_COMMAND);
	outb(status, MOUSE_OUTPUT_PORT);
	outb(MOUSE_CMD_WRITE, MOUSE_COMMAND);
	outb(MOUSE_INTS_ON, MOUSE_OUTPUT_PORT); 
	outb(MOUSE_MAGIC_WRITE, MOUSE_COMMAND);
	outb(MOUSE_ENABLE_DEV, MOUSE_OUTPUT_PORT);
	x_pos = 0;
	y_pos = 0;
	s = 0;
	temp = (uint8_t) 'S';
	pre_ATTRIB = 0x7;
	paint = 0x2;
}


