#ifndef IDT_LINKAGE_H
#define IDT_LINKAGE_H

#include "idt.h"

void divide_by_zero_linkage();
void debug_linkage();
void nmi_linkage();
void breakpoint_linkage();
void overflow_linkage();
void bound_range_linkage();	
void invalid_op_linkage();
void device_na_linkage();
void double_fault_linkage();
void coprocessor_seg_overrun_linkage();
void invalid_tss_linkage();
void seg_not_present_linkage();
void stack_seg_fault_linkage();
void general_protection_linkage();
void page_fault_linkage();
void reserved_linkage();
void math_fault_linkage();
void alignment_check_linkage();
void machine_check_linkage();
void floating_point_linkage();



#endif 
