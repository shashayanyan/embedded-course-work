#include "ring.h"

void ring_init(ring_t* ring) {
    ring->head = 0;
    ring->tail = 0;
}

int ring_empty(ring_t* ring) {
    return (ring->head == ring->tail);
}

int ring_full(ring_t* ring) {
    uint32_t next = (ring->head + 1) % MAX_CHARS;
    return (next == ring->tail);
}

void ring_put(ring_t* ring, uint8_t code) {
    uint32_t next = (ring->head + 1) % MAX_CHARS;
    ring->buffer[ring->head] = code;
    ring->head = next;
}

uint8_t ring_get(ring_t* ring) {
    uint8_t bits;
    uint32_t next = (ring->tail + 1) % MAX_CHARS;
    bits = ring->buffer[ring->tail];
    ring->tail = next;
    return bits;
}