/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "rtc.h"
#include "exception.h"
#include "keybrd.h"
#include "interrupt_wrapper.h"
#include "interrupt_handler.h"
#include "paging.h"
#include "exception_wrappers.h"
#include "file_system_driver.h"
#include "syscall.h"
#include "syscall_wrapper.h"
#include "sche.h"
#include "pit.h"
//#include "system_call_wrapper.h"

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

#define EXCEPTION_NUM 0x20
#define RTC_ENTRY 0x28
#define KEYBRD_ENTRY 0x21
#define SYSCALL_ENTRY 0X80
#define PIT_ENTRY 0x20
#define MOUSE_VECTOR 0x2C

#define TEST_READ_BUFFER_SIZE 128
#define TEST_WRITE_BUFFER_SIZE 27

extern void init_rtc();
extern void init_exception(void);

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
	uint32_t file_system_start;
	multiboot_info_t *mbi;

	/* Clear the screen. */
	clear();

	/* Am I booted by a Multiboot-compliant boot loader? */
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
	{
		printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
		return;
	}

	/* Set MBI to the address of the Multiboot information structure. */
	mbi = (multiboot_info_t *) addr;

	/* Print out the flags. */
	printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

	/* Are mem_* valid? */
	if (CHECK_FLAG (mbi->flags, 0))
		printf ("mem_lower = %uKB, mem_upper = %uKB\n",
				(unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

	/* Is boot_device valid? */
	if (CHECK_FLAG (mbi->flags, 1))
		printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

	/* Is the command line passed? */
	if (CHECK_FLAG (mbi->flags, 2))
		printf ("cmdline = %s\n", (char *) mbi->cmdline);

	if (CHECK_FLAG (mbi->flags, 3)) {
		int mod_count = 0;
		int i;
		module_t* mod = (module_t*)mbi->mods_addr;

		file_system_start = mod->mod_start;  // store mod_start as start address of file system
		
		while(mod_count < mbi->mods_count) {
			printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
			printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
			printf("First few bytes of module:\n");
			for(i = 0; i<16; i++) {
				printf("0x%x ", *((char*)(mod->mod_start+i)));
			}
			printf("\n");
			mod_count++;
			mod++;
		}
	}
	/* Bits 4 and 5 are mutually exclusive! */
	if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
	{
		printf ("Both bits 4 and 5 are set.\n");
		return;
	}

	/* Is the section header table of ELF valid? */
	if (CHECK_FLAG (mbi->flags, 5))
	{
		elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

		printf ("elf_sec: num = %u, size = 0x%#x,"
				" addr = 0x%#x, shndx = 0x%#x\n",
				(unsigned) elf_sec->num, (unsigned) elf_sec->size,
				(unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
	}

	/* Are mmap_* valid? */
	if (CHECK_FLAG (mbi->flags, 6))
	{
		memory_map_t *mmap;

		printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
				(unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
		for (mmap = (memory_map_t *) mbi->mmap_addr;
				(unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
				mmap = (memory_map_t *) ((unsigned long) mmap
					+ mmap->size + sizeof (mmap->size)))
			printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
					"     type = 0x%x,  length    = 0x%#x%#x\n",
					(unsigned) mmap->size,
					(unsigned) mmap->base_addr_high,
					(unsigned) mmap->base_addr_low,
					(unsigned) mmap->type,
					(unsigned) mmap->length_high,
					(unsigned) mmap->length_low);
	}

	/* Construct an LDT entry in the GDT */
	{
		seg_desc_t the_ldt_desc;
		the_ldt_desc.granularity    = 0;
		the_ldt_desc.opsize         = 1;
		the_ldt_desc.reserved       = 0;
		the_ldt_desc.avail          = 0;
		the_ldt_desc.present        = 1;
		the_ldt_desc.dpl            = 0x0;
		the_ldt_desc.sys            = 0;
		the_ldt_desc.type           = 0x2;

		SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
		ldt_desc_ptr = the_ldt_desc;
		lldt(KERNEL_LDT);
	}

	/* Construct a TSS entry in the GDT */
	{
		seg_desc_t the_tss_desc;
		the_tss_desc.granularity    = 0;
		the_tss_desc.opsize         = 0;
		the_tss_desc.reserved       = 0;
		the_tss_desc.avail          = 0;
		the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
		the_tss_desc.present        = 1;
		the_tss_desc.dpl            = 0x0;
		the_tss_desc.sys            = 0;
		the_tss_desc.type           = 0x9;
		the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

		SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

		tss_desc_ptr = the_tss_desc;

		tss.ldt_segment_selector = KERNEL_LDT;
		tss.ss0 = KERNEL_DS;
		tss.esp0 = 0x800000 - 4;
		ltr(KERNEL_TSS);
	}

///////////////////////////////

	/* Initialize first 20 IDT entries for exceptions */
	/* Initialize interrupt descriptor entries defined in x86_desc.h*/
	int i;
	for (i=0; i<EXCEPTION_NUM; i++)
	{

		idt[i].seg_selector = KERNEL_CS;    // set up elements in the exception entry
		idt[i].reserved4 = 0;			
		idt[i].reserved3 = 0;		
		idt[i].reserved2 = 1;
		idt[i].reserved1 = 1;
		idt[i].size = 1;
		idt[i].reserved0 = 0;
		idt[i].dpl = 0;          
		idt[i].present = 1;

		SET_IDT_ENTRY(idt[i], reserved);
	}

	// /* set 20 idt entries for exceptions */
    init_exception();

	// set idt entry for RTC
	{ 
		idt[RTC_ENTRY].seg_selector = KERNEL_CS;   // set up elements in the RTC entry
		idt[RTC_ENTRY].reserved4 = 0;			
		idt[RTC_ENTRY].reserved3 = 0;		
		idt[RTC_ENTRY].reserved2 = 1;
		idt[RTC_ENTRY].reserved1 = 1;
		idt[RTC_ENTRY].size = 1;
		idt[RTC_ENTRY].reserved0 = 0;
		idt[RTC_ENTRY].dpl = 0;          
		idt[RTC_ENTRY].present = 1;

		SET_IDT_ENTRY(idt[RTC_ENTRY], rtc_wrapper);   // set idt entry for RTC
	}

	// set idt entry for keyboard
	{
		idt[KEYBRD_ENTRY].seg_selector = KERNEL_CS;   // set up elements in the keyboard entry
		idt[KEYBRD_ENTRY].reserved4 = 0;			
		idt[KEYBRD_ENTRY].reserved3 = 0;		
		idt[KEYBRD_ENTRY].reserved2 = 1;
		idt[KEYBRD_ENTRY].reserved1 = 1;
		idt[KEYBRD_ENTRY].size = 1;
		idt[KEYBRD_ENTRY].reserved0 = 0;
		idt[KEYBRD_ENTRY].dpl = 0;          
		idt[KEYBRD_ENTRY].present = 1;

		SET_IDT_ENTRY(idt[KEYBRD_ENTRY], keybrd_wrapper);   // set idt entry for keyboard

	} 

	//set idt entry for system call
	{
		idt[SYSCALL_ENTRY].seg_selector = KERNEL_CS;		
		idt[SYSCALL_ENTRY].reserved4 = 0;
		idt[SYSCALL_ENTRY].reserved3 = 0;
		idt[SYSCALL_ENTRY].reserved2 = 1;		
		idt[SYSCALL_ENTRY].reserved1 = 1;
		idt[SYSCALL_ENTRY].size = 1;
		idt[SYSCALL_ENTRY].reserved0 = 0;
		idt[SYSCALL_ENTRY].dpl = 3;
		idt[SYSCALL_ENTRY].present = 1;
		SET_IDT_ENTRY(idt[SYSCALL_ENTRY], syscall_wrapper);
	}
	
	{
		idt[MOUSE_VECTOR].seg_selector = KERNEL_CS;		
		idt[MOUSE_VECTOR].reserved4 = 0;
		idt[MOUSE_VECTOR].reserved3 = 0;
		idt[MOUSE_VECTOR].reserved2 = 1;		
		idt[MOUSE_VECTOR].reserved1 = 1;
		idt[MOUSE_VECTOR].size = 1;
		idt[MOUSE_VECTOR].reserved0 = 0;
		idt[MOUSE_VECTOR].dpl = 0;
		idt[MOUSE_VECTOR].present = 1;
		SET_IDT_ENTRY(idt[MOUSE_VECTOR], mouse_linkage);      
	}

	// set idt entry for pit
	{
		idt[PIT_ENTRY].seg_selector = KERNEL_CS;		
		idt[PIT_ENTRY].reserved4 = 0;
		idt[PIT_ENTRY].reserved3 = 0;
		idt[PIT_ENTRY].reserved2 = 1;		
		idt[PIT_ENTRY].reserved1 = 1;
		idt[PIT_ENTRY].size = 1;
		idt[PIT_ENTRY].reserved0 = 0;
		idt[PIT_ENTRY].dpl = 3;
		idt[PIT_ENTRY].present = 1;
		SET_IDT_ENTRY(idt[PIT_ENTRY], pit_wrapper);
	}




/////////////////////////////////

	// keyboard_handler(); 

	 //	test divide_error
	
	 // i=1/0;  

	// initialize devices
	// enable associated interrupts on PIC
	
	/* devices initialization ends*/	

	/* Initialize devices, memory, filesystem, enable device interrupts on the
	 * PIC, any other initialization stuff... */

	/* Enable interrupts */
	/* Do not enable the following until after you have set up your
	 * IDT correctly otherwise QEMU will triple fault and simple close
	 * without showing you any output */

	 i8259_init();   /* Init the PIC */ 

     // printf("Enabling Interrupts\n");


	 paging_init();    // set up paging

	 //testing purpose of paging
	 // uint8_t * test_ptr = 0x000B8000;
	 // * test_ptr = 1;
	 // // uint8_t * test_ptr = 0xB6000;
	 // *test_ptr = 1;

	 // printf("paging works!!\n");

	 // clear();   
	 sche_init();
	 pit_init();
	 rtc_init();     // init rtc  
	 keybrd_init();
	 init_mouse();

	 fs_init(file_system_start);   // init file system, test case: test_fs_init() is commented in the fs_init function
	 //sche_init();
	 for(i = 0; i < 30; i++){
		pid_status[i] = 0;
	 }
	 pid = -1;




   // enable interrupts on processor
	 /*****test cases*****/

	 // uint8_t* filename;
	 /*test rtc open*/
	 // rtc_open(filename);

	 /*test rtc write*/
	 // int32_t fd;
	 // uint8_t* buf;
	 // rtc_write(fd, buf, 2);

	 // // /*test rtc read*/ 
	 // while(1){
	 // 	rtc_read(fd, buf, 16);
	 // 	printf("rtc_read\n");
	 // }


	  //test_dir_read();
	  // read_test_text();  // read file depends on the READ_TEXT and READ_TXT flag on the top of file_system_driver.c 
	 //read_test_exe();
	 /*****test cases*****/


	//puts("here",1);
	 enable_irq(8);  //enable rtc interrupt 
	 enable_irq(1);  //enable keyboard interrupt
	 enable_irq(0);
	 sti();

	 //while(1){
		/*
		printf("here\n");
		dentry_t tmp_dentry;
		uint32_t tmp_eip, user_esp;
		uint8_t buf[4];
		read_dentry_by_name((uint8_t*)"testprint", &tmp_dentry);
		read_data(tmp_dentry.inode_num, 0, (uint8_t*)(0x08048000), 0x400000);	
		read_data(tmp_dentry.inode_num, 24, buf, 4);
		printf("here2\n");
		tmp_eip = 0x080481a4;	
		user_esp = 0x8000000 + 0x400000 - 4; //user space stack = 132MB - 4B
		
		tss.esp0 =0x800000-4;	//kernel stack = 128MB - 4B
		tss.ss0 = KERNEL_DS;
		asm volatile ("							\n\
				 cli 						\n\
				 movw %0, %%ax				\n\
				 movw %%ax, %%ds 			\n\
    			 pushl %0 					\n\
    			 pushl %1 					\n\
				 pushfl						\n\
				 popl %%eax 				\n\
    			 orl %2, %%eax 				\n\
    			 pushl %%eax 				\n\
    			 pushl %3 					\n\
    			 pushl %4 					\n\
				 iret 						\n\
    			 "
    			 :
    			 :"i"(USER_DS), "r"(user_esp), "r"(0x200), "i"(USER_CS), "r"(tmp_eip)
    			 :"eax", "memory"
    			 );
		*/
	 //}
	execute((uint8_t *) "shell");
	while(1){
		
	}


	/* Spin (nicely, so we don't chew up cycles) */


	// uint8_t test_read_buffer[TEST_READ_BUFFER_SIZE];										//initialize buffer for testing terminal_read
	// uint8_t test_write_buffer[TEST_WRITE_BUFFER_SIZE] = "teststringforwritefunction\n";     //initialize buffer for testing terminal_write
	// while(1){
	// 	memset(test_read_buffer, 0, TEST_READ_BUFFER_SIZE);
	// 	printf("Number of bytes read: %d\n", terminal_read(0, test_read_buffer, TEST_READ_BUFFER_SIZE-1));		//testing terminal_read
	// 	terminal_write(1, test_read_buffer, TEST_READ_BUFFER_SIZE);										//testing terminal _write
	// 	terminal_write(1, test_write_buffer, TEST_WRITE_BUFFER_SIZE);			
	// }

	asm volatile(".1: hlt; jmp .1;");
}
