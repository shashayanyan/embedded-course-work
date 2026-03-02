#include "history.h"
#include "event.h"
#include <stddef.h>

#define MAX_HISTORY 10
#define MAX_LINE 80

static char lines[MAX_HISTORY][MAX_LINE];
static int head = 0;
static int count = 0;
static int view_offset = 0; // How far back we are currently viewing

// State variables for the async request
static int req_dir = 0;
static void (*req_cb)(const char*) = NULL;

void history_init(void) {
    head = 0;
    count = 0;
    view_offset = 0;
}

// string copy
static void string_copy(char* dest, const char* src) {
    while(*src) { *dest++ = *src++; }
    *dest = '\0';
}

void history_add(const char* line) {
    if (line[0] == '\0') return; // Ignore empty enters
    
    string_copy(lines[head], line);
    head = (head + 1) % MAX_HISTORY;
    if (count < MAX_HISTORY) count++;
    
    view_offset = 0; // Reset view when a new command is entered
}

// This runs from the event looop, NOT directly from the interrupt
static void history_reaction(void* cookie) {
    if (!req_cb) return;

    if (req_dir == -1) { // UP Arrow (Older)
        if (view_offset < count) view_offset++;
    } else if (req_dir == 1) { // DOWN Arrow (Newer)
        if (view_offset > 0) view_offset--;
    }

    if (view_offset == 0) {
        req_cb(""); // Back to the current empty line
    } else {
        int idx = (head - view_offset + MAX_HISTORY) % MAX_HISTORY;
        req_cb(lines[idx]);
    }
}

void history_request(int dir, void (*callback)(const char*)) {
    req_dir = dir;
    req_cb = callback;
    // Post the work to the event loop 
    event_post(history_reaction, NULL, 0); 
}