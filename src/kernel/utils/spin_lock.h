#ifndef SPIN_LOCK_H
#define SPIN_LOCK_H

#include <stdbool.h>

typedef bool spin_lock_t;

void 
wait(spin_lock_t *spin_lock);

void 
release(spin_lock_t *spin_lock);

#endif
