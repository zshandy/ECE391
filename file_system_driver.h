#ifndef _FILE_SYSTEM_DRIVER_H
#define _FILE_SYSTEM_DRIVER_H

#include "lib.h"
#include "syscall.h"

#define FN_MAX 32
#define FD_MAX 8
#define FILE_START_INDEX 1
#define	REGULAR_FILE 2


// directory entry
typedef struct dentry{
	uint8_t file_name[FN_MAX];
	uint32_t file_type;  // 0 for a file giving user-level access to RTC, 1 for the directory, and 2 for a regular file
	uint32_t inode_num;      // index node number of the file
	uint8_t reserved[24];
} dentry_t;

// file operation structure
// typedef struct file_operations  
// {
// 	int32_t (*open_t) (const uint8_t* filename);
// 	int32_t (*close_t) (int32_t fd);
// 	int32_t (*write_t) (int32_t fd, const uint8_t* buf, int32_t nbytes);
// 	int32_t (*read_t) (int32_t fd, uint8_t* buf, int32_t nbytes);
// } file_operations;

// file structure
// typedef struct file
// {
// 	file_operations* f_op;    // file operations table pointer
// 	uint32_t inode_ptr;	      // inode pointer
// 	uint32_t file_position;   // file position
// 	uint32_t flags;   // flags 
// } file;

// index node. 4kB per block. 
typedef struct inode           
{                          
	uint32_t data_length;   //  data length
	uint32_t data_index[1023];   // data blocks
} inode_t;

//sturcture for the bootblock 
typedef struct bootblock
{
	uint32_t dir_entry_num;
	uint32_t inode_num;
	uint32_t data_block_num;
	uint8_t reserved[52];
	dentry_t dir_entry[63];
} bootblock_t;

void fs_init(uint32_t addr);

//open the file with given file name
int32_t file_open(const uint8_t* filename);

// close the file of given file descriptor
int32_t file_close(int32_t fd);

// write n-byte data from buf to file specified by fd  
int32_t file_write(int32_t fd, const uint8_t* buf, int32_t nbytes);

// read the content of file specified by fd, store length of nbytes data into buf
int32_t file_read (int32_t fd, uint8_t* buf, int32_t nbytes);    

// open the file with given file name
int32_t dir_open(const uint8_t* filename);

// close the given directory
int32_t dir_close(int32_t fd);

int32_t dir_write(int32_t fd, const uint8_t* buf, int32_t nbytes);

// read the file names of the given directory 
int32_t dir_read(int32_t fd, uint8_t* buf, int32_t nbytes);

// read directory entry by file name
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);

// read the directory entry by directory index  
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);

//read bytes length of data from the given file 
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

void test_fs_init(void);
void test_dir_read(void);
void read_test(void);
void read_test_text(void);
void read_test_exe(void);
#endif
