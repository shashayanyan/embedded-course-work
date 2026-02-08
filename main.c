#include "main.h"
#include "uart.h"
#include "console.h"


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

/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the startup.s file.
 */
void _start() {
  console_init(line_handler);
  cursor_hide();

  char cursor_chars[] = {'|', '/', '-', '\\'};
  int cursor_idx = 0;
  uint8_t cursor_color = RED;
  long long counter = 0;

  while (1) {
    uint8_t c;
    if (uart_receive(UART0, &c) == 1) {
        // Erase the old cursor 
        int r, col;
        cursor_position(&r, &col);
        cursor_at(r, col);
        kprintf(" ");
        cursor_at(r, col);

        console_echo(c);
    }

    counter++;
    if (counter > 2000000) { // 500ms?
      counter = 0;

      int r, col;
      cursor_position(&r, &col);

      // Draw the new cursor
      cursor_at(r, col);
      console_color(cursor_color);
      kprintf("%c", cursor_chars[cursor_idx]);
      
      // Restore cursor position and color
      cursor_at(r, col);
      console_color(COLOR_RESET);

      // Update for next 
      cursor_idx = (cursor_idx + 1) % 4;
      cursor_color = (cursor_color == RED) ? WHITE : RED;
    }
  }
}



