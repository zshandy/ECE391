#include "syscall.h"
#include "sche.h"
#include "interrupt_handler.h"

typedef int32_t function();

//file operation
int32_t rtc_file_op[4] = {(int32_t) rtc_read, (int32_t) rtc_write, (int32_t) rtc_open, (int32_t) rtc_close};
int32_t terminal_file_op[4] = {(int32_t) terminal_read, (int32_t) terminal_write, (int32_t) terminal_open, (int32_t) terminal_close};
int32_t directory_file_op[4] = {(int32_t) dir_read, (int32_t) dir_write, (int32_t) dir_open, (int32_t) dir_close};
int32_t file_file_op[4] = {(int32_t) file_read, (int32_t) file_write, (int32_t) file_open, (int32_t) file_close};

#define present 0x00000003	//present
#define FOUR_MB_PRESENT 0x83
#define FOUR_MB 0x0400000
#define EIGHT_MB 0x0800000
#define EIGHT_KB 0x2000
#define PROGRAM_IMG_ADDR 0x08048000 
#define PROGRAM_IMG_OFFSET 0x00048000
#define USER_ESP 0x0083FFFFC
#define cur_pcb(pid) (pcb_t *) (EIGHT_MB - EIGHT_KB*(terminal_pid[cur_index]+1)) 	//function to help get current pcb
#define buf_len 32
#define MB_128 0x08000000
#define MB_132 0x08400000
#define VIDEO_MEM 0xB8000
#define FOUR_KB 0x1000
#define VIDEO_DIR_OFFSET 37
#define BUFFER_SIZE 1024
#define fd_min 2
#define fd_max 7

int talk_init_flag = 0;
int out_talk = 0;
int talk_port = 0;
int end_port = 9;
int other_ports[9];
int pid = -1;		//indicate the current pid
uint8_t global = -1; 	//to store the status
int pid_status[];
uint8_t status = 0;

void play_sound (int freq_number, uint32_t duration){


	//send the freqency.
	outb((uint8_t)(freq_number & 0xff), 0x42);
	outb((uint8_t)((freq_number >> 8)& 0xff), 0x42);

	//start the beep
	uint8_t tmp = inb(0x61);
	if (tmp != (tmp | 3)){
		outb(tmp | 3, 0x61);
	}
	int32_t i = duration;
	int32_t j = 65535;
	//calculating how long it lasts
	for (i = duration; i > 0; i--){
		while (j > 0){
			j--;
		}
		j = 65535;
	}

	uint8_t temp = inb(0x61) & 0xFC;
	outb(temp, 0x61);
}

/*
*   int32_t halt (uint8_t status)
*   Inputs: status			
*   Return Value: never return or 0 on success
*	Function: halt the current process
*/
int32_t halt (uint8_t status){

	//restore the esp0 and ss0
	pcb_t * cur = (pcb_t *) (EIGHT_MB - EIGHT_KB*(terminal_pid[cur_index]+1));
	tss.esp0 = cur->esp0;
	tss.ss0 = cur->ss0;
	pid = cur->PID;
	/*
	puts("halt: ", 1);
	putc((uint8_t)(pid+0x30), 1);
	puts(" ", 1);
	putc((uint8_t)(cur->parent+0x30), 1);
	puts(" ", 1);
	putc((uint8_t)(cur_index+0x30), 1);
	puts("\n", 1);
	*/
	//check whether it is first process
	halt_fd();
	pid_status[pid] = 0;
	pid = cur->parent;
	terminal_pid[cur_index] = pid;
	//printf("halt2:%d\n", pid);
	//printf("halt cur: %x, cur_pid: %x\n", (int)cur, cur->PID);
	//printf("halt esp: %x\n", cur->esp);
	//printf("halt ebp: %x\n", cur->ebp);
	if(pid == -1){
		execute((uint8_t *) "shell");
	}
	//reset the paging
	global = status;
	uint32_t addr = EIGHT_MB + FOUR_MB * pid;
	syscall_page(addr);

	// printf("exit to parent, now pid: %d\n", pid);
	
	//jump to parent process
	asm volatile (
        		"movl %0, %%esp			\n\
        		movl %1, %%ebp			\n\
        		jmp *%2"
       			 :
        		 :"r"(cur->esp), "r"(cur->ebp), "r" (cur->return_add)
       			 );

	return 0;
}

void alert()
{
		outb(182, 0x43);
		play_sound(4560, 1000);
}
/*
*   void halt_fd()
*   Inputs: none		
*   Return Value: none
*	Function: halt the current fd
*/

