#ifndef _EVENT_H_
#define _EVENT_H_

#include <stdint.h>

/**
 * The function defines a single event.
 * 
 * cookie is a pointer to context data for the reaction function.
 * react is a function pointer to the event handler (the "reaction").
 * eta is the Estimated Time of Arrival for the event, in system ticks.
 */
struct event {
    void* cookie;
    void (*react)(void* cookie);
    uint64_t eta;
};

/**
 * Initialize the event scheduler.
 */
void event_init(void);

/**
 * Post a new event to the event queue.
 * 
 * react is the reaction function to call when the event fires.
 * cookie is a context pointer to pass to the reaction.
 * delay is the delay from now (in ticks?) when the event should fire.
 */
void event_post(void (*react)(void*), void* cookie, uint32_t delay);

/**
 * Start the main event loop. function never returns.
 */
void event_loop(void);

/**
 * Gets the current system time in ticks.
 * return the current time (mocked?).
 */
uint64_t time_now(void);

#endif /* _EVENT_H_ */
