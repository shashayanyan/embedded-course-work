#include "main.h"
#include "uart.h"
#include "console.h"
#include "event.h"


/*
 * Define ECHO_ZZZ to have a periodic reminder that this code is polling
 * the UART, actively. This means the processor is running continuously.
 * Polling is of course not the way to go, the processor should halt in
 * a low-power state and wake-up only to handle an interrupt from the UART.
 * But this would require setting up interrupts...
 */
#define ECHO_ZZZ

extern uint32_t stack_top;

void panic() {
  while (1)
	  ;
}

// faire une boucle de 1sec
void wait(){
	for (int i=0; i<1000000; i++){
	}
}

void check_memory() {
  void *max = (void*)MEMORY;
  void *addr = &stack_top;
  if (addr >= max)
    panic();
}

void line_handler(char* str) {
  int len = 0;
  while(str[len] != '\0') {
    len++;
  }

  kprintf(" -> ");
  for (int i = len - 1; i >= 0; i--) {
    kprintf("%c", str[i]);
  }
}

// Reaction for the cursor
void animate_cursor_reaction(void* cookie) {
    static char cursor_chars[] = {'|', '/', '-', '\\'};
    static int cursor_idx = 0;
    static uint8_t cursor_color = RED;

    int r, col;
    cursor_position(&r, &col);

    // draw new cursor
    cursor_at(r, col);
    console_color(cursor_color);
    kprintf("%c", cursor_chars[cursor_idx]);
    
    // Restore cursor position and color for user typing
    cursor_at(r, col);
    console_color(COLOR_RESET);

    // Update next frame
    cursor_idx = (cursor_idx + 1) % 4;
    cursor_color = (cursor_color == RED) ? WHITE : RED;

    // repost the event for the next frame
    event_post(animate_cursor_reaction, NULL, 500000); // ~500ms??
}

// Reaction for polling UART
void poll_uart_reaction(void* cookie) {
    uint8_t c;
    if (uart_receive(UART0, &c) == 1) {
        // Erase the old cursor before processing the character
        int r, col;
        cursor_position(&r, &col);
        cursor_at(r, col);
        kprintf(" ");
        cursor_at(r, col);

        console_echo(c);
    }
    // repost the event to continue polling
    event_post(poll_uart_reaction, NULL, 1);
}


/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the startup.s file.
 */
void _start() {
  console_init(line_handler);
  event_init();
  cursor_hide();

  // post initial events
  event_post(poll_uart_reaction, NULL, 1);
  event_post(animate_cursor_reaction, NULL, 1);

  // start the scheduler.
  event_loop();
}



