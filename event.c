#include "event.h"
#include "main.h" // For NULL
#include "timer.h"
#include <stddef.h>

#define MAX_EVENTS 32

extern void irqs_disable(void);
extern void irqs_enable(void);
extern void wfi(void);

// not a sorted list yet, maybe change later?
static struct event event_queue[MAX_EVENTS];
static int num_events = 0;

// for status bar
uint32_t idle_time_ms = 0;
uint32_t events_processed = 0;

// for the status bar
void get_and_reset_stats(uint32_t* cpu_usage, uint32_t* events) {
    // CPU usage = 100% - idle%
    if (idle_time_ms > 1000) idle_time_ms = 1000;
    *cpu_usage = 100 - (idle_time_ms / 10);
    *events = events_processed;
    
    idle_time_ms = 0;
    events_processed = 0;
}

uint64_t time_now(void) {
    // returning the real hardware-driven milliseconds
    return system_ticks;
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
        // --- 1. SEARCH FOR READY EVENTS ---
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
        // --- 2. DO THE WORK (if any) ---
        if (best_event_idx != -1 && event_queue[best_event_idx].eta <= now) {
            // Found an event to run!
            struct event evt = event_queue[best_event_idx];
            
            // mark as empty before running
            event_queue[best_event_idx].react = NULL;
            num_events--;
            events_processed++;
            evt.react(evt.cookie);

        } 
        // --- 3. NO WORK? SLEEP SECURELY --- (based on the emails...)
        else {
            // Disable interrupts to close the race condition window
            irqs_disable();
            
            // Check the queue ONE MORE TIME. 
            // An interrupt might have fired and posted an event
            // right before we disabled the interrupts...
            now = time_now();
            best_event_idx = -1;
            min_eta = UINT64_MAX;
            for (int i = 0; i < MAX_EVENTS; i++) {
                if (event_queue[i].react != NULL && event_queue[i].eta < min_eta) {
                    min_eta = event_queue[i].eta;
                    best_event_idx = i;
                }
            }

            if (best_event_idx == -1 || event_queue[best_event_idx].eta > now) {
                uint64_t sleep_start = time_now();
                // Still no work. It is safe to sleep.
                wfi();           // sleep
                irqs_enable();   // Re-enable interrupts
                idle_time_ms += (time_now() - sleep_start);
            } else {
                // Work snuck in so don't sleep, just re-enable interrupts and loop around...
                irqs_enable();
            }
        }
    }
}
