#include "sche.h"
#include "syscall.h"
#include "interrupt_handler.h"

#define FOUR_MB 0x0400000
#define EIGHT_MB 0x0800000
#define EIGHT_KB 0x2000
#define VIDEO_MEM 0xB8000
#define SCREEN_SIZE 4096
#define max_terminal 3


int terminal_pid[];
int cur_index;
int next_index;
pcb_t * cur;
int init_flag;

/*
*   void sche_init()
*   Inputs: void
*   Return Value: void
*	Function: enable and setting up the scheduler handler
*/
void sche_init(){

	int i;
	//setting up the initial pid for each terminal
	for(i = 0; i < max_terminal; i++){
		terminal_pid[i] = -1;
	}
	talk_init_flag = 1;
	//initialize the indexes and flags
	cur_index = -1;
	next_index = 0;
	init_flag = 1;
}

/*
*   void scheduling()
*   Inputs: void
*   Return Value: void
*	Function: opens up shells for each terminal and run programs simutaneously on each corresponding terminal
*/
void scheduling(){

	//constantly changing the running index as a cycle
	next_index = (cur_index + 1 ) % max_terminal;

	//call paging and set up new pages for video mem
	vid_new(VIDEO_MEM + 2 * SCREEN_SIZE * next_index, next_index);
	uint32_t esp, ebp;

	//moves esp and ebp to registers
	asm volatile(
				"movl %%esp, %0 	\n \
				 movl %%ebp, %1"
				:"=r"(esp), "=r"(ebp)
				:
				:"memory"
				);

	//conditons for calculating different pcb addresses based on pid
	if(terminal_pid[next_index] == -1 && pid == -1){
		cur = (pcb_t *) (EIGHT_MB - EIGHT_KB*(next_index+1));

	}else if (terminal_pid[next_index] == -1 && pid != -1){
		cur = (pcb_t *) (EIGHT_MB - EIGHT_KB*(terminal_pid[cur_index]+1));
	} 
	else{
		cur = (pcb_t *) (EIGHT_MB - EIGHT_KB*(terminal_pid[cur_index]+1));
	}

	//saving the old esp and ebp to the pcb
	cur->sche_esp = esp;
	cur->sche_ebp = ebp;

	//check if the terminal is first time run
	if(terminal_pid[next_index] == -1){
		//opens up a shell if it is first run and set up flags and indexes
		init_flag = 1;
		cur_index = next_index;

		//send eoi when done
		send_eoi(0);
		execute((uint8_t *) "shell");

	}
	else{
		//if not first time, calculate its pcb address and change indexes
		cur = (pcb_t *) (EIGHT_MB - EIGHT_KB*(terminal_pid[next_index] + 1));
		cur_index = next_index;

		//call paging after calculated the new addresses from the executed pid
		uint32_t addr = EIGHT_MB + FOUR_MB * (cur -> PID); 
		syscall_page(addr);

	}

	//restore the esp and ebp and change ss0 and esp0
	tss.ss0 = KERNEL_DS;
	tss.esp0 = EIGHT_MB - EIGHT_KB*(cur->PID) - 4;
	esp = cur->sche_esp;
	ebp = cur->sche_ebp;

	//move esp and ebp to the registers
	send_eoi(0);
	asm volatile(
		"movl %0, %%esp  \n 	\
		movl %1, %%ebp"
		:
		:"r"(esp), "r"(ebp)
		:"memory"
		);
	return;
}
