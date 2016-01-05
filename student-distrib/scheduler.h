#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "terminal.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"

#include "pcb.h"
#include "idt.h"

void switch_task();
void remove_task_from_queue(terminal_t * task_rm);
void add_task_to_queue(terminal_t * task_add);

#endif
