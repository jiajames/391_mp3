#include "file_system.h"
#include "lib.h"
#include "pcb.h"
#include "systemcall.h"

static bootBlock_t * bootHead; //static head of fiel system

static int32_t currentDirIndex;

/*
 * int32_t init_bootBlock_address(bootBlock_t * address)
 * DESCRIPTION: initializes our bootBlock and saves it for local use
 * INPUT: bootBlock_t * address - address of our bootBlock
 * OUTPUT: always 0
 */
int32_t init_bootBlock_address(bootBlock_t * address)
{
	bootHead = address; 
	currentDirIndex = 0; // initialize the directory index to 0
	return 0;
}

/*
 * int32_t test_file_system()
 * DESCRIPTION: tests used for our checkpoint grading
 * INPUT: none
 * OUTPUT: ignore
 */
int32_t 
test_file_system()
{

	/***********************************************************************************
										TEST 1
	Print out a text file and/or an executable. 
	This should be easily changable so that we can choose whatever file we want to read.
	***********************************************************************************/
	/*
	clear();
	int i;
	int screen = 0;
	int size = 40000;
	uint8_t buffer[size];
	uint8_t name[40] = "verylargetxtwithverylongname.txttttt\0"; // Names should be < 32 characters. If its longer it will truncate to 31 char with "\0" terminating character
	int32_t a = read_file(name, buffer, size);
	if (a == -1)
	{
		printf("File read error.\n");
		while(1);
	}

	for (i = 0; i < a; i++)
	{
		printf("%c", buffer[i]);
		screen++;
		if ((char)buffer[i] == (char)("\n") || screen > 80)
		{
			screen = 0;
			if (screen > 80)
				printf("\n");
		}

	}

	while(1);
	*/

	/***********************************************************************************
										TEST 2
	Print out the size in bytes of a text file and/or an executable.
	***********************************************************************************/
	/*
	clear();
	int size = 40000;
	uint8_t buffer[size];
	uint8_t name[40] = "verylargetxtwithverylongname.txttttt\0";
	int32_t a = read_file(name, buffer, size);
	if (a == -1)
	{
		printf("File read error.\n");
		while(1);
	}
	printf("File Size (in bytes): %d\n", a);
	while(1);
	*/
	/***********************************************************************************
										TEST 3
	Print all filenames on the fs image. 
	Take a look at how the ls(syscalls/ece391ls.c) program does this. 
	It involves directory read.
	***********************************************************************************/
	/*		
	clear();
	uint8_t buffer[32];
	int i;
	for (i = 0; i < 62; i++)
	{
		int a = read_directory(buffer);
		if (a == -1)
		{
			printf("Done reading directory.\n");
			while(1);
		}
		else
			printf("Directory %d: %s\n", i, buffer);
	}

	while(1);
	*/
	return 0;
}

/*
 * int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
 * DESCRIPTION: finds file by name and returns data for it
 * INPUT: const uint8_t* fname - file name we are searching for
 *        dentry_t* dentry - struct that contains valuable file data
 * OUTPUT: 0 if file found, -1 if file not found
 */
int32_t
read_dentry_by_name (const uint8_t* fname, dentry_t* dentry)
{
	/* 
	Think of it as a dictionary searching for a file,
	with the key being the filename. If found, it'll return a dentry
	struct and return 0 if found
	*/
	int8_t tempString[CHAR_LENGTH];
	int i, length;

	strcpy(tempString, (int8_t *)fname);

	length = strlen((int8_t *)fname);

	for ( i = strlen((int8_t *)fname); i < CHAR_LENGTH ; i++)
	{
		tempString[i] = 0x0; // store 0 for padding 
	}

	if (length >= CHAR_LENGTH)
		tempString[CHAR_LENGTH] = (int8_t)('\0');

	for(i = 0; i < NUM_DIR_ENTRIES; i++)
	{
		if (!strncmp(tempString, (bootHead->dir_Entries[i]).fileName, CHAR_LENGTH -1))
		{	
			*dentry = bootHead -> dir_Entries[i];
			return 0; 
		}
	}

	//if nothing is found, return failure
	return -1;

}

/*
 * int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry)
 * DESCRIPTION: finds file by index and returns data for it
 * INPUT: uint32_t index - index to find file for
 *        dentry_t* dentry - struct that contains valuable file data
 * OUTPUT: 0 if file found, -1 if file not found
 */