void halt_fd(){
	int i;
	for(i = fd_min; i < MAX_FILE_NUM; i++)
		(cur_pcb(pid))->file_array[i].flags = 0;
}

/*
*   int32_t execute(const uint8_t * command)
*   Inputs: the command to execute		
*   Return Value: the status on success and -1 on failure or error condition
*	Function: execute the given command
*/

int32_t execute(const uint8_t * command){
	cli();
	if(talk_init_flag == 0){
		disable_irq(0);
		hd_read_user(hd_buffer, 16, 2);
		hd_buffer[512] = '\0';
		int user_flag = 0;
		if(hd_buffer[0] != '\0'){
			user_flag = 1;
		}
		if(user_flag){
			int right = 0;
			while(!right){
				right = 1;
				puts("Enter the password: ", 1);
				sti();
				int count = terminal_read(-1, (uint8_t*)password, 513);
				cli();
				if(count > 0){
					password[count-1] = '\0';
				}
				else{
					right = 0;
					puts("Incorrect password!\n", 1);
					continue;
				}
				int i = 0;
				for(; i < count; i++){
					if(password[i] != hd_buffer[i]){
						right = 0;
						puts("Incorrect password!\n", 1);
						break;
					}
				}
			}
		}
		else{
			int out = 0;
			while(!out){
				puts("Please set a new password: ", 1);
				sti();
				int count = terminal_read(-1, (uint8_t*)password, 513);
				cli();
				if(count > 0){
					password[count-1] = '\0';
				}
				else{
					puts("There must be some characters or numbers in the password!\n", 1);
					continue;
				}
				hd_write_user(password, 16, 2);
				puts("Password set, good to chat!\n", 1);
				out = 1;
			}
		}
		int i;
		for(i = 0; i < end_port; i++){
			other_ports[i] = 0;
		}
		puts("You can now chat with our users now!\n", 1);
		user_flag = 0;
		talk_port = 0;
		for(i = 0; i < end_port; i++){
			hd_read_user(hd_buffer, 16, i+3);
			hd_buffer[512] = '\0';
			if(hd_buffer[0] == 0 && user_flag == 0){
				user_flag = 1;
				hd_buffer[0]= (uint8_t)1;
				hd_write_user(hd_buffer, 16, i+3);
				talk_port = i;
				puts("Hi, user number ", 1);
				putc((uint8_t)(i+1+0x30), 1);
				puts(" \n", 1);
			}
			if(hd_buffer[0] != 0){
				other_ports[i] = 1;
			}
		}
		
		int talk_flag = 1;
		status = 1;
		enable_irq(0);
		sti();
		while(!out_talk){
			if(talk_flag){
				int count = terminal_read(0, (uint8_t*)hd_buffer+1, 511);
				//time_out = 0;
				if(count > 0){
					status++;
					if(status == 10){
						status = 2;
					}
					hd_buffer[0] = status;
					hd_write_user(hd_buffer, 16, talk_port+3);
					talk_flag = 0;
				}
				else{
					puts("You must enter something!\n", 1);
				}
			}
			else{
				talk_flag = 1;
			}
		}
		talk_init_flag = 1;
		cli();
	}
	
	// printf("HI\n");
	int32_t cmd_len = 0, args_len = 0;
	uint8_t exe_file[128];
	uint8_t exe_args[128];
	dentry_t temp;
	uint32_t esp;
	uint32_t ebp;
	int k;
	
	if(command == NULL)
		return -1;
	//store the esp and ebp
    asm volatile (
    			 "movl %%esp, %0		\n 		\
        		 movl %%ebp, %1"
       			 :"=r"(esp), "=r"(ebp)
        		 :
       			 :"memory" 
       			 );

    // printf("command: %s\n", command);
    //check whether it is legal command
    for(k = 0; k < buf_len; k++){
    	exe_file[k] = '\0';
    	exe_args[k] = '\0';
    }
	if(command[0] == '\0' || command[0] == ' '){
		puts("Exception: illegal command!! No filename!!\n", 1);
		alert();
		return -1;
	}

	//store the command
	while(command[cmd_len] != '\0' && command[cmd_len] != ' ' && command[cmd_len] != '\n'){
		exe_file[cmd_len] = command[cmd_len];
		cmd_len++;
	}

	// printf("leng: %d\n", cmd_len);
	//clean the exe_file buf
	exe_file[cmd_len] = '\0';
	// printf("command: %s\n", exe_file);

	//check the command again
	if(cmd_len > buf_len){
		puts("Exception: illegal command!! Filename too long!!\n", 1);
	    alert();

		return -1;
	}

	cmd_len++;

	//store the argument
	while(command[cmd_len+args_len] != '\0' && command[cmd_len+args_len] != '\n'){
		exe_args[args_len] = command[cmd_len+args_len];
		args_len++;
	}

	// printf("%d, %s\n", args_len, exe_args);
	if(args_len > 32){
		printf("arg: %s, %d\n", exe_args, args_len);
		puts("Exception: argument too long!!\n", 1);
		alert();

		return -1;
	}

	//chech whether it is executable
	int f_type;
	f_type = read_dentry_by_name(exe_file, &temp);
	if(f_type == -1){
		puts("Exception: no file to execute!!\n", 1);
		alert();
		return -1;
	}

	//check if it is ELF
	uint8_t ELF_FLAG[4];
	f_type = read_data(temp.inode_num, 0, ELF_FLAG, 4);
	if(f_type == -1){
		puts("Exception: Unable to read the data!!\n", 1);
		alert();
		return -1;
	}


	if(ELF_FLAG[0] != 0x7f || ELF_FLAG[1] != 0x45 || ELF_FLAG[2] != 0x4c || ELF_FLAG[3] != 0x46){
		puts("Exception: Not the ELF!!\n", 1);
		alert();
		return -1;
	}
	//handling maximum number of shells
	if(pid == 29){
		puts("Hit the maximum shell, we just have at most 29 shells\n", 1);
		alert();
		return -1;
	}
	int temp_pid;
	if(init_flag == 1){
		temp_pid = terminal_pid[cur_index];
		int i;
		for(i = 0; i < 30; i++){
			if(pid_status[i] == 0){
				pid_status[i] = 1;
				pid = i;
				break;
			}
		}
		terminal_pid[cur_index] = pid;
	}
	else{
		temp_pid = terminal_pid[display_index];
		int i;
		for(i = 0; i < 30; i++){
			if(pid_status[i] == 0){
				pid_status[i] = 1;
				pid = i;
				break;
			}
		}
		terminal_pid[display_index] = pid;
	}
	init_flag = 0;
	// printf("current pid: %d\n\n", pid);

	//set up paging
	uint32_t addr = EIGHT_MB + FOUR_MB * pid; 
	syscall_page(addr);

	uint8_t USER_EIP[4];
	int eip = 0;
	// int a = PROGRAM_IMG_ADDR;

	//file loader
	read_data(temp.inode_num, 0, (uint8_t *) PROGRAM_IMG_ADDR, FOUR_MB - PROGRAM_IMG_OFFSET);
	read_data(temp.inode_num, 24, USER_EIP, 4);
	//getting user eip
	int i;
	for(i = 0; i < 4; i++){
		eip |= (USER_EIP[i] << (i*8));  //bits 27-24
	}
	//printf("exe eip: %x\n", eip);
	//printf("exe esp: %x\n", esp);
	//printf("exe ebp: %x\n", ebp);
	//get the pcb with corressponding kernel address 
	pcb_t * new_pcb = (pcb_t *) (EIGHT_MB - EIGHT_KB*(pid+1)); //get the current pcb
	build_pcb(new_pcb, temp_pid);

	//getting the arg
	for(i = 0; i < args_len; i++){
		new_pcb->pcb_exe_args[i] = exe_args[i];
	}
	for(i = args_len; i < 32; i++){
		new_pcb->pcb_exe_args[i] = '\0';
	}
	new_pcb->pcb_arg_size = args_len;

	//store the parent esp and ebp
	new_pcb->esp = esp;
	new_pcb->ebp = ebp;


	//store the return address for halt
    asm volatile (

    			 "leal halt_ret, %%eax		\n 	 	\
        		 movl %%eax, %0"
       			 :"=m"(new_pcb->return_add)
        		 :
       			 :"eax", "memory" 
       			 );
	asm volatile (

			 "leal halt_ret, %%eax		\n 	 	\
			 movl %%eax, %0"
			 :"=m"(new_pcb->return_add)
			 :
			 :"eax", "memory" 
			 );

    //set the ss0 and esp0
	tss.ss0 = KERNEL_DS;
	tss.esp0 = EIGHT_MB - EIGHT_KB*pid - 4;
	//putc((uint8_t)(cur_index+0x30), 1);
	//puts("\n", 1);
	//printf("exe:%d %d\n", pid, cur_index);
	//sti();
	//process of context switch
		//puts("this ", 1);
		//putc((uint8_t)(cur_index+0x30), 1);
		//puts("\n", 1);
	asm volatile (
		 "movw %0, %%ax 				\n 		\
		 movw %%ax, %%ds 			\n 		\
		 movw %%ax, %%es 			\n 		\
		 movw %%ax, %%fs 			\n 		\
		 movw %%ax, %%gs 			\n 		\
		 pushl %0				\n 		\
		 pushl %1 	\n 		\
		 pushfl 				\n  	\
		 popl %%eax 			\n 		\
		 orl %2, %%eax 			\n 		\
		 pushl %%eax 			\n 		\
		 pushl %3 		\n 		\
		 pushl %4 	 					"
		 : 								
		 :"i"(USER_DS), "i"(USER_ESP), "r"(0x200), "i"(USER_CS), "r"(eip)					
		 :"memory", "cc", "eax");

	//while(1 && cur_index == 1){}
	//iret
	asm volatile("iret");
	
	//halt will jump to here
	asm volatile("halt_ret:");

	//putc((uint8_t)0x30,1);
	return global;
}


