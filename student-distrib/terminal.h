#ifndef _TERMINAL_H
#define _TERMINAL_H
#include "i8259.h"
#include "types.h"
#include "lib.h"
#include "systemcall.h"
#include "x86_desc.h"
#include "idt.h"
#include "paging.h"

#define shift_left_pressed 0x2A
#define shift_right_pressed 0x36
#define shift_left_released 0xAA
#define shift_right_released 0xB6
#define ctrl_left_pressed 0x1D
#define ctrl_right_pressed 0x1D
#define ctrl_left_released 0x9D
#define ctrl_right_released 0x9D
#define capslock_pressed 0x3A
#define capslock_released 0xBA
#define letter_l 0x26
#define alt_pressed 0x38
#define F1_released 0xBB
#define F2_released 0xBC
#define F3_released 0xBD
#define KEY_RELEASED  0x80
#define alt_released 0xB8
#define NUM_CHARS 128
#define CHARS_PER_LINE 80

#define MAXTERMS 3

#define term0_buffer 0x600000
#define term1_buffer 0x601000
#define term2_buffer 0x602000
#define MEM_4KB 0x1000  


void save_terminal(int term);
void terminal_init();
void terminal_key_pressed( unsigned char scancode);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
void backspace_pressed();
void pressed_clear();
int32_t keyboard_read();
int find_terminal(int program_id);
char * get_term_buffer(int term);

typedef struct terminal
{
  int x,y;
  unsigned char line_buffer[NUM_CHARS];
  unsigned int buffer_i ;
  uint32_t esp;
  uint32_t ebp;
  uint32_t cr3;
  uint32_t kernel_esp;
  uint32_t program_id;
  uint32_t active;
  /* data */
} terminal_t;
terminal_t terminals[3];
volatile int old_terminal;
int newterminal_flag;

#endif
