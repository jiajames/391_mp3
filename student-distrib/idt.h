#ifndef IDT_H
#define IDT_H


#define set_0 0x00
#define set_1 0x01
#define set_3 0x03


 extern void divide_by_zero(void);
 extern void debug(void);
 extern void nmi(void);
 extern void breakpoint(void);
 extern void overflow(void);
 extern void bound_range(void);
 extern void invalid_op(void);
 extern void device_na(void);
 extern void double_fault(void);
 extern void coprocessor_seg_overrun(void);
 extern void invalid_tss(void);
 extern void seg_not_present(void);
 extern void stack_seg_fault(void);
 extern void general_protection(void);
 extern void page_fault(void);
 extern void reserved(void);
 extern void math_fault(void);
 extern void alignment_check(void);
 extern void machine_check(void);
 extern void floating_point(void);

extern uint32_t irq0_wrapper;
extern uint32_t irq1_wrapper;
extern uint32_t irq8_wrapper;
extern uint32_t syscall_wrapper;
extern int32_t esp_0;
extern int32_t ebp_0;

extern uint32_t int_ret_helper(int32_t ebp, int32_t esp);

extern void idt_initialize();

#endif
