#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "types.h"

#define CHAR_LENGTH 32
#define NUM_DIR_ENTRIES 63
#define NUM_DATA_BLOCK 1023	
#define FOUR_KILOBYTES 4096
#define ONE_KILOBYTES 1024
#define PROG_IMAGE_ADDRESS 0x08048000

typedef struct dentry{
    
    int8_t fileName[CHAR_LENGTH];
    int32_t fileType;
    int32_t inodeNum;
    int8_t reserved[24]; 
}dentry_t;

typedef struct bootBlock{
	int32_t numDirectory;
	int32_t numInode;
	int32_t numDataBlock;
	int8_t reserved[52];
	dentry_t dir_Entries[NUM_DIR_ENTRIES];
}bootBlock_t;


typedef struct inode{
	int32_t length;
	int32_t dataBlock[NUM_DATA_BLOCK];
}inode_t;

typedef struct data_block {
	int32_t data[ONE_KILOBYTES];
}data_block_t;


bootBlock_t *getBootHead();

int32_t test_file_system();
extern int32_t init_bootBlock_address(bootBlock_t * address);
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t open_file (const uint8_t * filename);
int32_t close_file (int32_t fd);
int32_t read_file  (int32_t fd, void *buf, int32_t bytes);
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes);

int32_t open_directory (const uint8_t* filename);
int32_t close_directory (int32_t fd);
int32_t read_directory (int32_t fd,  void* buf, int32_t nbytes);
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes);

int32_t read_file_sys_helper  (const uint8_t * filename, const void *buf, int32_t bytes);

extern inode_t * get_inodeptr(int32_t inodeNum);
#endif