int32_t
read_dentry_by_index (uint32_t index, dentry_t* dentry)
{	
	/* 
	Same as read by name but the key being the index
	*/
	if(index < 0 || index >= bootHead->numDirectory || dentry == NULL)
		return -1;

	*dentry = bootHead->dir_Entries[index];

	return 0;


}

/*
 * int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * DESCRIPTION: reads data from a file in the file system into a buffer
 * INPUT: uint32_t inode - index node of our file
 		  uint32_t offset - number of bytes to offset our read
 		  uint8_t* buf - buffer we are copying data into
 		  uint32_t length - number of bytes to read
 * OUTPUT: number of bytes read, or -1 if file not found
 */
int32_t
read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
	int i;
	int32_t blockIndex, blockNUM, blockOffset, data_length;
	uint8_t * data_ptr;
	uint8_t * data_end ; 
	inode_t * inode_ptr;

	blockNUM = offset/FOUR_KILOBYTES; //calculate which block index to start on;
	blockOffset = offset%FOUR_KILOBYTES;
	inode_ptr = (inode_t * ) (& bootHead[inode + 1]); // get the inode pointer
 	data_length = inode_ptr->length; 

 	if( offset == data_length)
 		return 0;

	//check whether the inode is out of range, or buffer is null
	if ( (inode >= bootHead->numInode)  || inode < 0 
		|| buf == NULL //|| inode_ptr->length > offset
		|| offset < 0 || length <= 0 )  
		return -1; 

	if (length >= (data_length - offset))
		length = (data_length - offset);

	blockIndex = inode_ptr->dataBlock[blockNUM] + bootHead->numInode +1; // go the the beginning of the data block 
		
	data_ptr = (uint8_t *)(&bootHead[blockIndex]); //point to the head of the block
	data_ptr += blockOffset; // points to the location of the first memory
	data_end = data_ptr + FOUR_KILOBYTES - blockOffset -1; // store the location where the data block end 

		for ( i = 0; i < length ; i ++)
		{
			buf[i] = (char)(*data_ptr) ;
	
			/* dont forget the case where the pointer is at the end of file!~!!!!! ! !! !  */
			if(data_ptr == data_end) // if this uses up all the block, jump to the next block
			{
				blockNUM++;
				blockIndex = inode_ptr->dataBlock[blockNUM] + bootHead->numInode +1; // go the the beginning of the data block 
				data_ptr = (uint8_t *)(&bootHead[blockIndex]); //point to the head of the block
				data_end = data_ptr + FOUR_KILOBYTES -1; // store the location where the data block end 
			}
			else
			{
				data_ptr++;
			}

		}

		return i;

}

/*
 * int32_t open_file (const uint8_t* filename)
 * DESCRIPTION: opens file from filesystem
 * INPUT: const uint8_t* filename - filename to look for
 * OUTPUT: 0 on success, 1 on failure
 */
int32_t open_file (const uint8_t* filename)
{
	dentry_t tempDentry;
	return read_dentry_by_name(filename, &tempDentry);
}

/*
 * int32_t close_file (int32_t fd)
 * DESCRIPTION: to fit systemcall format. does not do anything
 * INPUT: int32_t fd - ignored
 * OUTPUT: 0 always
 */
int32_t close_file (int32_t fd)
{
	return 0;
}

/*
 * int32_t read_file  (int32_t fd, void *buf, int32_t bytes)
 * DESCRIPTION: reads a file from the filesystem into a buffer
 * INPUT: int32_t fd - file descriptor
 		  void *buf - buffer to read into
 		  int32_t bytes - number of bytes to read
 * OUTPUT: 0 on file completely read, -1 on failure to read, or number of bytes read
 */
