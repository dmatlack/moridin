/** 
 * @file console.h
 * @brief Function prototypes for the console driver.
 *
 * This contains the prototypes and global variables for the console
 * driver
 *
 * @author Mark Wong Siang Kai (msiangka)
 * @bug No known bugs.
 */

#ifndef __CONSOLE_H_
#define __CONSOLE_H_

#define SUCCESS 0
#define INDEX_OUT_OF_BOUNDS -1
#define INVALID_COLOR -2
#define INVALID_FUNCTION -3
#define CONSOLE_SUCCESS 0
#define CONSOLE_FAILURE -2

int   console_init   (void);
int   putbyte        (char ch);
void  putbytes       (const char* s, int len);
int   set_term_color (int color);
void  get_term_color (int* color);
int   set_cursor     (int row, int col);
void  get_cursor     (int* row, int* col);
void  clear_console  (void);
void  draw_char      (int row, int col, int ch, int color);
char  get_char       (int row, int col);

#endif /* __CONSOLE_H_ */
