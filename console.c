#include "console.h"
#include "main.h"
#include "uart.h"
#include <stdint.h>

// cursor position
static int cursor_row;
static int cursor_col;

// line callback
static void (*line_callback)(char*);

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
  kprintf("%c[%d;%dH", 27, row + 1, col + 1);
}

void cursor_position(int* row, int* col) {
  *row = cursor_row;
  *col = cursor_col;
}

void cursor_hide() {
  kprintf("%c[?25l", 27);
}

void cursor_show() {
  kprintf("%c[?25h", 27);
}

void console_color(uint8_t color) {
  kprintf("%c[%dm", 27, color);
}

void console_clear() {
  kprintf("%c[H%c[2J", 27, 27);
  cursor_row = 0;
  cursor_col = 0;
}

void console_init(void (*callback)(char*)) {
  console_clear();
  line_callback = callback;
}

// line buffer
#define LINE_LEN 80
static char line_buffer[LINE_LEN];
static int line_pos;

static enum {
  NORMAL,
  ESCAPE,
  ESCAPE_BRACKET
} echo_state = NORMAL;

void console_echo(uint8_t byte) {
  switch (echo_state) {
    case NORMAL:
      if (byte >= 32 && byte <= 126) { // printable ASCII
        if (line_pos < LINE_LEN - 1) {
          kprintf("%c", byte);
          line_buffer[line_pos++] = byte;
          cursor_col++;
        }
      } else if (byte == 8 || byte == 127) { // backspace
        if (line_pos > 0) {
          line_pos--;
          cursor_left();
          kprintf(" ");
          //cursor_left();
        }
      } else if (byte == '\n' || byte == '\r') { // enter
        line_buffer[line_pos] = '\0';
        
        if (line_callback) {
          int saved_row, saved_col;
          cursor_position(&saved_row, &saved_col);
          line_callback(line_buffer);
          cursor_at(saved_row, saved_col);
        }

        kprintf("\n");
        cursor_row++;
        cursor_col = 0;
        line_pos = 0;
      } else if (byte == 3) { // Ctrl-C
        kprintf("^C\n");
        cursor_row++;
        cursor_col = 0;
        line_pos = 0;
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
          cursor_up();
          break;
        case 'B': // down
          cursor_down();
          break;
        case 'C': // right
          cursor_right();
          break;
        case 'D': // left
          cursor_left();
          break;
      }
      echo_state = NORMAL;
      break;
  }
}
