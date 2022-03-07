#ifndef CPU_INIT_H
#define CPU_INIT_H

#include "stivale2.h"

void startup_aps(struct stivale2_struct_tag_smp *smp_info);
void ap_entry();

#endif
