// slide 15
#ifndef _RING_H_
#define _RING_H_

#include <stdint.h>

#define MAX_CHARS 512

typedef struct {
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint8_t buffer[MAX_CHARS];
} ring_t;

void ring_init(ring_t* ring);
int ring_empty(ring_t* ring);
int ring_full(ring_t* ring);
void ring_put(ring_t* ring, uint8_t code);
uint8_t ring_get(ring_t* ring);

#endif /* _RING_H_ */