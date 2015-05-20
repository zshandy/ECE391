/*This is our keybrd.c which take inputs from keyboard to print to the screen*/

#include "keybrd.h"
#include "interrupt_handler.h"
#include "sche.h"

#define END_OF_BUF '\0'
#define KEY_LIMIT 127

int can_read = 1;
uint8_t key_buf[BUFFER_LIMIT][SCREEN_LIMIT];
uint8_t out_buf[BUFFER_LIMIT][SCREEN_LIMIT];


/*
*   int32_t terminal_read (int32_t fd, uint8_t* usr_buf, int32_t usr_buf_len)
*   DESCRIPTION: copy data from one buffer to another
*   Inputs: length			--  length of data we want to copy
*   Return Value: none
*	Function: go through all data in out buffer and copy everything into key buffer
*/
int32_t terminal_read (int32_t fd, uint8_t* buf, int32_t nbytes){
	while (can_read || (display_index != cur_index && talk_init_flag));									//while readable and displaying index is the same as cur_index, go ahead
	can_read = 1;
	uint32_t i;
    if (nbytes <= 0)															//if the end of buffer is reached, return 0
    	return 0;
    //printf("terminal:%s\n",buf);
	for (i = 0; i < nbytes && out_buf[i][display_index] != END_OF_BUF && i < KEY_LIMIT; i++)	//loop through the given length of the buffer when it is no the end or reached limit
		buf[i] = out_buf[i][display_index];					     							//transfer all out_buffer to user_buffer value and clear out_buffer
	
	out_buf[0][display_index] = 0;
	buf[i] = END_OF_BUF;
	return i;																		//return the number of bytes read
}

/* 
 * terminal_write
 *   DESCRIPTION: The terminal_write function will print the buffer to screen
 *   INPUTS: fd				-- file descriptor indexprint keyboard typed information 
 * 				  on the terminal
 *			 user_buf		-- buffer use to write out data 
 *		     userbuf_count	-- number of data stored  
 *   OUTPUTS: none
 *   RETURN VALUE: i 		-- number of data written out 
 *   SIDE EFFECTS: none
 */

int32_t terminal_write(int32_t fd, const uint8_t* buf, int32_t nbytes){
	if((buf == NULL) || (nbytes < 0))							//if the buffer given to write is NULL or the end is reached, return -1
		return -1;

	uint32_t i;
	for (i = 0; i < nbytes; i++)
		putc(buf[i], 0);												// print data stored in user buffer to screen
	return nbytes;
}

/* 
 * terminal_open
 *   DESCRIPTION: The terminal_open function will open the terminal 
 *   INPUTS: command     --  command to open terminal 
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
 
int32_t terminal_open(const uint8_t * filename) {
	return 0; 															//open success, return 0
}

/* 
 * terminal_close
 *   DESCRIPTION: The terminal_close function will close the terminal 
 *   INPUTS: fd        -- file descriptor index
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */
 
int32_t terminal_close(int32_t fd){
	return -1;															//simply return -1 on closing terminal
}  

/*
* void keybrd_init();
*   Inputs: void
*   Return Value: none
*	Function: Initialize the terminal 
*/
void keybrd_init(){
	clear();				
	int i,j;
	display_index = 0;
	for (i = 0; i < BUFFER_LIMIT; i++){
		for (j = 0; j < SCREEN_LIMIT; j++){
		out_buf[i][j] = 0;						// initialize out buffer
		key_buf[i][j] = 0;						// initialize key buffer
		}
	}
	//for (i = 0; i < SCREEN_LIMIT; i++)
	// keypressed_count[i] = 0;					// initialize keypressed_count
	return;
}