int32_t read_file  (int32_t fd, void *buf, int32_t bytes)
{

	dentry_t tempDentry;
	int returnVal;
	uint32_t inodeIndex;
	uint32_t offset;
	int byteRead;

	int cur_programid = find_programid();
	pcb_t * currPCB = (pcb_t *)(MB_8 - KB_8*cur_programid);

	int8_t *filename = currPCB->fd[fd].fileName;
	returnVal = read_dentry_by_name((uint8_t*)filename, &tempDentry);

	if(returnVal == -1)
		return -1;

	offset = currPCB->fd[fd].filePosition;

	inodeIndex = tempDentry.inodeNum;

	byteRead = read_data(inodeIndex, offset, (uint8_t*)buf, bytes);

	if(byteRead == -1) //read data error
		return -1;

	if(byteRead == 0) //reach end of file, return file length
	{	
		currPCB->fd[fd].filePosition = ((inode_t * )(& bootHead[tempDentry.inodeNum + 1]))->length;	
		return 0;
	}
	else // default case
	{
		currPCB->fd[fd].filePosition = byteRead + offset;
		return byteRead;
	}
}

/*
 * int32_t read_file_sys_helper (int32_t fd, void *buf, int32_t bytes)
 * DESCRIPTION: reads a file from the filesystem into a buffer
 * INPUT: int32_t fd - file descriptor
 		  void *buf - buffer to read into
 		  int32_t bytes - number of bytes to read
 * OUTPUT: 0 on file completely read, -1 on failure to read, or number of bytes read
 */
int32_t read_file_sys_helper  (const uint8_t * filename, const void *buf, int32_t bytes)
{

	dentry_t tempDentry;
	int returnVal;
	uint32_t inodeIndex;
	uint32_t offset = 0;
	int byteRead;

	returnVal = read_dentry_by_name(filename, &tempDentry);

	if(returnVal == -1)
		return -1;

	inodeIndex = tempDentry.inodeNum;

	byteRead = read_data( inodeIndex, offset, (uint8_t*) buf, bytes);

	if(byteRead == -1) //read data error
		return -1;
	if(byteRead == 0) //reach end of file, return file length
		return ((inode_t * )(& bootHead[tempDentry.inodeNum + 1]))->length;
	else // default case
		return byteRead;

}

/*
 * int32_t write_file (int32_t fd, const void* buf, int32_t nbytes)
 * DESCRIPTION: to fit systemcall format. does not do anything and should not be called
 * INPUT: ignored
 * OUTPUT: -1 always
 */
int32_t write_file (int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

/*
 * int32_t open_directory (const uint8_t* filename)
 * DESCRIPTION: to fit systemcall format. does not do anything
 * INPUT: ignored
 * OUTPUT: 0 always
 */
int32_t open_directory (const uint8_t* filename)
{
	return 0;
}

/*
 * int32_t close_directory (int32_t fd)
 * DESCRIPTION: to fit systemcall format. does not do anything
 * INPUT: ignored
 * OUTPUT: 0 always
 */
int32_t close_directory (int32_t fd)
{
	return 0;
}

/*
 * int32_t read_directory  (int32_t fd, void *buf, int32_t bytes)
 * DESCRIPTION: reads the next unread file from the directory into a buffer
 * INPUT: int32_t fd - file descriptor
 		  void *buf - buffer to read into
 		  int32_t bytes - number of bytes to read
 * OUTPUT: 0 on file completely read, -1 on failure to read, or number of bytes read
 */
int32_t read_directory(int32_t fileName, void* buf, int32_t nbytes)
{	
	
	if(currentDirIndex +1 > bootHead->numDirectory)
		{	
			currentDirIndex = 0;
			return 0;
		}

	strcpy((int8_t *)buf, (int8_t *)(bootHead->dir_Entries[currentDirIndex].fileName));//, strlen(bootHead->dir_Entries[currentDirIndex].fileName));
	
	currentDirIndex++;

	return strlen(buf);
}

/*
 * int32_t write_directory (int32_t fd, const void* buf, int32_t nbytes)
 * DESCRIPTION: to fit systemcall format. does not do anything and should not be called
 * INPUT: ignored
 * OUTPUT: -1 always
 */
int32_t write_directory (int32_t fd, const void* buf, int32_t nbytes)
{
	return -1;
}

/*
 * inode_t * get_inodeptr(int32_t inodeNum)
 * DESCRIPTION: grades the inodeptr for other functions to use
 * INPUT: int32_t inodeNum - inode number to grab ptr for
 * OUTPUT: inode ptr for the inodeNum provided
 */
inode_t * get_inodeptr(int32_t inodeNum)
{
	return (inode_t*) &bootHead[inodeNum + 1];
}

