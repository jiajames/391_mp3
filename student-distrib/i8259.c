/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

#include "x86_desc.h"
#include "lib.h"
#include "idt.h"
#include "systemcall.h"
#include "terminal.h"
#include "rtc.h"
#include "scheduler.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
// we should probably keep track of these in irq_enable/disable() instead of always reading them. meh
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */


void* irq_functions[16] = {irq0,irq1,0,0,0,0,0,0,irq8,0,0,0,0,0,0,0};

/* Initialize the 8259 PIC */
/*	void i8259_init(void)
	function: initializes the master and slave PIC by sending the
	right command/config words
	input: nothing!
	returns: nothing!
*/
void
i8259_init(void)
{
	outb(ICW1,PIC1_COMMAND);  // starts the initialization sequence (in cascade mode)
	outb(ICW1,PIC2_COMMAND);
	outb(ICW2_MASTER, PIC1_DATA);     // ICW2: Master PIC vector offset
	outb( ICW2_SLAVE, PIC2_DATA);      // ICW2: Slave PIC vector offset
	outb(ICW3_MASTER, PIC1_DATA);     // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb( ICW3_SLAVE, PIC2_DATA);     // ICW3: tell Slave PIC its cascade identity (0000 0010)

	outb(ICW4, PIC1_DATA);
	outb(ICW4, PIC2_DATA);

    master_mask = 0xFF; // initially mask all interrupts for master and slave
    slave_mask  = 0xFF;
	outb(master_mask, PIC1_DATA);   // mask all interrupts initially
	outb(slave_mask, PIC2_DATA);
}

/* Enable (unmask) the specified IRQ */
/*	void enable_irq(uint32_t irq_num)
	function: enables the selected IRQ (unmasks it on the i8259)
	input: uint32_t irq_num, the selected irq to enable
	returns: nothing!
*/
void
enable_irq(uint32_t irq_num)
{
	uint16_t port;
    uint8_t value;

    if(irq_num < 8) {   // if irq less than 8
        port = PIC1_DATA;   // send request to master pic
    } else {
        enable_irq(2);  // slave irq enables require irq2 (cascade) to be enabled
        port = PIC2_DATA;   // but also send this request to slave pic
        irq_num -= 8;   // with offset of -8
    }
    value = inb(port) & ~(1 << irq_num); // subtract the selected irq from the interrupt mask
    outb(value, port);
}

/* Disable (mask) the specified IRQ */
/*	void disable_irq(uint32_t irq_num)
	function: disables the selected IRQ (masks it on the i8259)
	input: uint32_t irq_num, the selected irq to disable
	returns: nothing!
*/
void
disable_irq(uint32_t irq_num)
{
	uint16_t port;
    uint8_t value;

    if(irq_num < 8) {   // if irq less than 8
        port = PIC1_DATA;   // send request to master pic
    } else {
        port = PIC2_DATA;   // send request to slave pic
        irq_num -= 8;        // with offset of -8
    }
    value = inb(port) | (1 << irq_num); // add the selected irq to the interrupt mask
    outb(value, port);
}

/* Send end-of-interrupt signal for the specified IRQ */
/*	void send_eoi(uint32_t irq_num)
	function: sends EOI to i8259 PICs, including support for slave cascaded PIC
	input: uint32_t irq_num, the irq to send an EOI for
	returns: nothing!
*/
void
send_eoi(uint32_t irq_num)
{
    // send irq_num OR EOI to pics
	if(irq_num >= 8){
		outb(EOI|(irq_num-8), PIC2_COMMAND);  // on >=8 irq, alert slave
        outb(EOI|2, PIC1_COMMAND);            // and master
    }
    else
	   outb(EOI|irq_num, PIC1_COMMAND);        // otherwise alert just master

}

/*	void irq1()
	function: specifically handles the keyboard interrupt
	takes in scancode from the keyboard and writes it to terminal
	input: nothing!
	returns: nothing!
*/
void irq1(){
	unsigned char scancode = inb(KYBD_ADDR);

    terminal_key_pressed(scancode);

}

/*	void irq0()
	function: specifically handles the PIT interrupt for scheduling
	input: nothing!
	returns: nothing!
*/
void irq0(){
	switch_task();
}

/*	void irq8()
	function: specifically handles the RTC interrupt for user-program timing
	input: nothing!
	returns: nothing!
*/
void irq8(){
    rtc_interrupt();
}

// general form of irq handler taken from Bran's kernel tutorial
/* Each of the IRQ ISRs point to this function, rather than
*  the specific irqX() functions. The IRQ Controllers need
*  to be told when you are done servicing them, so you need
*  to send them an "End of Interrupt" command (0x20). There
*  are two 8259 chips: The first exists at 0x20, the second
*  exists at 0xA0. If the second controller (an IRQ from 8 to
*  15) gets an interrupt, you need to acknowledge the
*  interrupt at BOTH controllers, otherwise, you only send
*  an EOI command to the first controller. If you don't send
*  an EOI, you won't raise any more IRQs */

/*	void irq_handler(uint32_t irq_num)
	function: acts as the generic interrupt handler. wraps each irq and routes
	to the appropriate specific irq handler. Also sends an EOI to the i8259 for
	that num
	input: int irq_num, the number of the irq
	returns: nothing!
*/
void irq_handler(uint32_t irq_num)
{
    // This is a blank function pointer
    void (*handler)();

    // Find out if we have a custom handler to run for this
    //  IRQ, and then finally, run it
    handler = irq_functions[irq_num];
    if (handler)        // if handler defined
    {
        handler();      // run it
    }

    send_eoi(irq_num);  // send dat sweet end-of-interrupt
}

/*	void init_PIT(int hertz)
	function: initializes PIT to a given frequency (for scheduling)
	input: int hertz, the frequency desired. Can range from 1 to 1193180
	returns: nothing!
*/
void init_PIT(int hertz)
{
  if(hertz > 0 && hertz <= MAXPITFREQ){	// bound check
	  uint8_t lower, upper;
	  uint32_t divisor =  MAXPITFREQ/hertz; // ~1.19 MHz freq of PIT clock / 100Hz = divisor

	  outb(0x36,0x43);  // send command byte 0x36 to PIT at 0x43

	   // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
	   lower = (uint8_t)(divisor & 0xFF);
	   upper = (uint8_t)( (divisor>>8) & 0xFF );

	   // Send the frequency divisor.
	   outb(lower, 0x40);   // 0x40 is the command address of the PIT
	   outb(upper, 0x40);
   }
}
