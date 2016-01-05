#include "systemcall.h"
#include "rtc.h"
#include "x86_desc.h"
#include "pcb.h"
#include "file_system.h"
#include "paging.h"
#include "terminal.h"
#include "lib.h"
#include "scheduler.h"

int32_t openTasks[7] = {-1,-1,-1, -1, -1 , -1, -1};
sys_ops_t rtc_ops = {&rtc_open, &rtc_close, &rtc_read, &rtc_write};
sys_ops_t terminal_ops = {&terminal_open, &terminal_close, &terminal_read, &terminal_write};
sys_ops_t directory_ops = {&open_directory, &close_directory, &read_directory, &write_directory};
sys_ops_t file_ops = {&open_file, &close_file, &read_file, &write_file};
uint32_t curr_status = -1;
int32_t first_shell_entry_point;

/*
 * int32_t execute(const uint8_t* command)
 * DESCRIPTION: loads and executes a new program
 * INPUT: const uint8_t* command - string that contains the program name and its arguments
 * OUTPUT: curr_status - 0 to 255 if if halt is executed, 256 if dies to exception, -1 if command not executed
 */
int32_t execute(const uint8_t* command)
{

		uint8_t buf[BUF_SIZE];
		uint32_t entry_point, file_name_length, argument_length;
		int i, cr3_flush, terminal_number;
		uint8_t file_name[32];
		int8_t saved_argument[128];

/*********************************************************************/
			// PARSING COMMAND ,EXCLUDING ARGUMENT FOR NOW

		if(command == NULL) // check for NULL command
		{
			printf("command is null\n");
			while(1);
			return -1;
		}

		while(*command== ' ') // delete space in the beginning
			command++;

		file_name_length =0;
		while(*command!= '\n' && *command!= ' ' && *command!= '\0') // parsing the command without argument
		{
			file_name[file_name_length] = *command;
			command++;
			file_name_length++;

			if(file_name_length > 32)
				{
					printf("invalid command\n");
					return -1;
				}
		}
		file_name[file_name_length] = '\0';


/*******************************************************************************************************/

		while(*command== ' ' && *command!= '\0') // delete space in the beginning
			command++;

		argument_length =0;
		while(*command!= '\n' && *command!= ' ' && *command!= '\0') // parsing the command without argument
		{
			saved_argument[argument_length] = *command;
			command++;
			argument_length++;

			if(argument_length > 128) // not sure if this is 128 ********IMPORTANT***********
				{
					printf("invalid command\n");
					while(1);
				}
		}
		saved_argument[argument_length] = '\0';

/********************************************************************/
						// CHECK EXE, GET ENTRY POINT


		if(open_file(file_name) == -1) // open file name
			return -1;

		read_file_sys_helper(file_name, buf, BUF_SIZE); // read to get the header information

		if ( buf[0]!= 0x7f|| buf[1]!= 0x45 || buf[2]!=0x4c ||buf[3]!=0x46) //check for executable file "magic number" given in documentation
			return -1;

		// entry point
		entry_point = (buf[27]<<24) + (buf[26]<<16) + (buf[25]<<8) + (buf[24]); //we get bit 24-27 to get the entry point specify in the documentatoin


		// ^^^^ this entry point stuff should go into the loader later
/*************************************************************************/
							/*SET UP PAGING*/



		// make up an array of avaliable memory map and check
		//  (ie)   1 0 0 0 1
		// 			12-24 megabyte are free
		//
		// shell 1 "shell" - 8-12 megabyte
		// shell 2 "ls" - 12-16 megabyte
		// *******remember that physical = linear for this MP

		// openTasks has 3 locations right now
		// if cur_programid is 0, that means we are executing
		// "shell" from the kernel, and there is no current program
		// aka cur_programid is 0.
		// It takes our first openTask array location
		// and each task takes the next location
		int cur_programid = find_programid(); // find current program id

		if(cur_programid == 0)
			first_shell_entry_point = entry_point;


		int new_programid;

		for (i = 1; i < 8; i++)
		{
			if(openTasks[i] == -1)
			{
				new_programid = i;
				if(newterminal_flag == 1)
				{
					openTasks[new_programid] = 0;
					newterminal_flag = 0;
				}
				else
				{
					openTasks[new_programid] = cur_programid; // refer back to prev progeram id
				}
				break;
			}

			if (i == 7)
			{
				printf("open task error. pausing now.\n");
				while(1);
			}
		}

/*****************************************************************************************************************/
				 /* SCHEDULING */

		 if( ( terminal_number = find_terminal(cur_programid))!= -1)
	 	{
	 		if (openTasks[new_programid] != 0)
		 		remove_task_from_queue(&terminals[terminal_number]);
		 	terminal_number = find_terminal(new_programid);
		 	add_task_to_queue(&terminals[terminal_number]);
	 	}



/****************************************************************************************************************/
					// set up the pcb for the next process
		terminals[old_terminal].program_id = new_programid;

		pcb_t * newPCB = (pcb_t *)(MB_8 - KB_8*new_programid); // pcb structure for the program
		// enable the terminal, this is rtc + terminal stuff don't worry about it
		newPCB->fd[SDIN].flags = 1;
		newPCB->fd[SDIN].sysops_ptr = &terminal_ops;
		newPCB->fd[SDOUT].flags = 1;
		newPCB->fd[SDOUT].sysops_ptr = &terminal_ops; // SDOUT == 1, SDIN == 0

		strcpy((int8_t*)newPCB->argument,saved_argument);

		for (i = 2; i < 7; i++)
			newPCB->fd[i].flags = 0;

		// save old pcb
		pcb_t * currPCB = (pcb_t *)(MB_8 - KB_8*cur_programid);

		// if cur_programid is 0, first task being executed from
		// kernel. That means we don't have a current program,
		// so we don't need to save anything into a currPCB
		// because there is no pcb
		if (cur_programid != 0)
		{
			// save cr3, esp, and ebp
			asm volatile ("movl %%CR3, %0\n\
						  movl %%esp, %1"
						   : "=b"(currPCB->cr3), "=r"(currPCB->esp));
			asm volatile("movl %%ebp, %0"
				: "=a"(currPCB->ebp)
				:: "cc" );
		}

		//printf("set currPCB cr3 esp ebp\n");
		//File Loader
		// call function from the file which load the file into 0x
		// load program image into 0x08048000
		// add here
		//eip should be passed from the loader
		// entrypoit will be moved here
/********************************************************************/
				//load the page

		int pageval = page_loader(file_name, new_programid);
		if (pageval != 0)
		{
			printf("pageval error. pageval=%d", pageval);
			while(1);
		}

/******************************************************************************************************/


/*****************************************************************************************************/
		// flush the tlb
	asm volatile("movl %%cr3, %0" : "=r" (cr3_flush));
	asm volatile("movl %0, %%cr3" : : "r" (cr3_flush));

		// loader will load the correct directory into cr3
		asm volatile ("movl %%CR3, %0": "=b"(newPCB->cr3));
		// linked list PCB, parentPCB linking
		newPCB->prev_pcb = (uint32_t *)currPCB;
		newPCB->parent_pid = cur_programid;

		//context switch
		// write tss.esp0/ebp0 with new process in kernal stack
		//save current esp/ebp or anything you need in PCB
		// push artificial IRET context onto stack

/************************************************************************************************************/
		/*SET UP TSS*/
		 tss.esp0 = MB_8 - KB_8*(new_programid - 1) - PREV;//entry_point;
		 tss.ss0 = KERNEL_DS;

/****************************************************************************************************************/
		//push artifical iret

		// look up the argument of iret, theres 5 of them
		// TO PUSH ON TO KERNAL STACK
		// disable interrupts
	    // Ring3_SS
		// Ring3_ESP
		//	EFLAGS
		// enable interrupts
		//  Ring3_CS
		// Ring3_EIP
		 //http://stackoverflow.com/questions/6892421/switching-to-user-mode-using-iret
		 //we orl $0x200 to enable to interrupt flag bit

        asm ("cli\n\
        	movw %0, %%ax\n\
            movw %%ax, %%ds\n\
            movw %%ax, %%es\n\
            movw %%ax, %%fs\n\
            movw %%ax, %%gs\n\
            pushl %0\n\
            pushl %1\n\
            pushfl\n\
            popl %%eax\n\
            orl $0x200, %%eax\n\
            pushl %%eax\n\
            pushl %2\n\
            pushl %3\n\
            iret\n\
        halt_ret_label:"
             :        /* output */
             :"g"(USER_DS|0x3), "g"(MB_132 - PREV), "g"(USER_CS|0x3), "g"(entry_point)      /* input */
             :"eax" , "memory"         /* clobbered register */
             );
		 //iret
        return curr_status;
}

