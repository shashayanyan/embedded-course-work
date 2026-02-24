#include "stream.h"
#include "ring.h"
#include "event.h"

// Define the extern API 
extern void uart0_unmask_tx_interrupt(void);

ring_t rx_ring;
ring_t tx_ring;

static void (*rx_listener)(void*) = NULL;
static void* rx_cookie = NULL;

static void (*tx_listener)(void*) = NULL;
static void* tx_cookie = NULL;

void stream_init(void) {
    ring_init(&rx_ring);
    ring_init(&tx_ring);
}

void stream_set_read_listener(int stream, void (*listener)(void*), void* cookie) {
    rx_listener = listener;
    rx_cookie = cookie;
}

void stream_set_write_listener(int stream, void (*listener)(void*), void* cookie) {
    tx_listener = listener;
    tx_cookie = cookie;
}

int stream_read(int stream, uint8_t* buffer, size_t length) {
    int count = 0;
    while (count < length && !ring_empty(&rx_ring)) {
        buffer[count++] = ring_get(&rx_ring);
    }
    return count;
}

int stream_write(int stream, uint8_t* buffer, size_t length) {
    int count = 0;
    while (count < length && !ring_full(&tx_ring)) {
        ring_put(&tx_ring, buffer[count++]);
    }
    
    // If we put data in the TX ring, wake up the UART to start sending 
    if (count > 0) {
        uart0_unmask_tx_interrupt();
    }
    return count;
}

// --- called ONLY by the UART ISR ---

void stream_rx_put_from_isr(uint8_t code) {
    if (!ring_full(&rx_ring)) {
        ring_put(&rx_ring, code);
    }
    // Post the read event so the application wakes up to process the byte!
    if (rx_listener) {
        event_post(rx_listener, rx_cookie, 0); 
    }
}

int stream_tx_empty_from_isr(void) {
    return ring_empty(&tx_ring);
}

uint8_t stream_tx_get_from_isr(void) {
    return ring_get(&tx_ring);
}

void stream_fire_write_listener_from_isr(void) {
    if (tx_listener) {
        event_post(tx_listener, tx_cookie, 0);
    }
}