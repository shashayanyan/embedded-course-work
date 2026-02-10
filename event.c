#include "event.h"
#include "main.h" // For NULL
#include <stddef.h>

#define MAX_EVENTS 32

// not a sorted list yet, maybe change later?
static struct event event_queue[MAX_EVENTS];
static int num_events = 0;
static uint64_t ticks = 0;

uint64_t time_now(void) {
    // For now a mocking ticker of a logical clock ...
    return ticks++;
}

void event_init(void) {
    for (int i = 0; i < MAX_EVENTS; i++) {
        event_queue[i].react = NULL;
    }
    num_events = 0;
}

void event_post(void (*react)(void*), void* cookie, uint32_t delay) {
    if (num_events >= MAX_EVENTS) {
        // what do I do here?
        return;
    }

    // Find an empty slot
    int i = 0;
    while(i < MAX_EVENTS && event_queue[i].react != NULL) {
        i++;
    }

    if (i < MAX_EVENTS) {
        event_queue[i].eta = time_now() + delay;
        event_queue[i].cookie = cookie;
        event_queue[i].react = react;
        num_events++;
    }
}

void event_loop(void) {
    for (;;) {
        uint64_t now = time_now();
        int best_event_idx = -1;
        uint64_t min_eta = UINT64_MAX;

        // Find the next event that is ready to run
        for (int i = 0; i < MAX_EVENTS; i++) {
            if (event_queue[i].react != NULL && event_queue[i].eta < min_eta) {
                min_eta = event_queue[i].eta;
                best_event_idx = i;
            }
        }

        if (best_event_idx != -1 && event_queue[best_event_idx].eta <= now) {
            // Found an event to run!
            struct event evt = event_queue[best_event_idx];
            
            // mark as empty before running
            event_queue[best_event_idx].react = NULL;
            num_events--;

            evt.react(evt.cookie);

        } else {
            // No events ready, wait for interrupt? what else can I do here?
            asm volatile("wfi");
        }
    }
}
