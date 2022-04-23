#include "utils/spin_lock.h"

void 
wait(spin_lock_t *spin_lock)
{
	while(*spin_lock);
	*spin_lock = true;
}

void 
release(spin_lock_t *spin_lock)
{
	*spin_lock = false;
}