/*
*   void build_pcb(pcb_t * temp)
*   Inputs: current pcb		
*   Return Value: none
*	Function: build the pcb 
*/

void build_pcb(pcb_t * temp, int temp_pid){

	temp->PID = pid; //store the current pid

	//stdin
	temp->file_array[0].f_op = (int32_t *) terminal_file_op;
	temp->file_array[0].inode = 0;
	temp->file_array[0].file_position = 0;
	temp->file_array[0].flags = 1;

	//stdout
	temp->file_array[1].f_op = (int32_t *) terminal_file_op;
	temp->file_array[1].inode = 0;
	temp->file_array[1].file_position = 0;
	temp->file_array[1].flags = 1;

	//store the parent's esp0 and ss0
	temp->esp0 = tss.esp0;
	temp->ss0 = tss.ss0;

	//store parent's pid
	temp->parent = temp_pid;			
	/*
	puts("build: ", 1);
	putc((uint8_t)(temp_pid+0x30), 1);
	puts(" ", 1);
	putc((uint8_t)(pid+0x30), 1);
	puts(" ", 1);
	putc((uint8_t)(display_index+0x30), 1);
	puts("\n", 1);
	*/
}


/*
*   int32_t read(int32_t fd, uint8_t * buf, int32_t nbytes)
*   Inputs: fd, buffer and number of bytes to be read		
*   Return Value: return each function on success and -1 on failure
*	Function: call the rtc/filesystem/terminal read function
*/

