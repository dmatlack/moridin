/** 
 * @file console.c 
 * @brief A console device driver implementation conforming to p1kern.h
 * 
 * @author Mark Wong Siang Kai (msiangka)
 * @author David Matlack
 *
 * @bug No known bugs
 */
#include "console.h"

#include <x86/video_defines.h>
#include <x86/asm.h>
#include <lib/stdint.h>

#define CELL_SIZE 2
#define HI 0
#define LO 1

static uint16_t get_offset(int row, int col);
static void *get_addr(int row, int col);
static void write_to_console(int row, int col, int part, char val);
static char get_from_console(int row, int col, int part);
static void update_fields(int row, int col, int color);

static void scroll_up();
static void update_cursor();
static void handle_new_line();
static void handle_backspace();
static void handle_default(char ch);
static int myputbyte(char c);

/* global variables and their default values */
static int cursor_row = 0;
static int cursor_col = 0;
static char term_color = FGND_GREEN | BGND_BLACK;

/**
 * @brief gets the offset of the cursor 
 *
 * This functions gets the offset of the cursor, given as row and col, from
 * CONSOLE_MEM_BASE, ie how many cells away from CONSOLE_MEM_BASE. Invariant:
 * 0 <= row < CONSOLE_HEIGHT; 0 <= col < CONSOLE_WIDTH
 *
 * @param row the current row of the cursor
 * @param col the current col of the cursor
 * @returns the offset of the cursor
 */
static uint16_t get_offset(int row, int col) {
  return row * CONSOLE_WIDTH + col;
}

/**
 * @brief translates the position of the cursor into video memory
 * 
 * This function converts the position of the cursor, given as row and col, into
 * video memory. Invariant: 0 <= row < CONSOLE_HEIGHT; 0 <= col < CONSOLE_WIDTH
 *
 * @param row the current row of the cursor
 * @param col the current col of the cursor
 * @returns the address of the video memory
 */
static void *get_addr(int row, int col) {
  //assert(row >= 0 && row < CONSOLE_HEIGHT && col >= 0 && col < CONSOLE_WIDTH);
  return (void *) (CONSOLE_MEM_BASE + CELL_SIZE * get_offset(row, col));
}

/**
 * @brief writes to the console the value given
 *
 * This function writes to the console the value given. Invariant: 0 <= row <
 * CONSOLE_HEIGHT; 0 <= col < CONSOLE_WIDTH
 *
 * @param row the current row of the cursor
 * @param col the current col of the cursor
 * @param part if HI writes to the first byte otherwise writes to the second
 * byte; part must be either HI or LO
 * @param val the value to write
 */
static void write_to_console(int row, int col, int part, char val) {
  //assert(part == HI || part == LO);
  *((char *) get_addr(row, col) + part) = val;
}

/**
 * @brief gets the value from the console
 *
 * This function gets the value at position row, col from the console.
 * Invariant: 0 <= row < CONSOLE_HEIGHT; 0 <= col < CONSOLE_WIDTH
 *
 * @param row the current row of the cursor
 * @param col the current col of the cursor
 * @param part if HI gets the first byte otherwise gets the second byte
 */
static char get_from_console(int row, int col, int part) {
  return *((char *) get_addr(row, col) + part);
}

/**
 * @brief updates the value of the cursor row, cursor col and terminal
 * foreground and background color
 *
 * This function updates the value of cursor_row, cursor_col and term_color
 *
 * @param new_row
 * @param new_col
 * @param new_color
 */
static void update_fields(int row, int col, int color) {
  cursor_row = row;
  cursor_col = col;
  term_color = color;
}

/**
 * @brief scrolls the screen up
 *
 * This function scrolls the console screen up by one row. The cursor_col is
 * reset to be at the start of the line. Assumes that cursor_row = 
 * CONSOLE_HEIGHT - 1
 */
