#include "file_system_driver.h"

#define BLOCK_SIZE 4096
#define SMALL_BUF 500
#define LARGE_BUF 6000
#define SIZE_THREAD 400
#define TEST_FD 2

#define EIGHT_MB 0x0800000
#define EIGHT_KB 0x2000

static uint32_t D;
static uint32_t N;
static uint32_t inode_start;
static uint32_t dir_size;
static uint32_t data_start;
static uint32_t fs_start;   // file system start address
bootblock_t* boot_ptr;


/*********file system initialization*********/
void fs_init(uint32_t addr)
{
	fs_start = addr;   // init file system start address
	boot_ptr = (bootblock_t*) addr;   
	dir_size = boot_ptr->dir_entry_num;   // init size of dir_entry
	N = boot_ptr->inode_num;   // init inode
	D = boot_ptr->data_block_num;   // init data block
	inode_start = fs_start + BLOCK_SIZE;  // init inode start address
	data_start = inode_start + N * BLOCK_SIZE;   // init data block start address
}

/*********file system initialization*********/





/***************dir operations***************/

// dir_read
// read the file names of the given directory 
// input: fd -- file descriptor, buf -- buffer used to store filenames, nbytes -- length of bytes to read from directory
int32_t dir_read(int32_t fd, uint8_t* buf, int32_t nbytes){   


	// printf("dir pid: %d\n", pid);
	pcb_t * new_pcb = (pcb_t *) (EIGHT_MB - EIGHT_KB*(pid+1));
	uint32_t offset = (new_pcb->file_array[fd].file_position);
	// printf("offset: %d\n position: %d\n", offset, new_pcb->file_array[fd].file_position);
	(new_pcb->file_array[fd].file_position)++;
	if ((fd <= 1) || (fd >= FD_MAX))  // return -1, if fd out of scope
		return -1;
	if (1 + offset >= dir_size)  // return 0 if end of directory
		return 0;		
	uint8_t* dentry_name = boot_ptr->dir_entry[offset].file_name;
	strncpy((int8_t*)buf,(int8_t*)dentry_name, nbytes);
	return strlen((int8_t*)(boot_ptr->dir_entry[offset].file_name));



}

int32_t read_file_in_dir(uint32_t offset, uint8_t* buf, uint32_t length)
{
	if (buf == NULL)
	return -1;
	if (offset+1 >= dir_size)
	return 0;			
	
	uint8_t* dentry_name =  boot_ptr->dir_entry[offset].file_name;
	strncpy((int8_t*)buf,(int8_t*)dentry_name, length);

	return strlen((int8_t*)(boot_ptr->dir_entry[offset].file_name));
}

 void test_dir_read(void)
{
	printf("test read the directory...\n");
	int32_t cnt, i=0;
    uint8_t buf[FN_MAX+1];

    while (0 != (cnt = read_file_in_dir(i++, buf ,FN_MAX))) {
        if (-1 == cnt) {
	        printf("directory entry read failed\n");
	        return;
	    }
	    buf[cnt] = '\n';
	    printf("%s", buf);
    }
	
	return;
}

int32_t dir_write(int32_t fd, const uint8_t* buf, int32_t nbytes){
	return -1; 
}


// dir_open:
// open the file with given file name
// input: filename -- file name to be opened
// output: 0
int32_t dir_open(const uint8_t* filename){
	return 0;
}

// dir_close:
// close the file of given directory
// input: fd -- file descriptor
// output: 0
int32_t dir_close(int32_t fd){
	return 0;
}
/***************dir operations*****************/





/***************file operations****************/


/* 
 * file_write
 * DESCRIPTION: read the content of file specified by fd, store length of nbytes data into buf       
 * INPUTS: fd -- file descriptor, buf -- buffer, nbytes -- number of bytes 
 * OUTPUTS: none
 * RETURN VALUE: returns -1 
 */
int32_t file_read (int32_t fd, uint8_t* buf, int32_t nbytes)
{

	pcb_t * new_pcb = (pcb_t *) (EIGHT_MB - EIGHT_KB*(pid+1));

	if(fd <= 1)  // if fd is out of scope, return -1
		return -1;
	if(fd >= FD_MAX)
		return -1;

	int32_t data_read = read_data(new_pcb->file_array[fd].inode, new_pcb->file_array[fd].file_position, buf, nbytes);   //get nbytes data from file

	if(data_read != -1){
		new_pcb->file_array[fd].file_position += data_read; //update file position
		return data_read;
	}

	return -1;
}

