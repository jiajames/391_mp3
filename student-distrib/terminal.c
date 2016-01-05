#include "terminal.h"
#include "scheduler.h"

extern int screen_x;
extern int screen_y;

volatile int shift = 0;
volatile int ctrl = 0;
volatile int capslock = 0;
volatile unsigned char line_buf[NUM_CHARS];
volatile int alt = 0;
volatile int buffer_i = 0;
volatile int back_ok = 0;
volatile int check_enter = 0;
volatile int buffer_index = 0;

int irp = 0;
// ps2 keyboard mapping lovingly ripped from Bran's kernel tutorial
/*
    find the terminal that match the program id;
*/


/*  int find_terminal(int program_id)
  function: find the terminal that match the program id;
  input: int program_id, the program to find by id
  returns: (int) the terminal corresponding to program_id if one matched
        -1 otherwise
*/

int find_terminal(int program_id)
{
  int i;

  for(i =0 ; i < 3 ; i++)
  {
    if( program_id == terminals[i].program_id)
        return i;
  }

  // on failure, return -1;
  return -1;

}

/*  int save_terminal(int term)
  function: save the registers of the current program into the selected
        terminal of the array of terminals
  input: int terminal, the terminal to save to
  returns: nothing!
*/

void save_terminal(int term){
  asm volatile ("movl %%CR3, %0"
                   : "=b"(terminals[term].cr3));

  terminals[term].esp = esp_0;
  terminals[term].ebp = ebp_0;

  terminals[term].kernel_esp = tss.esp0;

  terminals[term].program_id = find_programid();

  terminals[term].x = get_cursor_x();
  terminals[term].y = get_cursor_y();

  if(term == 0)
    memcpy((char*)term0_buffer, (char*)VIDEO, MEM_4KB);
  else if(term == 1)
    memcpy((char*)term1_buffer, (char*)VIDEO, MEM_4KB);
  else
    memcpy((char*)term2_buffer, (char*)VIDEO, MEM_4KB);

  non_video_mapping(terminals[term].program_id, term);
}

unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

unsigned char kbdus_shift[128] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
  '(', ')', '_', '+', 0, /* Backspace */
  '\t',     /* Tab */
  'Q', 'W', 'E', 'R', /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, /* Enter key */
    0,      /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
 '\"', '~',   0,    /* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',      /* 49 */
  'M', '<', '>', '?',   0,        /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

/*  void terminal_switch(int term)
  function: switch to the selected terminal if different from current
        saves the current terminal's registers and video memory into its struct
  input: int term, the terminal to eventually switch to
  returns: nothing!
*/

void terminal_switch(int term){

  if(term < 0 || term > 2 || old_terminal == term)
    return;

  save_terminal(old_terminal);

  old_terminal = term;

     clear();

  terminal_t term_new = terminals[term];

  if (terminals[term].active == 0)
  {
    terminals[term].active = 1;
    newterminal_flag = 1;
    add_task_to_queue(&terminals[term]);
    send_eoi(1);
    execute((const uint8_t*)"shell");
    return;
  }

  video_mapping(term_new.program_id);


  set_cursor(term_new.x, term_new.y);

  send_eoi(1);

  // update esp, ebp, cr3, tss.esp0, tss.ss0
  asm volatile("movl %0, %%cr3" : : "r" (term_new.cr3));

  if(term == 0)
    memcpy((char*)VIDEO, (char*)term0_buffer, MEM_4KB);
  else if(term == 1)
    memcpy((char*)VIDEO, (char*)term1_buffer, MEM_4KB);
  else
    memcpy((char*)VIDEO, (char*)term2_buffer, MEM_4KB);  

   tss.esp0 = MB_8 - KB_8*(terminals[term].program_id - 1) - PREV;//entry_point;

  tss.ss0 = KERNEL_DS;

   int_ret_helper(term_new.ebp, term_new.esp);
}


/*  void terminal_init()
  function: initialize the array of terminal structs for use
  input: nothing!
  returns: nothing!
*/

void terminal_init()
{
  int i;
  old_terminal = 0;
  char * buf0; char* buf1; char* buf2;

  buf0 = get_term_buffer(0);
  buf1 = get_term_buffer(1);
  buf2 = get_term_buffer(2);

  for(i = 0; i < (NUM_ROWS*NUM_COLS); i++)
  {
    buf0[i] = ' ';
    buf1[i] = ' ';
    buf2[i] = ' ';
  }

  for (i = 0; i < 3; i++)
  {

    terminals[i].x = 0;
    terminals[i].y = 0;
    terminals[i].esp = NULL;
    terminals[i].ebp = NULL;
    terminals[i].cr3 = NULL;
    terminals[i].kernel_esp = NULL;
    terminals[i].program_id = 0;
    terminals[i].active = 0;
    terminals[i].buffer_i = 0;
  }
    terminals[0].active = 1;  
}