/*
 * halt (uint8_t status)
 * DESCRIPTION: terminates a process
 * INPUT: uint8_t status - a specified return value from the program ran
 * OUTPUT: none - should always return to parent program's execute system call
 */
int32_t halt (uint8_t status)
{
	int cur_programid = find_programid();
	int parent_programid = openTasks[cur_programid];
	curr_status = (uint32_t) status;
	int i, terminal_number;

	/*****************************************************************************************************************/
				 /* SCHEDULING */

	if( ( terminal_number = find_terminal(cur_programid))!= -1)
	{
		if (openTasks[cur_programid] != 0)
	 		remove_task_from_queue(&terminals[terminal_number]);
	 	terminal_number = find_terminal(parent_programid);
	 	add_task_to_queue(&terminals[terminal_number]);
	}



/****************************************************************************************************************/
	if(parent_programid == 0)
	{
		        asm ("cli\n\
        	movw %0, %%ax\n\
            movw %%ax, %%ds\n\
            movw %%ax, %%es\n\
            movw %%ax, %%fs\n\
            movw %%ax, %%gs\n\
            pushl %0\n\
            pushl %1\n\
            pushfl\n\
            popl %%eax\n\
            orl $0x200, %%eax\n\
            pushl %%eax\n\
            pushl %2\n\
            pushl %3\n\
            iret \n "
             :        /* output */
             :"g"(USER_DS|0x3), "g"(MB_132 - PREV), "g"(USER_CS|0x3), "g"(first_shell_entry_point)      /* input */
             :"eax" , "memory"         /* clobbered register */
             );
	}

	pcb_t * current_pcb;
	pcb_t * parent_pcb;

	current_pcb = (pcb_t*)(MB_8 - KB_8*cur_programid);

	parent_pcb = (pcb_t*)(MB_8 - KB_8*parent_programid);

	terminals[old_terminal].program_id = parent_programid;

	// clear the paging
	set_programpage(parent_programid);

	//how to remove the current index from from openTasks
	openTasks[cur_programid] = -1;

	for (i = 2; i < 8; i++)
		close(i);
	// fix esp, ebp
		tss.esp0 = MB_8 - KB_8*(parent_programid - 1) - PREV;
		asm volatile("movl %0, %%esp" :: "g"(parent_pcb->esp));
		asm volatile("movl %0, %%ebp" :: "g"(parent_pcb->ebp));


	asm volatile("jmp halt_ret_label");
// never reached here
	return status;
}

