#ifndef _PCB_H
#define _PCB_H

#include "file_system.h"

#define NUM_FILE_DESCRIPTOR 8
#define ARGUMENT_LEN 128
typedef struct sys_ops {
	int32_t (*syscall_open)(const uint8_t * filename);
	int32_t (*syscall_close)(int32_t fd);
	int32_t (*syscall_read)(int32_t fd, void * buf, int32_t nbytes);
	int32_t (*syscall_write)(int32_t fd, const void * buf, int32_t nbytes);
	uint32_t value ;
} sys_ops_t;
typedef struct file_descriptor {

	// file operations table pointer
	int8_t fileName[32];
	uint32_t * inode_ptr;
	uint32_t filePosition; // offset of the file or pointer ??
	uint32_t flags;
	sys_ops_t * sysops_ptr;

} file_descriptor_t ;

typedef struct __attribute__((packed)) pcb_t pcb_t;

struct pcb_t {

	file_descriptor_t fd[NUM_FILE_DESCRIPTOR]; // first two index contain stdin,stdout
	dentry_t dentry[NUM_FILE_DESCRIPTOR];
	uint32_t * prev_pcb;
	uint32_t esp;
	uint32_t ebp;
	uint32_t eip;
	uint32_t cr3;
	uint32_t parent_pid;
	// add a dentry
	uint8_t argument[ARGUMENT_LEN];
	// add more!
};

#endif
