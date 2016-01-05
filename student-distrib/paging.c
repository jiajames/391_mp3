#include "paging.h"


/*	void paging_initialize()
	function: initializes the paging by turning off segmentation (mostly) in x86_desc
		Sets pages for our tasks and then enables pagiong
	input: nothing!
	returns: nothing!
*/

void paging_initialize()
{
	int i;
	uint32_t cr4, cr0;

	// enable PSE to allow 4 MB pages (writing to cr4)
	asm volatile ("mov %%CR4, %0": "=b"(cr4)); 	
	cr4 |= PSE_EN;
	asm volatile ("mov %0, %%CR4":: "b"(cr4));

	// initialize our directory and table
	for(i = INIT_NULL; i < PAGE_SIZE; i++)
	{
		page_directory[i] = INIT_NULL;
		page1_table[i] = INIT_NULL;

		task1_page_directory[i] = INIT_NULL;
		task1_page1_table[i] = INIT_NULL;

		task2_page_directory[i] = INIT_NULL;
		task2_page1_table[i] = INIT_NULL;		

		task3_page_directory[i] = INIT_NULL;
		task3_page1_table[i] = INIT_NULL;	

		task4_page_directory[i] = INIT_NULL;
		task4_page1_table[i] = INIT_NULL;	

		task5_page_directory[i] = INIT_NULL;
		task5_page1_table[i] = INIT_NULL;	

		task6_page_directory[i] = INIT_NULL;
		task6_page1_table[i] = INIT_NULL;	
	}

	// directory points to our first page and turning on bits
	page_directory[PAGE_1] = ((uint32_t)page1_table | PRESENT | READ_WRITE | USER_SV);
	page_directory[KERNEL_PAGE] = (KERNEL_MEM | SIZE_4MB | PRESENT);
	page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV);

	task1_page_directory[PAGE_1] = ((uint32_t)task1_page1_table | PRESENT | READ_WRITE | USER_SV);
	task1_page_directory[KERNEL_PAGE] = (KERNEL_MEM | SIZE_4MB | PRESENT);
	task1_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
	task1_page_directory[32] = (0x800000 | SIZE_4MB | PRESENT | USER_SV | READ_WRITE);

	task2_page_directory[PAGE_1] = ((uint32_t)task2_page1_table | PRESENT | READ_WRITE | USER_SV);
	task2_page_directory[KERNEL_PAGE] = (KERNEL_MEM | SIZE_4MB | PRESENT);
	task2_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
	task2_page_directory[32] = (0xC00000 | SIZE_4MB | PRESENT | USER_SV | READ_WRITE);

	task3_page_directory[PAGE_1] = ((uint32_t)task3_page1_table | PRESENT | READ_WRITE | USER_SV);
	task3_page_directory[KERNEL_PAGE] = (KERNEL_MEM | SIZE_4MB | PRESENT);
	task3_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
	task3_page_directory[32] = (0x1000000 | SIZE_4MB | PRESENT | USER_SV | READ_WRITE);

	task4_page_directory[PAGE_1] = ((uint32_t)task4_page1_table | PRESENT | READ_WRITE | USER_SV);
	task4_page_directory[KERNEL_PAGE] = (KERNEL_MEM | SIZE_4MB | PRESENT);
	task4_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
	task4_page_directory[32] = (0x1400000 | SIZE_4MB | PRESENT | USER_SV | READ_WRITE);

	task5_page_directory[PAGE_1] = ((uint32_t)task5_page1_table | PRESENT | READ_WRITE | USER_SV);
	task5_page_directory[KERNEL_PAGE] = (KERNEL_MEM | SIZE_4MB | PRESENT);
	task5_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
	task5_page_directory[32] = (0x1800000 | SIZE_4MB | PRESENT | USER_SV | READ_WRITE);

	task6_page_directory[PAGE_1] = ((uint32_t)task6_page1_table | PRESENT | READ_WRITE | USER_SV);
	task6_page_directory[KERNEL_PAGE] = (KERNEL_MEM | SIZE_4MB | PRESENT);
	task6_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
	task6_page_directory[32] = (0x1C00000 | SIZE_4MB | PRESENT | USER_SV | READ_WRITE);

	// page_directory moved to register CR3
	asm volatile ("mov %0, %%CR3":: "b"(page_directory));
	
	// enable paging in cr0 register
	asm volatile ("mov %%CR0, %0": "=b"(cr0));
	cr0 |= PAGING_EN; // enables paging
	asm volatile ("mov %0, %%CR0":: "b"(cr0));
}


/*	void video_mapping(int program_id)
	function: maps video memory for the program passed in to write to
	input: int program_id, usually what is returned by get_programid()
	returns: nothing!
*/

void video_mapping(int program_id)
{
	switch(program_id)
	{
		case 1:
			task1_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
			return;
		case 2:
			task2_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
			return;
		case 3:
			task3_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
			return;
		case 4:
			task4_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
			return;
		case 5:
			task5_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
			return;
		case 6:
			task6_page1_table[VIDEO_PAGE] = (VIDEO_MEM | PRESENT | USER_SV | READ_WRITE);
			return;
		default:
			return;
	}
}

void non_video_mapping(int program_id, int terminal)
{
	switch(program_id)
	{
		case 1:
			switch(terminal)
			{
				case 0:
					task1_page1_table[VIDEO_PAGE] = (term0_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 1:
					task1_page1_table[VIDEO_PAGE] = (term1_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 2:
					task1_page1_table[VIDEO_PAGE] = (term2_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				default: 
					return;
			}
		case 2:
			switch(terminal)
			{
				case 0:
					task2_page1_table[VIDEO_PAGE] = (term0_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 1:
					task2_page1_table[VIDEO_PAGE] = (term1_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 2:
					task2_page1_table[VIDEO_PAGE] = (term2_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				default: 
					return;
			}
		case 3:
			switch(terminal)
			{
				case 0:
					task3_page1_table[VIDEO_PAGE] = (term0_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 1:
					task3_page1_table[VIDEO_PAGE] = (term1_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 2:
					task3_page1_table[VIDEO_PAGE] = (term2_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				default: 
					return;
			}
		case 4:
			switch(terminal)
			{
				case 0:
					task4_page1_table[VIDEO_PAGE] = (term0_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 1:
					task4_page1_table[VIDEO_PAGE] = (term1_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 2:
					task4_page1_table[VIDEO_PAGE] = (term2_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				default: 
					return;
			}
		case 5:
			switch(terminal)
			{
				case 0:
					task5_page1_table[VIDEO_PAGE] = (term0_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 1:
					task5_page1_table[VIDEO_PAGE] = (term1_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 2:
					task5_page1_table[VIDEO_PAGE] = (term2_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				default: 
					return;
			}
		case 6:
			switch(terminal)
			{
				case 0:
					task6_page1_table[VIDEO_PAGE] = (term0_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 1:
					task6_page1_table[VIDEO_PAGE] = (term1_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				case 2:
					task6_page1_table[VIDEO_PAGE] = (term2_buffer | PRESENT | USER_SV | READ_WRITE);
					return;
				default: 
					return;
			}
		default:
			return;
	}
}