/*
 * int32_t read (int32_t fd, void* buf, int32_t nbytes)
 * DESCRIPTION: reads data from keyboard, file, rtc, or directory. calls jump table corresponding to correct read function
 * INPUT: int32_t fd - file descriptor
 		  void* buf - buffer to read data into
 		  int32_t nbytes - number of bytes to read
 * OUTPUT: 0 on succes, -1 otherwise
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes)
{
	pcb_t * currPCB = (pcb_t*)(MB_8 - KB_8*find_programid());
	if(fd < 0 || fd > 7 || fd == 1 || currPCB->fd[fd].flags == 0)
		return -1;

	return currPCB->fd[fd].sysops_ptr->syscall_read(fd, (uint8_t*)buf, nbytes);

}

/*
 * int32_t write (int32_t fd, void* buf, int32_t nbytes)
 * DESCRIPTION: writes data into rtc or terminal. calls jump table corresponding to correct function
 * INPUT: int32_t fd - file descriptor
 		  void* buf - buffer to read data into
 		  int32_t nbytes - number of bytes to read
 * OUTPUT: number of bytes read on success, -1 otherwise
 */
int32_t write(int32_t fd,const void* buf, int32_t nbytes){
	pcb_t * currPCB = (pcb_t*)(MB_8 - KB_8*find_programid());
	if(fd < 0 || fd > 7 || fd == 0 || currPCB->fd[fd].flags == 0)
		return -1;
	if(currPCB->fd[fd].sysops_ptr->syscall_write(fd, (uint8_t*)buf, nbytes) == 0)
		return (int32_t)strlen(buf);
	else
		return -1;
}

