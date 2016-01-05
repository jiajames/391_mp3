#include "rtc.h"

volatile int rtc_interrupt_flag = 0;

/*	void rtc_open(void)
	function: initializes the RTC to 2Hz
	input: uint8_t* filename (unused)
	returns: int32_t the result of rtc_write() setting the rtc to 2Hz
*/

int32_t rtc_open(const uint8_t* filename){

	return rtc_write(0,NULL,2);	// init RTC to 2Hz
}

/*	void rtc_close(int32_t fd)
	function: closes the RTC
	input: int32_t fd (unused file descriptor)
	returns: 0, success
*/

int32_t rtc_close(int32_t fd){
	return 0;
}

/*	void rtc_read(int32_t fd, void* buf, int32_t nbytes)
	function: waits for the next RT interrupt before returning
	input: (all unused)
	returns: 0 once the RTC interrupts again
*/

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
	while(!rtc_interrupt_flag); // wait for rtc_interrupt() to be called from irq8() to change flag
	rtc_interrupt_flag = 0;
	return 0;
}

/*	void rtc_write(int32_t fd, const void* buf, int32_t freq)
	function: initializes the RTC to the power-of-two frequency provided
	input: int32_t fd (unused),
		const void* buf (unused),
		int32_t freq, the frequency to set the RTC to
	returns: 0 if the frequency is a power of 2
			-1 otherwise
*/

int32_t rtc_write(int32_t fd,const void* buf, int32_t freq){

	if(freq > 1024 || freq <= 0)	// common sense bound check
		return -1;

	unsigned int mask = 1;
	unsigned int divd = 16;	// the structure of our loop guarantees the divider will be 15 upon first check
	for(mask = 1; mask != 0; mask = mask << 1){
		divd--;
		if(mask == freq)	// check that freq is only a power of 2 (between 1 and 1024, inclusive)
			break;
	}

	if(mask != freq)
		return -1; // we failed to find a power of 2
	else{
		cli();

	    outb(SEL_REGA, RTC_ADDR);       // change freq
	    char prev = inb(CMOS_ADDR);
	    outb(SEL_REGA, RTC_ADDR);
	    prev &= MASK_L4;                   // clear lower 4 bits of register to wipe out old register bits
	    outb((prev | (divd-1)), CMOS_ADDR);   // place decimal 15 into register to set new rtc divider

	    outb(SEL_REGB, RTC_ADDR);       // turn on irq8
	    prev = inb(CMOS_ADDR);
	    outb(SEL_REGB, RTC_ADDR);
	    outb((prev|BIT7), CMOS_ADDR);   // OR previous regB value with BIT7 to enable rtc tick interrupt

	    sti();  // probably should omit this.... we call init_rtc() before enabling interrupts in kernel.c

	    return 0; // was originally return 0
	}
}

/*	void rtc_interrupt()
	function: handles a RTC interrupt by acknowledging reciept of interrupt,
		and setting interrupt flag to 1
	input: (all unused)
	returns: 0 once the RTC interrupts again
*/

void rtc_interrupt(){
	outb(SEL_REGC, RTC_ADDR);   // select register C
    inb(CMOS_ADDR);             // read from register C to reset rtc interrupt

    rtc_interrupt_flag = 1;		// a rtc interrupt has occurred
}
