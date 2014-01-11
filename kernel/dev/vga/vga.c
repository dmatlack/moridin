/**
 * @file vga.c
 *
 * @brief Device driver for the VGA.
 *
 * @author David Matlack
 */
#include <dev/vga.h>
#include <kernel/io.h>

#include <assert.h>
#include <debug.h>

#define OFFSET(row, col) ((row)*(VGA_COLS) + (col))
#define ROW(offset)      ((offset)/(VGA_COLS))
#define COL(offset)      ((offset) - (VGA_COLS*ROW(offset)))

#define EMPTY_CHAR ' ' 

#define VGA_ASSERTS(_row, _col) \
  do { \
    ASSERT_GREATEREQ(_row, 0);   \
    ASSERT_LESS(_row, VGA_ROWS); \
    ASSERT_GREATEREQ(_col, 0);   \
    ASSERT_LESS(_col, VGA_COLS); \
  } while (0)

struct {
  int cursor_row;
  int cursor_col;
  char color;
  //TODO will need a lock eventually
} vga;

static inline char *addr(int row, int col) {
  VGA_ASSERTS(row, col);
  return (char *)(VGA_COLOR_TEXT_BUFFER_ADDR + 2*OFFSET(row, col));
}
static inline char get_char(int row, int col) {
  VGA_ASSERTS(row, col);
  return *(addr(row, col) + 0);
}
static inline void set_char(int row, int col, char ch) {
  *(addr(row, col) + 0) = ch;
}
static inline void set_color(int row, int col, char color) {
  *(addr(row, col) + 1) = color;
}

/**
 * @brief draw the character at the given position, with the current color.
 */
static void draw_char(int row, int col, char ch, char color) {
  VGA_ASSERTS(row, col);
  set_char(row, col, ch);
  set_color(row, col, color);
}

/**
 * @brief Copy a row.
 */
static void copy_row(int from, int to) {
  int col;

  ASSERT_GREATEREQ(from, 0);
  ASSERT_LESS(from, VGA_COLS);
  ASSERT_GREATEREQ(to, 0);
  ASSERT_LESS(to, VGA_COLS);

  if (from == to) return;

  for (col = 0; col < VGA_COLS; col++) {
    draw_char(to, col, get_char(from, col), vga.color);
  }
}

/**
 * @brief Scroll the screen by i lines.
 */
static void scroll(int i) {
  int row, col;

  ASSERT_GREATEREQ(i, 0);

  for (row = i; row < VGA_ROWS; row++) {
    copy_row(row, row - i);
  }

  for (row = (VGA_ROWS - i); row < VGA_ROWS; row++) {
    for (col = 0; col < VGA_COLS; col++) {
      draw_char(row, col, EMPTY_CHAR, vga.color);
    }
  }
  
}

/**
 * @brief Put the cursor at the beginning of the next line, scrolling
 * if necessary.
 */
static void do_newline(void) {
  vga.cursor_col = 0;

  VGA_ASSERTS(vga.cursor_row, vga.cursor_col);

  if (vga.cursor_row == (VGA_ROWS-1)) {
    scroll(1);
  }
  else {
    vga.cursor_row++;
  }
}

/**
 * @brief Increment the cursor by i positions (where i can be positive or 
 * negative.
 *
 * Attempting to increment the cursor to a position before offset 0 (the top
 * left of the screen) will cause the cursor to just stay at the top left
 * of the screen.
 *
 * Attempting to increment the cursor to a position past the end of the screen
 * will cause the screen to scroll forward.
 */
static void move_cursor(int i) {
  int offset = OFFSET(vga.cursor_row, vga.cursor_col);
  int overflow;

  offset += i;
  overflow = offset - (VGA_SIZE - 1);

  if (offset < 0) {
    offset = 0;
  }
  if (overflow > 0) {
    int row_overflow = 1 + (overflow / VGA_COLS);

    scroll(row_overflow);
    offset -= (row_overflow * VGA_COLS);
  }

  vga.cursor_row = ROW(offset);
  vga.cursor_col = COL(offset);
}