/*  void terminal_key_pressed(unsigned char scancode)
  function: respond to the key pressed on the keyboard
        upon special key combinations do special actions
            (alt+F1/2/3), switch terminal
            (backspace), erase a character in the line buffer
            (ctrl+L), clear the terminal
        on other keys, place the key into the line buffer if there is still room,
        and echo the key to the screen. Handles line breaks upon hitting the end
        of the horizontal line on terminal, and triggers scrolling up upon
        hitting the bottom of terminal
        Pressing caps lock shifts following a-z keypreses to capital
        Pressing shift does the same for a-z,
            and also shifts `-= to alternate symbols (~-+)
  input: unsigned char scancode, the scancode for the key pressed on the ps2 keyboard
  returns: nothing!
*/

void terminal_key_pressed( unsigned char scancode){
    //if(scancode >= NUM_CHARS)   // bound check (only 128 valid scancodes)
    //   return;
    // check for modifier key state changes
      terminals[old_terminal].active = 1;
unsigned char toPrint;
  toPrint = kbdus[scancode];
   if((terminals[old_terminal].buffer_i == NUM_CHARS) && (toPrint == '\n'))
    {
      terminals[old_terminal].line_buffer[terminals[old_terminal].buffer_i-1] = toPrint;
      terminals[old_terminal].buffer_i--;
      }
              
   if(scancode == alt_pressed ){
       alt = 1;
   }
   if(scancode == alt_released){
     alt = 0;
   }

	if (scancode == shift_left_pressed || scancode == shift_right_pressed){
		shift = 1;
    }
	else if (scancode == shift_left_released || scancode == shift_right_released){
		shift = 0;
    }
	else if (scancode == ctrl_left_pressed || scancode == ctrl_right_pressed)
		ctrl = 1;
	else if (scancode == ctrl_left_released || scancode == ctrl_right_released)
		ctrl = 0;
	else if (scancode == capslock_released)
		capslock = !capslock;
	else{  // if no modifier keys pressed/released

     if(alt){
      if(scancode == F1_released){
          terminal_switch(0);
          return;
        //set to first terminal
      }
      if(scancode == F2_released){
//        printf("terminal 2\n");
          terminal_switch(1);
          return;
        //set to second terminal
      }
      if( scancode == F3_released){
//        printf("terminal 3\n");
          terminal_switch(2);
          return;
        //set to third terminal
      }
     }


		if( ctrl ){   // check for ctrl-L to clear screen
			if (scancode == letter_l)
			{
				pressed_clear();
			}
		}
		else if(scancode != capslock_pressed){    // adjust letters to uppercase on caps/shift
			if (toPrint >= 'a' && toPrint <= 'z'){
                if(capslock && shift)
                  toPrint = kbdus[scancode];
                else if(capslock || shift)
                  toPrint = kbdus_shift[scancode];
        }
        else if(toPrint == '\b' && scancode == 0x0E){ // on backspace, dont print and also modify video memory
            toPrint = 0;
            backspace_pressed();
        }
        else{ // on 0-9, or any other special characters, modify numbers to symbols on shift pressed
            if(shift){
                toPrint = kbdus_shift[scancode];
            }
        }

		if(!(scancode & KEY_RELEASED) && toPrint != 0){   // else if we have pressed (no print on release of key), and the scancode is a printable one
            check_enter = 0;
            if(toPrint == '\b'){    // check for backspace again
                toPrint = 0;
                backspace_pressed();
            }

            if(terminals[old_terminal].buffer_i < NUM_CHARS){
                if(toPrint != '\n')     // on non-newline, print
                  printf("%c", toPrint);
              else{                   // otherwise go to next line or move all of video memory up if on bottom already
                  if((get_cursor_y()+1) < NUM_ROWS)
                      set_cursor(0, get_cursor_y());
                  else{
                      check_enter = 1;
                      pressed_enter();
                  }
                  set_cursor(0, get_cursor_y());
              }
            //if(toPrint != '\n' && toPrint != '\r'){   // on non-newline, adjust buffer if not at 128 (full buffer)
     //         if(terminals[old_terminal].buffer_i < NUM_CHARS){
                terminals[old_terminal].line_buffer[terminals[old_terminal].buffer_i] = toPrint;
                terminals[old_terminal].buffer_i++;
       //       }
             
            }

           if(get_cursor_x() == 0 || get_cursor_x() == NUM_COLS)
            {

                if((get_cursor_y()+1) < NUM_ROWS)   //go to next line or move all of video memory up if on bottom already
                  set_cursor(0, get_cursor_y()+1);
                else
                  if(!check_enter)
                    pressed_enter();
            }

            if(toPrint == '\n' || toPrint == '\r'){ // test line buffer read from keyboard

            }
	    }
    }
	}

}

/*  int32_t terminal_open(const uint8_t* filename)
  function: open the terminal for use
  input: (unused)
  returns: 0 always
*/

int32_t terminal_open(const uint8_t* filename){
	return 0;  // add the close/write functions to syscall jump table eventually
}


