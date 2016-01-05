// #ifndef PAGING_H
// #define PAGING_H

// #include "types.h"
// #include "lib.h"
// #include "x86_desc.h"

// #define VIDEO_MEM 0xB8000
// #define KERNEL_MEM 0x400000
// #define VIDEO_PAGE 184

// #define INIT_NULL 0x0
// #define PAGE_SIZE 1024

// #define PSE_EN 0x00000010
// #define PAGING_EN 0x80000000

// #define PAGE_1 0x0
// #define KERNEL_PAGE 0x1

// #define PRESENT 0x01
// #define READ_WRITE 0x02
// #define USER_SV 0x04
// #define SIZE_4MB 0x80

// uint32_t page_directory[1024] __attribute__ ((aligned(4096))); 
// uint32_t page1_table[1024] __attribute__((aligned(4096)));

// uint32_t task1_page_directory[1024] __attribute__((aligned(4096)));  
// uint32_t task1_page1_table[1024] __attribute__((aligned(4096))); 

// uint32_t task2_page_directory[1024] __attribute__((aligned(4096))); 
// uint32_t task2_page1_table[1024] __attribute__((aligned(4096))); 

// uint32_t task3_page_directory[1024] __attribute__((aligned(4096))); 
// uint32_t task3_page1_table[1024] __attribute__((aligned(4096))); 


// extern void paging_initialize();

// #endif



#ifndef PAGING_H
#define PAGING_H

#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "terminal.h"

#define VIDEO_MEM 0xB8000
#define KERNEL_MEM 0x400000
#define VIDEO_PAGE 184

#define INIT_NULL 0x0
#define PAGE_SIZE 1024

#define PSE_EN 0x00000010
#define PAGING_EN 0x80000000

#define PAGE_1 0x0
#define KERNEL_PAGE 0x1

#define PRESENT 0x01
#define READ_WRITE 0x02
#define USER_SV 0x04
#define SIZE_4MB 0x80

uint32_t page_directory[1024] __attribute__ ((aligned(4096))); 
uint32_t page1_table[1024] __attribute__((aligned(4096)));

uint32_t task1_page_directory[1024] __attribute__((aligned(4096)));  
uint32_t task1_page1_table[1024] __attribute__((aligned(4096))); 

uint32_t task2_page_directory[1024] __attribute__((aligned(4096))); 
uint32_t task2_page1_table[1024] __attribute__((aligned(4096))); 

uint32_t task3_page_directory[1024] __attribute__((aligned(4096))); 
uint32_t task3_page1_table[1024] __attribute__((aligned(4096))); 

uint32_t task4_page_directory[1024] __attribute__((aligned(4096))); 
uint32_t task4_page1_table[1024] __attribute__((aligned(4096))); 

uint32_t task5_page_directory[1024] __attribute__((aligned(4096))); 
uint32_t task5_page1_table[1024] __attribute__((aligned(4096))); 

uint32_t task6_page_directory[1024] __attribute__((aligned(4096))); 
uint32_t task6_page1_table[1024] __attribute__((aligned(4096))); 


extern void paging_initialize();
void video_mapping(int program_id);
void non_video_mapping(int program_id, int terminal);

#endif



