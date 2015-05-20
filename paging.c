#include "paging.h"

#define pe_size 1024	//entry size for both page directory or page table
#define pm_size 4096	//memory size for pages in page table --> 4kB
#define RW 0x00000002	//not present
#define present 0x00000003	//present
#define VIDEO_OFF 184	//video memory offset of page table
#define FOUR_MB_PRESENT_USER 0x87
#define USER_PAGE 32
#define FOUR_MB_PRESENT 0x83
#define VIDEO_DIR_OFFSET 37
#define USER_RW 0x07

uint32_t page_directory[pe_size] __attribute__((aligned(pm_size)));
uint32_t page_table[pe_size] __attribute__((aligned(pm_size)));

/*
*   void syscall_page(uint32_t addr)
*   Inputs: the address to map			
*   Return Value: none
*	Function: map the physical address and set to user level
*/
void syscall_page(uint32_t addr){

	page_directory[USER_PAGE] = addr | FOUR_MB_PRESENT_USER; 	//set to user level

	//TLB flusH
	asm volatile (												
 		"movl %%cr3, %%eax		\n	 	 	\
 		 movl %%eax, %%cr3"					\
 		: 									\
 		: 									\
 		: "memory", "cc");			


}

/*
*   void vid_page(uint32_t addr)
*   Inputs: the address to map			
*   Return Value: none
*	Function: map the video mem address to virtual address and set to user level
*/
void vid_page(uint32_t addr){

	page_directory[0] |= USER_RW;
	page_table[VIDEO_DIR_OFFSET] = addr | USER_RW; 	//set to user level

	//TLB flush
	asm volatile (									
 		"movl %%cr3, %%eax		\n	 	 	\
 		 movl %%eax, %%cr3"					\
 		: 									\
 		: 									\
 		: "memory", "cc");			
		
}

/*
*   void vid_new(uint32_t addr, int display_index)
*   Inputs: the address to map, and the offset to the displaying terminal	
*   Return Value: none
*	Function: map 2 pages after the video mem offset to virtual address and set to user level
*/
void vid_new(uint32_t addr, int display_index){

	page_table[VIDEO_OFF + display_index * 2] = addr | USER_RW; 	//set to user level

	//TLB flush
	asm volatile (												
 		"movl %%cr3, %%eax		\n	 	 	\
 		 movl %%eax, %%cr3"					\
 		: 									\
 		: 									\
 		: "memory", "cc");			
		
		
}

/*
*   void paging_init()
*   Inputs: none		
*   Return Value: none
*	Function: initialize the paging
*/
void paging_init(){

 	int i;	//for loop index

 	//initialize the page directory with all not present
 	for(i = 0; i < pe_size; i++){

 		page_directory[i] = RW;
 	}

 	//initialize the page table with all not present except the video memory
 	for(i = 0; i < pe_size; i++){

 		if(i == VIDEO_OFF)
 			page_table[i] = (i*pm_size) | 0x07;
 		else
 			page_table[i] = (i*pm_size) | RW;
 	}

 	//let first page table to be present in the page directory
 	page_directory[0] = ((unsigned int)page_table) | 0x07;

 	//let second page directory to be present, which is kernel
 	page_directory[1] = pe_size * pm_size | FOUR_MB_PRESENT; //starting from 4 Mb (0x400000) | (0x80 | 0x03)
 	
 	//using the asm volatile to output the paging
 	asm volatile (
 		"movl $page_directory, %%eax 	\n 	\
 		movl %%eax, %%cr3 				\n	\
 		movl %%cr4, %%eax				\n	\
 		orl $0x00000010, %%eax			\n	\
 		movl %%eax, %%cr4				\n	\
 		movl %%cr0, %%eax				\n	\
 		orl $0x80000000, %%eax			\n	\
 		movl %%eax, %%cr0"					\
 		: 									\
 		: "g"(page_directory)				\
 		: "memory", "cc", "eax");			

 
 }