static void scroll_up(void) {
  int row, col;

  //assert(cursor_row == CONSOLE_HEIGHT - 1);
  
  /* scroll upwards by shifting the contents one row up */
  for (row = 0; row < CONSOLE_HEIGHT - 1; row++)
    for (col = 0; col < CONSOLE_WIDTH; col++)
      draw_char(row, col, get_from_console(row + 1, col, HI),
                get_from_console(row + 1, col, LO));

  /* reset everything on the last line */
  for (col = 0; col < CONSOLE_WIDTH; col++)
    draw_char(CONSOLE_HEIGHT - 1, col, ' ', term_color);

  /* update the cursor position */
  cursor_col = 0;
}

/**
 * @brief updates the cursor by communicating with the CTRC
 *
 * This function communicates with the CTRC to update the position of the
 * cursor to the given row, col
 */
static void update_cursor(int row, int col) {
  uint16_t index;
  
  index = get_offset(row, col);
  outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
  outb(CRTC_DATA_REG, index & BYTE_0);
  outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
  outb(CRTC_DATA_REG, (index >> NUM_BITS_IN_BYTE) & BYTE_0);
}

/**
 * @brief puts a new line to the console screen
 *
 * This functions prints a newline at the current location. The cursor is moved
 * to the first column of the next line (scrolling if necessary)
 */
static void handle_new_line(void) {
  /* end of screen */
  if (cursor_row == CONSOLE_HEIGHT - 1)
    scroll_up();
  else {
    cursor_col = 0;
    cursor_row++;
  }
}

/**
 * @brief erases the previous character
 *
 * This function erases the previous character by writing a space over it and
 * moves the cursor back one column. If the backspace occurs at the beginning of
 * a line (ie, if cursor_col == 0), the cursor position is moved up one row, if
 * possible (ie, if cursor_row > 0), to the last column of the previous line
 */
static void handle_backspace(void) {
  /* beginning of line */
  if (cursor_col == 0) {
    /* move up */
    if (cursor_row > 0) {
      draw_char(cursor_row - 1, CONSOLE_WIDTH - 1, ' ', term_color);
      cursor_row--;
      cursor_col = CONSOLE_WIDTH - 1;
    }
  }
  else {
    draw_char(cursor_row, cursor_col - 1, ' ', term_color);
    cursor_col--;
  }
}

/**
 * @brief puts a character to the console screen
 *
 * This functions prints the character given to the current console location. If
 * the current location is the end of a line the cursor position is moved to the
 * next line, scrolling if necessary. Assumes the character is not a newline,
 * carriage return or backspace
 *
 * @param ch the character to print
 */
static void handle_default(char ch) {
  //assert(ch != '\n' && ch != '\b' && ch != '\r');
  
  draw_char(cursor_row, cursor_col, ch, term_color);
  cursor_col++;
  
  if (cursor_col == CONSOLE_WIDTH) {
    /* bottom right */
    if (cursor_row == CONSOLE_HEIGHT - 1)
      scroll_up();

    /* end of line */
    else {
      cursor_row++;
      cursor_col = 0;
    }
  }
}

/**
 * @brief helper function to print a character to the current location of the
 * cursor
 *
 * This is basically putbyte, but defined so that other functions can use it
 * without having to update the cursor everytime by communicating with CTRC
 *
 * @returns the input character
 */
static int myputbyte(char ch) {
  //assert(cursor_row < CONSOLE_HEIGHT && cursor_col < CONSOLE_WIDTH &&
         cursor_row >= 0 && cursor_col >= 0);

  switch (ch) {
    /* newline */
    case '\n':
      handle_new_line();
      break;

    /* carriage return */
    case '\r':
      cursor_col = 0;
      break;

    /* backspace */
    case '\b':
      handle_backspace();
      break;

    default:
      handle_default(ch);
      break;
  }

  return ch;
}

/**
 * @brief initializes the console
 * @return 0 on success, < 0 on error
 */
int console_init(void) {
  return CONSOLE_SUCCESS;
}
/** 
 * @brief Prints character ch at the current location
 *        of the cursor.
 *
 * If the character is a newline ('\n'), the cursor is
 * be moved to the beginning of the next line (scrolling if necessary).  If
 * the character is a carriage return ('\r'), the cursor
 * is immediately reset to the beginning of the current
 * line, causing any future output to overwrite any existing
 * output on the line.  If backsapce ('\b') is encountered,
 * the previous character is erased.  See the main console.c description
 * for more backspace behavior.
 *
 * @param ch the character to print
 * @return The input character
 */