/* 
 * file_write
 * DESCRIPTION: write n-byte data from buf to file specified by fd        
 * INPUTS: fd-- file descriptor, buf --  buffer, nbytes-- number of bytes 
 * OUTPUTS: none
 * RETURN VALUE: returns -1
 */
int32_t file_write(int32_t fd, const uint8_t * buf, int32_t nbytes){
	return -1;
}

/* 
 * file_open
 * DESCRIPTION: opens the file with filename
 * INPUTS: filename: file to be opened
 * OUTPUTS: none
 * RETURN VALUE: 0
 */
int32_t file_open(const uint8_t* filename){
	return 0;
}

int32_t file_close(int32_t fd){
	return 0;

}
/***************file operations*****************/


 


/***************test cases**********************/

// read_test_text:
// Print out a text file and/or an executable. 
// Print out the size in bytes of a text file and/or an executable.
// input: none
// output: none
// effect: print out the content in the specific file, depending on the file flag 
void read_test_text(void)
{
	printf("test reading file...\n");
	clear();
	printf("test reading file...\n");

	dentry_t test_file;
	uint32_t i;
	int32_t bytes_read;

	uint32_t buffer_size = SMALL_BUF;   
	uint8_t buffer[buffer_size];
	if(-1 == read_dentry_by_name((uint8_t*)"frame1.txt", &test_file)){
		printf("failed reading file");
		return;
	}
	read_dentry_by_name((uint8_t*)"frame1.txt", &test_file);   // read the text file
	
	printf("The file type the file:%d\n", test_file.file_type);
	printf("The inode index of the file:%d\n", test_file.inode_num);
	bytes_read = read_data(test_file.inode_num, 0, buffer, buffer_size);  // size of file
	printf("size of file is : %d btyes\n", (int32_t)bytes_read);

	if (bytes_read <= 0)
	{
		printf("read data failed\n");
		return;
	}
	
	// print the content of file depending on how large it is
	if (bytes_read>=SIZE_THREAD){   
		printf("Since the file is too large,\nwe print the first and last 200 bytes in the file.\n");
		printf("\n");
		printf("First 200 bytes:\n");
		//for (i=0; i<bytes_read; i++)
		for (i=0; i<SIZE_THREAD/2; i++)
		{
			printf("%c", buffer[i]);
		}
		printf("\n\n");
		printf("Last 200 bytes:\n");
		for (i=bytes_read-SIZE_THREAD/2; i<bytes_read; i++){
			printf("%c", buffer[i]);
		}
		return;
	}

	for (i=0; i<bytes_read; i++){
		printf("%c", buffer[i]);
	}
	return;
}


// read_test_exe:
// Print out a text file and/or an executable. 
// Print out the size in bytes of a text file and/or an executable.
// input: none
// output: none
// effect: print out the content in the specific file, depending on the file flag 
void read_test_exe(void)
{
	printf("test reading file...\n");
	clear();
	printf("test reading file...\n");

	dentry_t test_file;
	uint32_t i;
	int32_t bytes_read;


	uint32_t buffer_size = LARGE_BUF;    
	uint8_t buffer[buffer_size];
	if(-1 == read_dentry_by_name((uint8_t*)"pingpong", &test_file)){
		printf("failed reading file");
		return;
	}
	read_dentry_by_name((uint8_t*)"pingpong", &test_file);   // read the execute file
	
	printf("The file type the file:%d\n", test_file.file_type);
	printf("The inode index of the file:%d\n", test_file.inode_num);
	bytes_read = read_data(test_file.inode_num, 0, buffer, buffer_size);  // size of file
	printf("size of file is : %d btyes\n", (int32_t)bytes_read);

	if (bytes_read <= 0)
	{
		printf("read data failed\n");
		return;
	}
	
	// print the content of file depending on how large it is
	if (bytes_read>=SIZE_THREAD){   
		printf("Since the file is too large,\nwe print the first and last 200 bytes in the file.\n");
		printf("\n");
		printf("First 200 bytes:\n");
		//for (i=0; i<bytes_read; i++)
		for (i=0; i<SIZE_THREAD/2; i++)
		{
			printf("%c", buffer[i]);
		}
		printf("\n\n");
		printf("Last 200 bytes:\n");
		for (i=bytes_read-SIZE_THREAD/2; i<bytes_read; i++){
			printf("%c", buffer[i]);
		}
		return;
	}

	for (i=0; i<bytes_read; i++){
		printf("%c", buffer[i]);
	}
	return;
}

