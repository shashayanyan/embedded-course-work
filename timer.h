#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

extern volatile uint64_t system_ticks;
extern volatile uint32_t system_seconds;
void timer_init(void);

#endif 