int putbyte(char ch) {
  int result;

  result = myputbyte(ch);
  update_cursor(cursor_row, cursor_col);
  
  return result;
}

/** 
 * @brief Prints the string s, starting at the current
 *        location of the cursor.
 *
 * If the string is longer than the current line, the
 * string fills up the current line and then
 * continues on the next line. If the string exceeds
 * available space on the entire console, the screen
 * scrolls up one line, and then the string
 * continues on the new line.  If '\n', '\r', and '\b' are
 * encountered within the string, they are handled
 * as per putbyte. If len is not a positive integer or s
 * is null, the function has no effect.
 *
 * @param s The string to be printed.
 * @param len The length of the string s.
 */
void putbytes(const char *s, int len) {
  int i;

  if (s != NULL && len >= 0) {
    for (i = 0; i < len; i++)
      myputbyte(s[i]);
  }
  update_cursor(cursor_row, cursor_col);
}


/** 
 * @brief Prints character ch with the specified color
 *        at position (row, col).
 *
 * If any argument is invalid, the function has no effect.
 *
 * @param row The row in which to display the character.
 * @param col The column in which to display the character.
 * @param ch The character to display.
 * @param color The color to use to display the character.
 */

void draw_char(int row, int col, int ch, int color) {
  write_to_console(row, col, HI, ch);
  write_to_console(row, col, LO, color);
}

/** 
 * @brief Returns the character displayed at position (row, col).
 * @param row Row of the character.
 * @param col Column of the character.
 * @return The character at (row, col).
 */
char get_char(int row, int col) {
  return get_from_console(row, col, HI);
}

/** 
 * @brief Changes the foreground and background color
 *        of future characters printed on the console.
 *
 * If the color code is invalid, the function has no effect.
 *
 * @param color The new color code.
 * @return 0 on success or integer error code less than 0 if
 *         color code is invalid.
 */
int set_term_color(int color) {
  int ret;

  if (color <= (BGND_LGRAY | FGND_WHITE)) {
    update_fields(cursor_row, cursor_col, color);
    ret = SUCCESS;
  }
  else {
    ret = INVALID_COLOR;
  }

  return ret;
}

/** 
 * @brief Writes the current foreground and background
 *        color of characters printed on the console
 *        into the argument color.
 * @param color The address to which the current color
 *        information will be written.
 */
void get_term_color(int *color) {
  if (color == NULL) 
    return;

  *color = term_color;
}

/** 
 * @brief Sets the position of the cursor to the
 *        position (row, col).
 *
 * Subsequent calls to putbytes should cause the console
 * output to begin at the new position. If the cursor is
 * currently hidden, a call to set_cursor() does not show
 * the cursor.
 *
 * @param row The new row for the cursor.
 * @param col The new column for the cursor.
 * @return 0 on success or integer error code less than 0 if
 *         cursor location is invalid.
 */
int set_cursor(int row, int col) {
  int result;
  
  if (0 <= row && row < CONSOLE_HEIGHT && 0 <= col && col < CONSOLE_WIDTH) {
    update_fields(row, col, term_color);
    update_cursor(cursor_row, cursor_col);
    result = SUCCESS;
  }
  else {
    result = INDEX_OUT_OF_BOUNDS;
  }

  return result;
}

/** 
 * @brief Writes the current position of the cursor
 *        into the arguments row and col.
 * @param row The address to which the current cursor
 *        row will be written.
 * @param col The address to which the current cursor
 *        column will be written.
 */
void get_cursor(int *row, int *col) {
  if (row == NULL || col == NULL) 
    return;

  *row = cursor_row;
  *col = cursor_col;
}  

/** 
 * @brief Clears the entire console.
 *
 * The cursor is reset to the first row and column
 *
 * @return Void.
 */
void clear_console(void) {
  int row, col;

  /* place a space in each position */
  for (row = 0; row < CONSOLE_HEIGHT; row++)
    for (col = 0; col < CONSOLE_WIDTH; col++)
      draw_char(row, col, ' ', term_color);

  cursor_row = cursor_col = 0;
  update_cursor(cursor_row, cursor_col);
}