/***************test cases************************/





/***************helper functions********************/

// read_dentry_by_name:
// DESCRIPTION: read directory entry by file name
// input: fname -- file name, dentry -- directory entry
// output: return -1 if fails, 0 if succeeds
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
	int i;
	for(i=0; i<dir_size; i++){   // search through entire directory
		uint8_t* dentry_name = boot_ptr->dir_entry[i].file_name;
		uint8_t check_name = strncmp((int8_t*)fname, (int8_t*)dentry_name, FN_MAX-1);  // check_name equals 0 if the strings are equal
		if(check_name==0){
			strncpy((int8_t*)(dentry->file_name), (int8_t*)(fname), FN_MAX);   // copy string of file name into dentry
			dentry->file_type = boot_ptr->dir_entry[i].file_type;				// update file type of the parameter dentry
			dentry->inode_num = boot_ptr->dir_entry[i].inode_num;			// update inode index of the parameter dentry
			return 0;
		}
	}
	return -1;   // return -1 on failure
}




 //  read_dentry_by_index:
 //  DESCRIPTION: read the directory entry by directory index   
 //  INPUTS: index --  directory index, dentry -- pointer to the reading dentry 
 // output: return -1 if fails, 0 if succeeds

int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
	
	if(index >= dir_size)   // return -1 if invalid index
		return -1;
	if(boot_ptr->dir_entry[index].file_type != REGULAR_FILE)   // return -1 if not regular file type
		return -1;

	strncpy((int8_t*)(dentry->file_name), (int8_t*)(boot_ptr->dir_entry[index].file_name), FN_MAX);   // copy string of file name into dentry

	dentry->file_type = boot_ptr->dir_entry[index].file_type;   // update file type of the parameter dentry
	dentry->inode_num = boot_ptr->dir_entry[index].inode_num;   // update inode index of the parameter dentry
	return 0;  
}


 // read_data:
 // DESCRIPTION:  read bytes length of data from the given file 
 // INPUTS: inode -- the inode index, offset --  start postion of the file ,
 		  // buf -- store the data read, length -- size of data to read
 //  OUTPUTS: none
 //  RETURN VALUE: -1 if fails, else return bytes read
 //  SIDE EFFECTS: The file pointer is then updated
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	uint32_t i, block_index, read_temp;
  	uint32_t byte_read = 0;
 	uint8_t* read_start;
 	uint32_t start_byte = offset;
 	uint32_t end_bytes = offset + length; 	
 	uint32_t start_pos = start_byte/BLOCK_SIZE;
 	uint32_t end_pos = end_bytes/BLOCK_SIZE;
	inode_t* inode_reading = (inode_t*)(inode_start + inode*BLOCK_SIZE); 
	uint32_t read_length = inode_reading->data_length;

	if (offset >= read_length)   // invalid start point 
 		return 0;	
	if (inode >= N)  // invalid inode index
		return -1;

 	if ((offset + length) >= read_length) // if length too long, clip it
 		length = read_length - offset;   // number of bytes to be read

	for (i= start_pos ; i<= end_pos ; i++)
	{
		read_temp = BLOCK_SIZE - (offset % BLOCK_SIZE); // reduce number of bytes to read
		block_index = inode_reading->data_index[i];
		
		if (block_index >= D) // check if index out of scope
			return -1;

		if (((offset % BLOCK_SIZE) + length) < BLOCK_SIZE) // check block size 
			read_temp = length;

		read_start = (uint8_t*)(data_start+block_index*BLOCK_SIZE + (offset % BLOCK_SIZE));
		memcpy(buf, read_start, read_temp); // copy the data

		offset = offset + read_temp;   // update offset	
		buf = buf + read_temp;   // update buffer
		length = length - read_temp;   // update length
		byte_read = byte_read + read_temp;   // update copied bytes
	} 

	return byte_read;
}


/*************helper functions******************/
