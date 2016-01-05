 
.globl divide_by_zero_linkage
.globl debug_linkage
.globl nmi_linkage
.globl breakpoint_linkage
.globl overflow_linkage
.globl bound_range_linkage
.globl invalid_op_linkage
.globl device_na_linkage
.globl double_fault_linkage, coprocessor_seg_overrun_linkage, invalid_tss_linkage, seg_not_present_linkage, stack_seg_fault_linkage, general_protection_linkage, page_fault_linkage, reserved_linkage, math_fault_linkage, alignment_check_linkage, machine_check_linkage
.globl floating_point_linkage

.globl divide_by_zero, debug, nmi, breakpoint, overflow, bound_range, invalid_op, device_na, double_fault, coprocessor_seg_overrun, invalid_tss, seg_not_present, stack_seg_fault, general_protection, page_fault, reserved, math_fault, alignment_check, machine_check, floating_point 
.globl int_ret_helper, irq0_wrapper, irq1_wrapper, irq8_wrapper, syscall_wrapper, returnValue, esp_0, ebp_0

.align 4

returnValue:	.long 0x0
esp_0:			.long 0x0
ebp_0:			.long 0x0

divide_by_zero_linkage:
 
pusha
call divide_by_zero
popa
iret 

debug_linkage:
 
pusha
call debug
popa
iret 

nmi_linkage:
 
pusha
call nmi
popa
iret 


breakpoint_linkage:
 
pusha
call breakpoint
popa
iret 


overflow_linkage:
 
pusha
call overflow
popa
iret 


bound_range_linkage:
 
pusha
call bound_range
popa
iret 


invalid_op_linkage:
 
pusha
call invalid_op
popa
iret 


device_na_linkage:
 
pusha
call device_na
popa
iret 


double_fault_linkage:
 
pusha
call double_fault
popa
iret 


coprocessor_seg_overrun_linkage:
 
pusha
call coprocessor_seg_overrun
popa
iret 


invalid_tss_linkage:
 
pusha
call invalid_tss
popa
iret 


seg_not_present_linkage:
 
pusha
call seg_not_present
popa
iret 


stack_seg_fault_linkage:
 
pusha
call stack_seg_fault
popa
iret 


general_protection_linkage:
 
pusha
call general_protection
popa
iret 


page_fault_linkage:
 
pusha
call  page_fault
popa
iret 


reserved_linkage:
 
pusha
call reserved
popa
iret 


math_fault_linkage:
 
pusha
call math_fault
popa
iret 


alignment_check_linkage:
 
pusha
call alignment_check
popa
iret 


machine_check_linkage:
 
pusha
call machine_check
popa
iret 


floating_point_linkage:
 
pusha
call floating_point
popa	
iret 

int_ret_helper:
	movl 4(%esp), %ebp
	movl 8(%esp), %esp
	iret

irq0_wrapper:
	pusha
	pushl $0
	call irq_handler
	popl %eax
	popa
	iret

irq1_wrapper:
	movl %esp, esp_0
	movl %ebp, ebp_0
	pusha
	pushl $1
	call irq_handler
	popl %eax
	popa
	iret

irq8_wrapper:
	pusha
	pushl $8
	call irq_handler
	popl %eax
	popa
	iret

syscall_wrapper:
		pusha
		pushl %edx
		pushl %ecx
		pushl %ebx
		cmpl $1, %eax
		jl done
		cmpl $8, %eax
		jg done
		call *syscall_array(,%eax,4)
		popl %ebx
		popl %ecx
		popl %edx
		movl %eax, returnValue
		popa
		movl returnValue, %eax
		iret
done:	popl %ebx
		popl %ecx
		popl %edx
		popa
		movl $-1, %eax
		iret

syscall_array:
	.long 0x0
	.long halt, execute, read, write, open, close, getargs, vidmap




















