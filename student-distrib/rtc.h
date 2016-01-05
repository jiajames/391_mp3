#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "lib.h"
#include "i8259.h"

void rtc_interrupt();
int32_t rtc_write(int32_t fd,const void* buf, int32_t freq);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);

#endif
