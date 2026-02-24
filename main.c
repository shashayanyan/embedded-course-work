#include "main.h"
#include "uart.h"
#include "console.h"
#include "event.h"
#include "isr.h"
#include "timer.h"
#include "stream.h"

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

// for command matching
static int starts_with(const char *prefix, const char *str){
  while(*prefix){
    if (*prefix != *str){
      return 0; //missmatch :/
    }
    prefix++;
    str++;
  }
  return 1; //matched
}

void line_handler(char* str) {
  if (starts_with("echo ", str)) {
    console_puts(&str[5]);
    console_puts("\r\n"); // Make sure to use \r\n, not just \n
  } 
  else if (starts_with("davinci ", str)) {
    char* payload = &str[8];
    int len = 0;
    while(payload[len] != '\0') len++;
    int cursor_col; int cursor_row;
    cursor_position(&cursor_row, &cursor_col);
    //console_puts(" -> ");
    for (int i = len - 1; i >= 0; i--) {
        // stream_write or console_echo a single char
        uint8_t c = payload[i];
        stream_write(0, &c, 1);
        cursor_col++; // Keep cursor in sync!
    }
    cursor_at(cursor_row, cursor_col);
    console_puts("\r\n");
  } 
  else if (str[0] == '\0') {
    // Empty enter, just print the prompt again
  } 
  else {
    console_puts("[ERROR] Unknown Command...\r\n");
    console_puts(str);
    console_puts("\r\n");
  }

  // Print the prompt for the next line
  console_puts("simple-shell>$ ");
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
    /*kprintf("%c", cursor_chars[cursor_idx]);*/
    //uart_send(UART0, cursor_chars[cursor_idx]);
    console_putc(cursor_chars[cursor_idx]);
    
    // Restore cursor position and color for user typing
    cursor_at(r, col);
    console_color(COLOR_RESET);

    // Update next frame
    cursor_idx = (cursor_idx + 1) % 4;
    cursor_color = (cursor_color == RED) ? WHITE : RED;

    // repost the event for the next frame
    event_post(animate_cursor_reaction, NULL, 500); // ~500ms??
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

// simple polling reaction to "read"
void on_uart_char(void* cookie) {
    uint8_t c;
    // We know data is ready because we got an interrupt!
    while (stream_read(0, &c, 1) == 1) { // stream replacing uart_recieve
        console_echo(c);
    }
    // NO event_post here! We wait for the next interrupt.
}


/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the startup.s file.
 */
void _start() {
  // 1. Initialize Software Queues & Data Structures
  event_init();
  stream_init();
  //cursor_hide();

  // 2. Initialize Core Interrupts & VIC
  irqs_setup();
  irq_init();
  // 3. Initialize Hardware Devices
  timer_init();
// 4. Connect Hardware Interrupts to the Stream API
  uart_enable_interrupt();
  stream_set_read_listener(0, on_uart_char, NULL);

  // 5. Now the console
  console_init(line_handler);

// 6. Enable the CPU to start catching interrupts
  irqs_enable();
  

  // post initial events
  //event_post(poll_uart_reaction, NULL, 1);
  // 7. Start background tasks and enter the Event Pump
  //event_post(animate_cursor_reaction, NULL, 500); 
  // animate_cursor removed, I'm confident now that the timer and interrupts work...

  kprintf("[DEBUG] Hello Youssef From debug console!!!\n");
  // start the scheduler.
  event_loop();
}