int32_t read(int32_t fd, uint8_t * buf, int32_t nbytes){
	// function * function_read;
	sti();
	if(fd > fd_max || fd < 0 || fd == 1 || buf == NULL) 	//check if it is a valid fd, and it cannot do stdout when read
		return -1;

	if((cur_pcb(pid))->file_array[fd].flags == 0) 	//check if it is in-use
		return -1;
	return ((function *)((cur_pcb(pid))->file_array[fd].f_op[0]))((int32_t) fd, (uint8_t *) buf, (int32_t) nbytes);		//call the read function of correspond file_type 

}

/*
*   int32_t write(int32_t fd, const uint8_t * buf, int32_t nbytes)
*   Inputs: fd, buffer and number of bytes to be written		
*   Return Value: return each function on success and -1 on failure
*	Function: call the rtc/filesystem/terminal write function
*/

int32_t write(int32_t fd, const uint8_t * buf, int32_t nbytes){

	if(fd > fd_max || fd < 0 || fd == 0 || buf == NULL) 	//check if it is a valid fd, and it cannot do stdin when write
		return -1;

	if((cur_pcb(pid))->file_array[fd].flags == 0)		//check if it is in-use
		return -1;

	// function_write = (function *)((cur_pcb(pid))->file_arr[fd].f_op[1]);
	return ((function *)((cur_pcb(pid))->file_array[fd].f_op[1]))((int32_t) fd, (uint8_t *) buf, (int32_t) nbytes);		//call the write function of correspond file_type 
		
}

/*
*   int32_t open(const uint8_t * filename)
*   Inputs: the filename	
*   Return Value: return fd number on success and -1 on failure
*	Function: call open function according to file type
*/

