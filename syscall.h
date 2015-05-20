#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "lib.h"
#include "rtc.h"
#include "keybrd.h"
#include "file_system_driver.h"
#include "types.h"
#include "paging.h"
#include "x86_desc.h"

#define MAX_FILE_NUM 8


typedef struct file_t{

	int32_t * f_op;
	int32_t inode;
	uint32_t file_position;
	uint32_t flags;	//check if it is in-use

} file_t;

//pcb structure
typedef struct pcb{

	file_t file_array[MAX_FILE_NUM];
	int parent;
	uint8_t pcb_exe_args[32];
	int32_t pcb_arg_size;
	uint8_t PID;
	uint32_t esp0;	//parent's esp0
	uint8_t ss0;	//parent's ss0
	uint32_t return_add;	//return address for halt 
	uint32_t esp;	//parent's esp
	uint32_t ebp;	//parent's ebp
	uint32_t sche_esp;
	uint32_t sche_ebp;
	uint32_t sche_esp0;
	uint32_t sche_ss0;
	// uint32_t sche_add;
} pcb_t;


int32_t halt (uint8_t status);
int32_t execute(const uint8_t * command);
int32_t read(int32_t fd, uint8_t * buf, int32_t nbytes);
int32_t write(int32_t fd, const uint8_t * buf, int32_t nbytes);
int32_t open(const uint8_t * filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t * buf, int32_t nbytes);
int32_t vidmap(uint8_t ** screen_start);
int32_t set_handler(int32_t signum, void * handler_address);
int32_t sigreturn(void);

int32_t find_available_fd();
void build_pcb(pcb_t * temp, int temp_pid);
void halt_fd();
void alert();
void play_sound (int freq_number, uint32_t duration);

int pid;
uint8_t global;
int pid_status[30];
extern int talk_init_flag;
extern int out_talk;
extern int talk_port;
extern int other_ports[];
extern int end_port;


#endif