/**
 * @brief Draw the character to the screen.
 */
static void putbyte(char ch) {
  VGA_ASSERTS(vga.cursor_row, vga.cursor_col);
  switch (ch) {
    case '\n':
      do_newline();
      break;
    case '\b':
      if (!(0 == vga.cursor_row && 0 == vga.cursor_col)) {
        move_cursor(-1);
        draw_char(vga.cursor_row, vga.cursor_col, EMPTY_CHAR, vga.color);
      }
      break;
    case '\r':
      vga.cursor_col = 0;
      break;
    default:
      draw_char(vga.cursor_row, vga.cursor_col, ch, vga.color);
      move_cursor(1);
      break;
  }
}

/**
 * @brief Sets the cursor on the screen to the specified row and
 * column.
 *
 * This function updates both our internal copy of the cursor's location
 * AND the actualy cursor's location on the screen.
 */
static void crtc_set_cursor(int row, int col) {
  int offset = OFFSET(row, col);

  VGA_ASSERTS(row, col);

  // update our copy
  vga.cursor_row = row;
  vga.cursor_col = col;

  // update the CRTC
  outb(CRTC_INDEX_REG, CRTC_CURSOR_LOCATION_LSB);
  outb(CRTC_DATA_REG, (uint8_t) offset & 0xff);
  outb(CRTC_INDEX_REG, CRTC_CURSOR_LOCATION_MSB);
  outb(CRTC_DATA_REG, (uint8_t) (offset >> 8) & 0xff);
}

/**
 * @brief Update the CRT cursor with whatever our current copy of the
 * cursor's location is.
 */
static void crtc_update_cursor(void) {
  crtc_set_cursor(vga.cursor_row, vga.cursor_col);
}

void vga_init(void) {
  vga.cursor_row = 0;
  vga.cursor_col = 0;
  vga.color = VGA_GREEN;
}

void vga_putbyte(char ch) {
  putbyte(ch);
  crtc_update_cursor();
}

void vga_putbytes(const char* s, int len) {
  int i;

  ASSERT_NOT_NULL(s);
  ASSERT_GREATEREQ(len, 0);

  for (i = 0; i < len; i++) {
    putbyte(s[i]);
  }
  crtc_update_cursor();
}

char vga_set_color(char color) {
  char old_color;
 
  old_color = vga.color;
  vga.color = color;

  return old_color;
}

char vga_get_color(void) {
  return vga.color;
}

void vga_set_cursor(int row, int col) {
  if (row < 0 || row >= VGA_ROWS || col < 0 || col >= VGA_COLS) {
    WARN("%s: invalid (row, col): (%d, %d)", __func__, row, col);
    return;
  }
  crtc_set_cursor(row, col);
}

void vga_get_cursor(int* row, int* col) {
  *row = vga.cursor_row;
  *col = vga.cursor_col;
}

void vga_clear(void) {
  int row, col;

  for (row = 0; row < VGA_ROWS; row++) {
    for (col = 0; col < VGA_COLS; col++) {
      draw_char(row, col, EMPTY_CHAR, vga.color);
    }
  }
  crtc_set_cursor(0, 0);
}

void vga_draw_char(int row, int col, char ch, char color) {
  if (row < 0 || row >= VGA_ROWS || col < 0 || col >= VGA_COLS) {
    WARN("%s: invalid (row, col): (%d, %d)", __func__, row, col);
    return;
  }
  draw_char(row, col, ch, color);
}

char vga_get_char(int row, int col) {
  char c;

  if (row < 0 || row >= VGA_ROWS || col < 0 || col >= VGA_COLS) {
    WARN("%s: invalid (row, col): (%d, %d)", __func__, row, col);
    return 0;
  }

  c = get_char(row, col);

  return c;
}
