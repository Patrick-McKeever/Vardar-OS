#ifdef SCHED_H
#define SCHED_H

#include "proc/proc.h"
#include <stdint.h>

void
global_init_scheduler(uint8_t num_cpus);

void
local_init_scheduler();

void
schedule_task(pcb_t *pcb);

void
unschedule_task(pcb_t *pcb);

void
exit_current_task();


void
context_switch();

#endif