/*
 * int32_t open (const uint8_t* filename)
 * DESCRIPTION: provides access to the filesystem
 * INPUT: const uint8_t* filename - file to open
 * OUTPUT: file descriptor number on success, -1 otherwise
 */
int32_t open (const uint8_t* filename){

	if (filename[0] == '\0')
	{
		return -1;
	}
	int cur_programid = find_programid();
	pcb_t * currPCB = (pcb_t *)(MB_8 - KB_8*cur_programid);
	dentry_t tempDentry;
	if( read_dentry_by_name(filename,&tempDentry) == 0)
	{
		int i = 2;
		int fd_used = -1;
		for(i = 2; i < NUM_FILE_DESCRIPTOR; i++){
			if(currPCB->fd[i].flags == 0){
				currPCB->fd[i].flags = 1;
				fd_used = i;
				break;
			}
		}
		if(fd_used == -1)
			{	// if we didn't find a free fd
			return -1;}

		currPCB->fd[fd_used].filePosition = 0;

		if(tempDentry.fileType == 0) { 	//File type is RTC
				currPCB->fd[fd_used].sysops_ptr = &rtc_ops;

				currPCB->fd[fd_used].sysops_ptr->value = 1;
				currPCB->fd[fd_used].inode_ptr = NULL;
		}
		else if(tempDentry.fileType == 1){ //File type is directory
				currPCB->fd[fd_used].sysops_ptr = &directory_ops;

				currPCB->fd[fd_used].sysops_ptr->value = 2;

		}
		else if(tempDentry.fileType == 2){	//file type is regular file

				currPCB->fd[fd_used].sysops_ptr = &file_ops;
				//inode_t *tempInode = ;
				currPCB->fd[fd_used].filePosition = 0;

				currPCB->fd[fd_used].inode_ptr = (uint32_t*) get_inodeptr(tempDentry.inodeNum);

				strcpy(currPCB->fd[fd_used].fileName, (int8_t*) filename);

				currPCB->fd[fd_used].sysops_ptr->value = 3;

		}

		else{

			return -1;
		}

	if (currPCB->fd[fd_used].sysops_ptr->syscall_open(filename) == 0)
		return fd_used;
	else
		return -1;
	}

	return -1;
}

/*
 * int32_t close (int32_t fd)
 * DESCRIPTION: closes a file descriptor
 * INPUT: int32_t fd - file descriptor to close
 * OUTPUT: return 0 on success, -1 on failure
 */
int32_t close (int32_t fd){
	int cur_programid;
	pcb_t * cur_pcb;

	if(fd <2 || fd >7)
		return -1;
	if((cur_programid= find_programid())==-1)
	 		return -1;

	 	cur_pcb = (pcb_t*)(MB_8 - KB_8*cur_programid);

	if (cur_pcb->fd[fd].flags == 0)
		return -1;

	 	cur_pcb->fd[fd].flags = 0;
	 	cur_pcb->fd[fd].sysops_ptr = NULL;
	 	cur_pcb->fd[fd].filePosition = 0;
	 	return 0;
}

/*
 * int32_t getargs (uint8_t* buf, int32_t nbytes)
 * DESCRIPTION: reads program command line arguments into a user-level buffer	
 * INPUT: uint8_t* buf - buffer to read into
 		  int32_t nbytes - number of bytes to read
 * OUTPUT: return 0 on success, -1 on failure
 */