int32_t open(const uint8_t * filename){
	
	int32_t fd;
	dentry_t temp;
	// function * function_open;

	if(filename == NULL)
		return -1; 
	fd = find_available_fd();	//check if there is a valid fd
	if(fd == -1)	
		return -1;
	// printf("fd: %d\n", fd);
	uint32_t f_type;
	f_type = read_dentry_by_name(filename, &temp);	//check if it is a valid filename

	if(f_type == -1)	//not a valid filename
		return -1;

	//initialization of different file_type
	switch(temp.file_type){
		case 0:
			(cur_pcb(pid))->file_array[fd].f_op = (int32_t *) rtc_file_op;
			(cur_pcb(pid))->file_array[fd].inode = 0;
			(cur_pcb(pid))->file_array[fd].file_position = 0;
			(cur_pcb(pid))->file_array[fd].flags = 1;
			((function *)((cur_pcb(pid))->file_array[fd].f_op[2]))((uint8_t *) filename);		//call the open function of the corresponding file_type
			return fd;

		case 1:
			(cur_pcb(pid))->file_array[fd].f_op = (int32_t *) directory_file_op;
			(cur_pcb(pid))->file_array[fd].inode = 0;
			(cur_pcb(pid))->file_array[fd].file_position = 0;
			(cur_pcb(pid))->file_array[fd].flags = 1;
			((function *)((cur_pcb(pid))->file_array[fd].f_op[2]))((uint8_t *) filename);		//call the open function of the corresponding file_type
			return fd;

		case 2:
			(cur_pcb(pid))->file_array[fd].f_op = (int32_t *) file_file_op;
			(cur_pcb(pid))->file_array[fd].inode = temp.inode_num;
			(cur_pcb(pid))->file_array[fd].file_position = 0;
			(cur_pcb(pid))->file_array[fd].flags = 1;
			((function *)((cur_pcb(pid))->file_array[fd].f_op[2]))((uint8_t *) filename);		//call the open function of the corresponding file_type
			return fd;

		default:
			return -1;
	}
	
}

/*
*   int32_t find_available_fd()
*   Inputs: none
*   Return Value: return 1 on success and -1 on failure
*	Function: find the next available fd number
*/
int32_t find_available_fd(){

	int i;
	for(i = fd_min; i < MAX_FILE_NUM; i++){
		if((cur_pcb(pid))->file_array[i].flags == 0)
			return i;	//return valid fd
	}
	return -1; 	//no valid fd
}

/*
*   int32_t close(int32_t fd)
*   Inputs: the fd number	
*   Return Value: return fd number on success and -1 on failure
*	Function: call close function on the given fd number
*/

int32_t close(int32_t fd){

	// function * function_close;

	if(fd < 2 || fd >fd_max)		//check if it is a valid fd
		return -1;

	if((cur_pcb(pid))->file_array[fd].flags == 0) 	//check if it is in-use
		return -1;

	((function *)((cur_pcb(pid))->file_array[fd].f_op[3]))((int32_t) fd);
	// printf("hello\n");
	(cur_pcb(pid))->file_array[fd].file_position = 0;	//reset the file_position
	(cur_pcb(pid))->file_array[fd].flags = 0;		//reset the flags
	(cur_pcb(pid))->file_array[fd].f_op = NULL;		//reset the f_op


	return 0;		//successfully return 
}

/*
*   int32_t getargs(uint8_t * buf, int32_t nbytes)
*   Inputs: buffer and number of bytes	
*   Return Value: return 0 on success and -1 on failure
*	Function: get the argument from the keyboard buffer
*/
int32_t getargs(uint8_t * buf, int32_t nbytes){

	if(buf == NULL)
		return -1;
	if((cur_pcb(pid))->pcb_arg_size > nbytes)					//check the arg size
		return -1;
	// printf("argument: %s\n", (cur_pcb(pid))->pcb_exe_args);
	int i;
	for(i = 0; i < BUFFER_SIZE; i++)							//reinitialize the buffer
		buf[i] = '\0';
	for(i = 0; i < (cur_pcb(pid))->pcb_arg_size; i++)			//put the arg into the buffer
		buf[i] = (cur_pcb(pid))->pcb_exe_args[i];
	// printf("arugument put into buf: %s\n", buf);
	return 0;													//return 0 on success
}

/*
*   int32_t vidmap(uint8_t ** screen_start)
*   Inputs: where the video_mem start
*   Return Value: return 0 on success and -1 on failure
*	Function: map video memory to other virtual address
*/
int32_t vidmap(uint8_t ** screen_start){

	if(screen_start < (uint8_t **) MB_128 || screen_start >= (uint8_t **) MB_132)
		return -1;												//check the screen address
	vid_new(0xB8000+4096*2*cur_index, cur_index);										//map video mem to virtual address
	(*screen_start) = (uint8_t *) ((184+cur_index*2) * FOUR_KB); //assign the address to screen start

	return 0;													//return 0 on success
	
}

int32_t set_handler(int32_t signum, void * handler_address){

	return -1;
}

int32_t sigreturn(void){
	
	return -1;
}
