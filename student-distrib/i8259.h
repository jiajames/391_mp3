/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT 0x20
#define SLAVE_8259_PORT  0xA0
#define PIC1_COMMAND	MASTER_8259_PORT
#define PIC1_DATA	(MASTER_8259_PORT+1)
#define PIC2_COMMAND	SLAVE_8259_PORT
#define PIC2_DATA	(SLAVE_8259_PORT+1)
/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1    0x11
#define ICW2_MASTER   0x20
#define ICW2_SLAVE    0x28
#define ICW3_MASTER   0x04
#define ICW3_SLAVE    0x02
#define ICW4          0x01

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI             0x60


// driver magic number Defines
#define KYBD_ADDR     0x60
#define KEY_RELEASED  0x80

#define RTC_ADDR      0x70
#define CMOS_ADDR     0x71
#define SEL_REGA      0x8A
#define SEL_REGB      0x8B
#define SEL_REGC      0x8C
#define RTC_DIV       15
#define MASK_L4       0xF0
#define BIT7          0x40

#define MAXPITFREQ    1193180

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);
/* init PIT */
void init_PIT(int hertz);

//void init_rtc();
void irq_handler(uint32_t irq_num);

// pic irq handlers
extern void irq0();
extern void irq1(); // keyboard irq
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8(); // rtc irq
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

#endif /* _I8259_H */