int32_t getargs (uint8_t* buf, int32_t nbytes)
{

	int cur_programid = find_programid();
	pcb_t * currPCB = (pcb_t *)(MB_8 - KB_8*cur_programid);
	int length = strlen((int8_t*)currPCB->argument);

	if(nbytes < length+1) // +1 for null terminating character
	{
		return -1;
	}

	strcpy((int8_t*) buf,(int8_t*)currPCB->argument);

	return 0;
}

/*
 * int32_t vidmap (uint8_t** screen_start) 
 * DESCRIPTION: maps text-mode video memory into user space at a pre-set virtual address
 * INPUT: uint8_t** screen_start - virtual start of the screen
 * OUTPUT: return video memory location on success, -1 on failure
 */
int32_t vidmap (uint8_t** screen_start)
{

	if (screen_start == NULL || screen_start == (uint8_t **)0x400000)
		return -1;

	*screen_start = (uint8_t *)video_mem;

	return ((int32_t)*screen_start);
}

/*
 * int find_programid()
 * DESCRIPTION: returns the number of the program we are currently in
 * INPUT: none
 * OUTPUT: returns the number of the program we are currently in
 */
int find_programid()
{
	uint32_t cr3;
	asm volatile ("movl %%CR3, %0": "=b"(cr3));

	if (cr3 == (uint32_t)page_directory)
		return 0;
	else if (cr3 == (uint32_t)task1_page_directory)
		return 1;
	else if (cr3 == (uint32_t)task2_page_directory)
		return 2;
	else if (cr3 == (uint32_t)task3_page_directory)
		return 3;
	else if (cr3 == (uint32_t)task4_page_directory)
		return 4;
	else if (cr3 == (uint32_t)task5_page_directory)
		return 5;
	else if (cr3 == (uint32_t)task6_page_directory)
		return 6;
	else
	{
		printf("can't find programid for some reason. Gonna pause here.\n");
		while(1);
	}

	return -1;
}

/*
 * void set_programpage(int program_id)
 * DESCRIPTION: sets the page directory to that of the program we are on
 * INPUT: program_id - current program number
 * OUTPUT: none
 */
void set_programpage(int program_id)
{
	uint32_t cr3;
	asm volatile ("movl %%CR3, %0": "=b"(cr3));

	if (program_id == 0)
	{
		cr3 = (uint32_t)page_directory;
	}
	else if (program_id == 1)
	{
		cr3 = (uint32_t)task1_page_directory;
	}
	else if (program_id == 2)
	{
		cr3 = (uint32_t)task2_page_directory;
	}
	else if (program_id == 3)
	{
		cr3 = (uint32_t)task3_page_directory;
	}
	else if (program_id == 4)
	{
		cr3 = (uint32_t)task4_page_directory;
	}
	else if (program_id == 5)
	{
		cr3 = (uint32_t)task5_page_directory;
	}
	else if (program_id == 6)
	{
		cr3 = (uint32_t)task6_page_directory;
	}
	else
	{
		printf("programpage error\n");
		while(1);
	}

	asm volatile("movl %0, %%CR3":: "b"(cr3));
	return;

}

/*
 * int page_loader(const uint8_t * file, int program_id)
 * DESCRIPTION: loads our current program into memory
 * INPUT: const uint8_t * file - name of the file we're loading
 		  int program_id - program's number
 * OUTPUT: 0 on success, -1 on failure
 */
int page_loader(const uint8_t * file, int program_id)
{

	dentry_t file_dentry;
	int32_t inodeNum, numBytes;
	inode_t * inode_ptr;

	if (read_dentry_by_name(file, &file_dentry) == -1)
	{
		printf("didn't find file by name\n");
		while(1);
		return -1;
	}
	inodeNum = file_dentry.inodeNum;
	inode_ptr = get_inodeptr(inodeNum);
	numBytes = inode_ptr->length;

	uint8_t * buf = (uint8_t*)PROGRAM_MEM;

	set_programpage(program_id);


	if (read_data(inodeNum, 0, buf, numBytes) == -1)
	{
		printf("readdata error\n");
		while(1);
	}

	return 0;
}