/*  int32_t terminal_close(int32_t fd)
  function: close the terminal
  input: (unused)
  returns: 0 always
*/
int32_t terminal_close(int32_t fd ){
	return 0;  // same TODO as terminal_open
}

/*  int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
  function: read from the terminal by waiting for keyboard input to fill up
        the line buffer until the user presses enter (return)
  input: (int32_t fd (unused),
        void* buf, the buffer to write into,
        int32_t nbytes (unused)
  returns: the number of bytes read from the keyboard
*/

int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){

  int term = find_terminal(find_programid());
  terminals[term].buffer_i = 0;

  while(terminals[term].buffer_i == 0 || terminals[term].line_buffer[terminals[term].buffer_i-1] != '\n');

  return keyboard_read(buf);  // same TODO as terminal_open
}

/*
write to terminal
takes in a null-terminated string to write to the terminal
has a side-effect of resetting the buffer so that we can't backspace and erase the message this function prints
*/

/*  int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes)
    function: write to terminal (from a user program or other) Handle normal
        lines reaching past the 80-width of the terminal and reaching bottom of
        terminal
    input: takes in a null-terminated string to write to the terminal
    returns: 0
    Note: has a side-effect of resetting the buffer so that we can't backspace
        and erase the message this function prints
*/

int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
   uint8_t* str = (uint8_t*) buf;
   uint8_t toPrint;
   int i = 0;
   int term = find_terminal(find_programid());
      toPrint = *str;
    while(*str != '\0' && i < nbytes){   // assume string is null-terminated

        // wrap around if we hit line end
        if(get_cursor_x() == 0)
          {
              if(get_cursor_y() < NUM_ROWS)
                set_cursor(0, get_cursor_y());
              else{
                pressed_enter();
                set_cursor(0, NUM_ROWS-1);
              }
          }

        if((strlen((char*)buf) > NUM_COLS) && (get_cursor_x()+1 == NUM_COLS))
        {
          if(get_cursor_y()+1 != NUM_ROWS)
                set_cursor(0, get_cursor_y()+1);
              else{
                pressed_enter();
                set_cursor(0, NUM_ROWS-1);
              }
        }          
        else if((get_cursor_x()+1) == NUM_COLS)
        {
          if(get_cursor_y() < NUM_ROWS)
                set_cursor(0, get_cursor_y()+1);
              else{
                pressed_enter();
                set_cursor(0, NUM_ROWS-1);
              }
        }
          i++;
          printf("%c", toPrint);

          str++;    // move onto next character
          toPrint = *str;
      }
      terminals[term].buffer_i = 0; // this disables backspace erasing the syscall-written message
      return 0;
      // was originally this -> return 0;
}

/*  void pressed_clear()
  function: clears the terminal, triggered by ctrl-L combination on keyboard
  input: none
  returns: nothing!
*/

void pressed_clear(){
	//clear the screen in video memory
	clear();
	set_cursor(0,0);   // set cursor back to 0,0
    terminals[old_terminal].buffer_i = 0;
}

/*  void pressed_clear()
  function: deletes most recent char in line buffer if it exists,
        triggered by backspace on keyboard
  input: none
  returns: nothing!
*/

void backspace_pressed(){
  if(terminals[old_terminal].buffer_i > 0)
  {
     terminals[old_terminal].buffer_i--;
     move_back();
  }
}

/*  int32_t keyboard_read(void* buf)
  function: read from the line_buffer up until buffer_i index
        (after press of return key from user)
    Handle normal case of lines reaching past the 80-width of the terminal and
        reaching bottom of terminal or if it happens because user presses enter
  input: void* buf, cast as (uint8_t*) to copy line buffer into it
  returns: number of bytes read from line buffer
*/

// read from the line_buffer up until buffer_i index
int32_t keyboard_read(void* buf){
    int i = 0;
    //for(i = 0 ; i < buffer_i; i++){
    while(i < terminals[old_terminal].buffer_i){  // print all chars except newline at end

        ((unsigned char*)buf)[i] = terminals[old_terminal].line_buffer[i];


        i++;
    } 
    ((unsigned char *)buf)[i] = '\0';

    update_cursor(get_cursor_y(), get_cursor_x());

    terminals[old_terminal].buffer_i = 0; // upon reading from the keyboard, reset the buffer_i index
    return i;
}

/*  char* get_term_buffer(int term)
  function: get the terminal line buffer of the terminal selected
  input: int term, the term we select
  returns: char* pointing to the line buffer of the terminal
*/

char * get_term_buffer(int term)
{
  switch(term)
  {
    case 0:
      return (char*)term0_buffer;
    case 1:
      return (char*)term1_buffer;
    case 2:
      return (char*)term2_buffer;
    default:
      printf("get_term_buffer error\n");
      while(1);
      return NULL;
  }

  printf("get_term_buffer error\n");
  while(1);
  return NULL;  
}
