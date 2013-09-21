/**
 * @file vga.h
 *
 * @brief Headerfile for the VGA (Video Graphics Array) device driver, which
 * handles displaying text early in the kernel startup process.
 *
 * To ensure backwards compatability, the BIOS starts up in Mode 7 (which is
 * originally from the 1981 MDA, or Monochrome Display Adapter). Mode 7 is a
 * color text mode that supports 80 characters x 20 lines.
 *
 * @reference http://www.brokenthorn.com/Resources/OSDev10.html
 *
 * @author David Matlack
 */
#ifndef __DEV_VGA_H__
#define __DEV_VGA_H__

/**
 * VGA Memory Layout
 *
 *    0xA0000 - 0xBFFFF Video Memory used for graphics modes
 *    0xB0000 - 0xB7777 Monochrome Text mode
 *    0xB8000 - 0xBFFFF Color text mode and CGA compatible graphics modes
 */
#define VGA_GRAPHICS_BUFFER_ADDR        0xA0000
#define VGA_GRAPHICS_BUFFER_SIZE        (0xC0000 - 0xA0000)
#define VGA_MONOCHROME_TEXT_BUFFER_ADDR 0xB0000
#define VGA_MONOCHROME_TEXT_BUFFER_SIZE (0xB8000 - 0xB0000)
#define VGA_COLOR_TEXT_BUFFER_ADDR      0xB8000
#define VGA_COLOR_TEXT_BUFFER_SIZE      (0xC0000 - 0xB8000)


/**
 * Mode 7 supports 80x25 character display.
 */
#define VGA_ROWS    25
#define VGA_HEIGHT  VGA_ROWS
#define VGA_COLS    80
#define VGA_WIDTH   VGA_COLS

/**
 * Color Text Buffer (0xB8000)
 *
 * +----------------+----------------+
 * | Character Byte | Attribute Byte |
 * +----------------+----------------+
 *
 * Attribute Byte
 *   Bits 0 - 2  Foreground color
 *   Bit 3       Foreground Intensity
 *   Bits 4 - 6  Background color
 *   Bit 7       Blinking or background intensity
 *
 */
#define VGA_BLACK      0x0
#define VGA_BLUE       0x1
#define VGA_GREEN      0x2
#define VGA_CYAN       0x3
#define VGA_RED        0x4
#define VGA_MAGENTA    0x5
#define VGA_BROWN      0x6
#define VGA_LGRAY      0x7
#define VGA_DGRAY      0x8
#define VGA_LBLUE      0x9
#define VGA_LGREEN     0xA
#define VGA_LCYAN      0xB
#define VGA_LRED       0xC
#define VGA_LMAGENTA   0xD
#define VGA_LBROWN     0xE
#define VGA_WHITE      0xF

/**
 * CRT Microcontroller - Controls various functions such as the cursor.
 *  
 *  Data Register: The data we want to write to the CRT.
 *  Index Register: The type of data we are writing.
 */
#define CRTC_DATA_REG  0x3D5
#define CRTC_INDEX_REG 0x3D4

#define CRTC_HORIZONTAL_TOTAL                  0x0   
#define CRTC_HORIZONTAL_DISPLAY_ENABLE_END     0x1   
#define CRTC_START_HORIZONTAL_BLANKING         0x2   
#define CRTC_END_HORIZONTAL_BLANKING           0x3   
#define CRTC_START_HORIZONTAL_RETRACE_PULSE    0x4   
#define CRTC_END_HORIZONTAL_RETRACE            0x5   
#define CRTC_VERTICAL_TOTAL                    0x6   
#define CRTC_OVERFLOW                          0x7   
#define CRTC_PRESET_ROW_SCAN                   0x8   
#define CRTC_MAXIMUM_SCAN_LINE                 0x9   
#define CRTC_CURSOR_START                      0xA   
#define CRTC_CURSOR_END                        0xB   
#define CRTC_START_ADDRESS_MSB                 0xC   
#define CRTC_START_ADDRESS_LSB                 0xD   
#define CRTC_CURSOR_LOCATION_MSB               0xE   
#define CRTC_CURSOR_LOCATION_LSB               0xF   
#define CRTC_VERTICAL_RETRACE_START            0x10  
#define CRTC_VERTICAL_RETRACE_END              0x11  
#define CRTC_VERTICAL_DISPLAY_ENABLE_END       0x12  
#define CRTC_OFFSET                            0x13  
#define CRTC_UNDERLINE_LOCATION                0x14  
#define CRTC_START_VERTICAL_BLANKING           0x15  
#define CRTC_END_VERTICAL_BLANKING             0x16  
#define CRTC_CRT_MODE_CONTROL                  0x17  
#define CRTC_LINE_COMPARE                      0x18  


int   vga_init           ();
int   vga_putbyte        (char ch);
void  vga_putbytes       (const char* s, int len);
int   vga_set_term_color (int color);
void  vga_get_term_color (int* color);
int   vga_set_cursor     (int row, int col);
void  vga_get_cursor     (int* row, int* col);
void  vga_clear_console  (void);
void  vga_draw_char      (int row, int col, int ch, int color);
char  vga_get_char       (int row, int col);

#endif /* __DEV_VGA_H__ */
