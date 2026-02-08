#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <stdint.h>
/*
 * Terminal Size: 
 *   - rows are horizontal
 *   - columns are vertical
 * 
 * *Nota Bene:* you must choose values for the ncols and nrows
 * that match your Linux terminal window size. You can normally
 * see that in the window's menus. A typical size is 80x24 or
 * 80x43 in the (ncols,nrows) format.
 * 
 * Important: some terminal emulation emulate a wrap-around policy 
 * at the end of a line rather than blocking the cursor. 
 * If it is the case, we suggest that you use a smaller
 * ncols, like 79 instead of 80, to avoid this wrapping-around policy.
 */
#define NCOLS 80
#define NROWS 24

// the reset color resets both the ink and background colors.
#define COLOR_RESET 0

// the following colors are for the ink (foreground)
#define BLACK 30
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define MAGENTA 35
#define CYAN 36
#define WHITE 37

// the following colors are for the background
#define BG_BLACK (BLACK+10)
#define BG_RED (RED+10)
#define BG_GREEN (GREEN+10)
#define BG_YELLOW (YELLOW+10)
#define BG_BLUE (BLUE+10)
#define BG_MAGENTA (MAGENTA+10)
#define BG_CYAN (CYAN+10)
#define BG_WHITE (WHITE+10)

/*
 * Functions to move the cursor from its current position
 */
void cursor_left();
void cursor_right();
void cursor_down();
void cursor_up();

/*
 * Function to move the cursor to the given coordinates
 */ 
void cursor_at(int row, int col);

/*
 * Functions to obtain the current cursor position 
 */
void cursor_position(int* row, int* col);

/* 
 * Functions to hide/show the terminal cursor
 */
void cursor_hide();
void cursor_show();

/*
 * Function to set the color, either for the ink or background
 */
void console_color(uint8_t color);

/*
 * Clears the terminal, like the bash command `clear`.
 * Positions the cursor at (0,0).
 */
void console_clear();

/*
 * Initializes the console, giving the callback
 * to call for each line entered on the keyboard.
 * A line is a C string but contains only ASCII 
 * characters ([32-126]), as a C string it is 
 * terminated by a '\0'.
 * A line is validated by the end user by hitting 
 * the key `Enter`.
 */
void console_init(void (*callback)(char*));

/*
 * Call this function with every byte read from the "keyboard".
 * Echoes to the terminal only ASCII characters ([32-126]).
 * Recognized special characters:
 *   - arrow keys (left,right,up,down)
 *   - delete key
 *   - backspace (code 127 or 8)
 *   - ctrl-c to clear the terminal
 */
void console_echo(uint8_t byte);

#endif /* _CONSOLE_H_ */