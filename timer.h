#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

extern volatile uint64_t system_ticks;

void timer_init(void);

#endif 