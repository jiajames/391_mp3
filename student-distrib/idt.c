#include "x86_desc.h"
#include "types.h"
#include "lib.h"
#include "idt.h"
#include "idt_linkage.h"
#include "i8259.h"
#include "systemcall.h"

/*
+ all functions following until idt_initialize() are pretty much the same
+ void [exception_type](void)
+ 	function: prints an error screen and basically freezes the OS if we mess up
+        the error and name explain the nature of the exception
+ 	input: nothing!
+ 	returns: nothing!
+*/

void
divide_by_zero(void)
{
    clear();
    printf("divide by zero exception\n");
    while(1);
}

void
debug(void)
{
    clear();
    printf(" debug exceptiond\n");
    while(1);

}
void
nmi(void)
{
    clear();
    printf("non-maskable interrupt\n");
    while(1);

}
void
breakpoint(void)
{
    clear();
    printf("breakpoint exception\n");
    while(1);

}
void
overflow(void)
{
    clear();
    printf("overflow exception\n");
    while(1);

}
void
bound_range(void)
{
    clear();
    printf("bound range exception\n");
    while(1);

}
void
invalid_op(void)
{
    clear();
    printf("invalid opcode exception\n");
    while(1);

}
void
device_na(void)
{
    clear();
    printf("device not avaliable exception\n");
    while(1);

}

void
double_fault(void)
{
    clear();
    printf("double fault exception\n");
    while(1);

}

void
coprocessor_seg_overrun(void)
{
    clear();
    printf("coprocessor segmentation overrun exception\n");
    while(1);

}
void
invalid_tss(void)
{
    clear();
    printf("invalid task state segment exception\n");
    while(1);

}
void
seg_not_present(void)
{
    clear();
    printf("segment not present exception\n");
    while(1);

}

void
stack_seg_fault(void)
{
    clear();
    printf("stack segmentation fault \n");
    while(1);

}

void
general_protection(void)
{
    clear();
    printf("general protection fault\n");
    while(1);

}	

void
page_fault(void)
{
    clear();
    int addr;
    printf("page fault \n");
    asm volatile("movl %%cr2, %0"
    			 :"=r"(addr)
    			 :
    			 :"memory");
    printf("addr:%x\n", addr);
    while(1);

}
void
reserved(void)
{
    clear();
    printf("reserved exception \n");
    while(1);

}

void
math_fault(void)
{
    clear();
    printf("math fault\n");
    while(1);
}
void
alignment_check(void)
{
    clear();
    printf("alignment check\n");
    while(1);

}

void
machine_check(void)
{
    clear();
    printf("machine check\n");
    while(1);

}

void
floating_point(void)
{
    clear();
    printf("floating point excecption\n");
    while(1);

}


/*	void idt_initialize()
	function: initializes the IDT with appropriate registers for interrupt gates,
        syscalls, and links the IDT entries to function pointers. then loads the
        idt table
	input: nothing!
	returns: nothing!
*/


void idt_initialize()
{
	int i;

	idt_desc_t exception;

	exception.seg_selector = KERNEL_CS;
	exception.reserved4 = set_0;
	exception.reserved3 = set_0;
	exception.reserved2 = set_1;
	exception.reserved1 = set_1;
	exception.size = set_1;
	exception.reserved0 = set_0;
	exception.dpl = set_0;
	exception.present = set_1;

	idt_desc_t interrupt;
	interrupt = exception;

	//interrupt.reserved3 = set_1;

	idt_desc_t syscall;
	syscall = interrupt;

	syscall.dpl = set_3;
	syscall.reserved3 = set_1;

	for(i = 0; i < 32; i++)
		idt[i] = exception;

	for(i = 32; i < 41; i++)
		idt[i] = interrupt;

	idt[0x80] = syscall;

	SET_IDT_ENTRY(idt[0], &divide_by_zero_linkage);
	SET_IDT_ENTRY(idt[1], &debug_linkage);
	SET_IDT_ENTRY(idt[2], &nmi_linkage);
	SET_IDT_ENTRY(idt[3], &breakpoint_linkage);
	SET_IDT_ENTRY(idt[4], &overflow_linkage);
	SET_IDT_ENTRY(idt[5], &bound_range_linkage);
	SET_IDT_ENTRY(idt[6], &invalid_op_linkage);
	SET_IDT_ENTRY(idt[7], &device_na_linkage);
	SET_IDT_ENTRY(idt[8], &double_fault_linkage);
	SET_IDT_ENTRY(idt[9], &coprocessor_seg_overrun_linkage);
	SET_IDT_ENTRY(idt[10], &invalid_tss_linkage);
	SET_IDT_ENTRY(idt[11], &seg_not_present_linkage);
	SET_IDT_ENTRY(idt[12], &stack_seg_fault_linkage);
	SET_IDT_ENTRY(idt[13], &general_protection_linkage);
	SET_IDT_ENTRY(idt[14], &page_fault_linkage);
	SET_IDT_ENTRY(idt[15], &reserved_linkage);
	SET_IDT_ENTRY(idt[16], &math_fault_linkage);			
	SET_IDT_ENTRY(idt[17], &alignment_check_linkage);
	SET_IDT_ENTRY(idt[18], &machine_check_linkage);
	SET_IDT_ENTRY(idt[19], &floating_point_linkage);

	// we add 32 to the IRQ number because of the offset
	SET_IDT_ENTRY(idt[32], &irq0_wrapper); // add PIT irq (IRQ0+32 = 32)
	SET_IDT_ENTRY(idt[33], &irq1_wrapper); // add keyboard irq (IRQ1+32 = 33)
	SET_IDT_ENTRY(idt[40], &irq8_wrapper); // add rtc irq      (IRQ8+32 = 40)

	// add system call handler
	SET_IDT_ENTRY(idt[0x80], &syscall_wrapper);

	lidt(idt_desc_ptr);

}
