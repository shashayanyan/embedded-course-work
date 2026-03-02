#include "console.h"
#include "main.h"
#include "uart.h"
#include <stdint.h>
#include "stream.h"
#include "history.h"
// cursor position
static int cursor_row;
static int cursor_col;
static int cursor_visible = 0;

// line callback
static void (*line_callback)(char*);

// line buffer
#define LINE_LEN 80
static char line_buffer[LINE_LEN];
static int line_pos;

static enum {
  NORMAL,
  ESCAPE,
  ESCAPE_BRACKET
} echo_state = NORMAL;


// Helper to write a single char to the stream
void console_putc(uint8_t c) {
    stream_write(0, &c, 1);
}

// Helper to print a string to the stream
void console_puts(const char* str) {
  int len = 0;
  while (str[len] != '\0') {
      if (str[len] == '\n') {
          cursor_row++;
          cursor_col = 0;
      } else if (str[len] != '\r') {
          cursor_col++;
      }
      len++;
  }
  // Write the whole string to the ring buffer at once
  stream_write(0, (uint8_t*)str, len);
}

// Helper to print an integer to the stream
void console_put_int(int num) {
    char buf[10];
    int i = 0;
    if (num == 0) {
        console_putc('0');
        return;
    }
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (i > 0) {
        console_putc(buf[--i]);
    }
}

static void ansi_cmd(const char* cmd) { // helper for commands to avoid confusion
    int len = 0;
    while(cmd[len]) len++;
    stream_write(0, (uint8_t*)cmd, len);
}

void console_erase_cursor(void) {
    if (!cursor_visible) return;
    char c = line_buffer[line_pos];
    if (c == '\0') c = ' ';
    
    ansi_cmd("\033[0m"); // Force Normal Video
    stream_write(0, (uint8_t*)&c, 1);
    ansi_cmd("\033[1D"); // Step back to original position
    cursor_visible = 0;
}


void cursor_left() {
  if (cursor_col > 0) {
    cursor_col--;
    cursor_at(cursor_row, cursor_col);
  }
}

void cursor_right() {
  if (cursor_col < NCOLS - 1) {
    cursor_col++;
    cursor_at(cursor_row, cursor_col);
  }
}

void cursor_down() {
  if (cursor_row < NROWS - 1) {
    cursor_row++;
    cursor_at(cursor_row, cursor_col);
  }
}

void cursor_up() {
  if (cursor_row > 0) {
    cursor_row--;
    cursor_at(cursor_row, cursor_col);
  }
}

void cursor_at(int row, int col) {
  cursor_row = row;
  cursor_col = col;
  /*kprintf("%c[%d;%dH", 27, row + 1, col + 1);*/
  console_putc(27); // ESC
  console_putc('[');
  console_put_int(row + 1);
  console_putc(';');
  console_put_int(col + 1);
  console_putc('H');
}

void cursor_position(int* row, int* col) {
  *row = cursor_row;
  *col = cursor_col;
}

void cursor_hide() {
  /*kprintf("%c[?25l", 27);*/console_puts("\033[?25l");
}

void cursor_show() {
  /*kprintf("%c[?25h", 27);*/console_puts("\033[?25h");
}

void console_color(uint8_t color) {
  /*kprintf("%c[%dm", 27, color);*/
  console_putc(27);
  console_putc('[');
  console_put_int(color);
  console_putc('m');
}

void console_clear() {
  ansi_cmd("\033[2;1H\033[0J");  
  cursor_row = 1;
  cursor_col = 0;
  console_puts("List of commands:\n- echo <str> :: repeats the <str>\n- davinci <str> :: applies da vinci code to <str> and prints it\n- PRESS C-c to clear console.\n- PRESS C-a c to stop the console\n");
  console_puts("simple-shell>$ ");
}

void console_init(void (*callback)(char*)) {
  console_clear();
  line_callback = callback;
}


static void clear_line_buffer(void) {
    for (int i = 0; i < 80; i++) { 
        line_buffer[i] = '\0';
    }
    line_pos = 0;
}

