#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "pcb.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "file_system.h"
#include "rtc.h"
#define MB_8 0x800000
#define KB_8 0x2000
#define MB_132 0x08400000
#define PROGRAM_MEM 0x08048000
#define SDIN 0
#define SDOUT 1
#define PREV 4
#define BUF_SIZE 40
#define USER_START 0x08000000
#define USER_END 0x08400000
#define video_mem 0xB8000

int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);
void rtc_interrupt();

int32_t get_sys_read(int32_t fd, void * buf, int32_t nbytes);

int32_t get_sys_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t open_test();
int32_t close_test();
int32_t read_test();
int32_t write_test();
int find_programid();
void set_programpage(int program_id);
int page_loader(const uint8_t * file, int program_id);

#endif