void console_echo(uint8_t byte) {
  switch (echo_state) {
    case NORMAL:
      ansi_cmd("\033[0m");
      console_erase_cursor();
      if (byte >= 32 && byte <= 126) { // printable ASCII
        // 1. Find the end of the string
        int end = line_pos;
        while (line_buffer[end] != '\0') end++;
        // 2. Shift memory RIGHT to make room 
        if (end < 79) {
            for (int i = end; i >= line_pos; i--) {
                line_buffer[i + 1] = line_buffer[i]; 
            }
            line_buffer[line_pos] = byte;
            
            // 3. Visually draw the new character AND the shifted rest-of-line
            int chars_drawn = 0;
            int draw_idx = line_pos;
            while (line_buffer[draw_idx] != '\0') {
                stream_write(0, (uint8_t*)&line_buffer[draw_idx], 1);
                draw_idx++;
                chars_drawn++;
            }
            
            // 4. Update trackers
            line_pos++;
            cursor_col++;
            
            // 5. Move physical cursor back to the correct spot
            // drew hars_drawn characters, but only advanced 1 space
            for (int d = 0; d < chars_drawn - 1; d++) {
                ansi_cmd("\033[1D");
            }
        }

      } else if (byte == 8 || byte == 127) { // backspace
        // === backspace in middle of typed characters is dead ===
        // needs to be fixed : IT'S FIXED NOW!!
        if (line_pos > 0) {
            // 1. Shift memory LEFT to close the gap
            int i = line_pos;
            while (line_buffer[i] != '\0') {
                line_buffer[i - 1] = line_buffer[i];
                i++;
            }
            line_buffer[i - 1] = '\0'; 
            
            // 2. Update trackers
            line_pos--;
            cursor_left();
            
            // 3. draw the shifted rest ofline
            int chars_drawn = 0;
            int draw_idx = line_pos;
            while (line_buffer[draw_idx] != '\0') {
                stream_write(0, (uint8_t*)&line_buffer[draw_idx], 1);
                draw_idx++;
                chars_drawn++;
            }
            
            // 4. Print a space at the end to erase the trailing duplicate character
            stream_write(0, (uint8_t*)" ", 1);
            chars_drawn++;
            
            // 5. Move physical cursor all the way back
            for (int d = 0; d < chars_drawn; d++) {
                ansi_cmd("\033[1D");
            }
        }
      } else if (byte == '\n' || byte == '\r') { // enter
        //line_buffer[line_pos] = '\0';
        
        // 1. Move the terminal to a new line visually BEFORE the callback
        console_puts("\r\n");
        line_pos = 0;

        // 2. Run the command (The callback will print the output and the new prompt)
        if (line_callback) {
          line_callback(line_buffer);
        }

        // 3. Empty buffer
        clear_line_buffer();
      } else if (byte == 3) { // Ctrl-C
        cursor_col = 16;
        line_pos = 0;
        clear_line_buffer();
        console_clear();
      } else if (byte == 27) {
        echo_state = ESCAPE;
      }
      // a C-style comment
      // All other control characters are ignored
      break;
    case ESCAPE:
      if (byte == '[') {
        echo_state = ESCAPE_BRACKET;
      } else {
        echo_state = NORMAL;
      }
      break;
    case ESCAPE_BRACKET:
      switch (byte) {
        case 'A': // up
          history_request(-1, console_replace_line);          
          break;
        case 'B': // down
          history_request(1, console_replace_line);
          break;
        case 'C': // right
          // Only move right if there is actually text ahead
            if (line_buffer[line_pos] != '\0') {
                line_pos++;
                cursor_right(); 
            }
          break;
        case 'D': // left
          if (line_pos > 0) {
                line_pos--;
                cursor_left(); 
            }
          break;
      }
      echo_state = NORMAL;
      break;
  }
}

void console_draw_status_bar(uint32_t uptime_sec, uint32_t cpu, uint32_t events) {
    int saved_r = cursor_row;
    int saved_c = cursor_col;
    cursor_at(0, 0); 
    
    // white background
    console_color(7);
    
    // content
    console_puts(" OS Uptime: ");
    console_put_int(uptime_sec);
    console_puts("s | CPU Usage: ");
    console_put_int(cpu);
    console_puts("% | Events/sec: ");
    console_put_int(events);
    console_puts("          "); // Extra space

    console_puts("\033[K");
    
    // resets colors back to normal
    console_color(0);
    
    // Restore cursor 
    cursor_at(saved_r, saved_c); 
}

void console_replace_line(const char* new_line) {
    // 1. Visually erase the current line on the screen
    while (line_pos > 0) {
        cursor_left();
        console_putc(' ');
        cursor_col++;
        cursor_left();
        line_pos--;
    }
    
    // 2. Copy the new line into our buffer and print it
    int i = 0;
    while (new_line[i] != '\0' && i < 79) {
        line_buffer[i] = new_line[i];
        console_putc(new_line[i]);
        cursor_col++;
        i++;
    }
    line_buffer[i] = '\0';
    line_pos = i;
}

void console_blink_cursor(void) {
    char c = line_buffer[line_pos];
    if (c == '\0') c = ' '; // If at end of line, highlight empty space
    
    cursor_visible = !cursor_visible;
    
    if (cursor_visible) {
        ansi_cmd("\033[7m"); // Reverse video ON
    } else {
        ansi_cmd("\033[0m"); // Reverse video OFF
    }
    
    stream_write(0, (uint8_t*)&c, 1);
    ansi_cmd("\033[0m"); 
    ansi_cmd("\033[1D"); 
}