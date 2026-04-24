/**
 * @file    dotmatrix.h
 * @brief   Public API for the dot-matrix LED display driver (dot_matrix.c).
 *
 * Exposes three functions for initialising the display hardware and
 * rendering text on the four 8×8 dot-matrix LED panels.
 */

#ifndef DOTMATRIX_H
#define DOTMATRIX_H

#include "types.h"

/**
 * @brief  Initialise the dot-matrix row GPIO and shift register pins.
 *         Must be called once during system startup before any Display() call.
 */
void Init_dm_rows(void);

/**
 * @brief  Display exactly 4 characters on the four panels for `dly` refresh cycles.
 * @param  str  Pointer to a 4-character (or longer) string buffer.
 * @param  dly  Number of full 8-row scan cycles to sustain the display.
 */
void display_str(u8 *str, u32 dly);

/**
 * @brief  Scroll an arbitrary-length string across all four panels.
 *
 * Scrolls `ptr` character by character using a 4-wide sliding window.
 * Returns immediately if sms_flag is set (new SMS pending).
 *
 * @param  ptr  Null-terminated string to scroll.
 * @param  num  Reserved (unused); kept for API compatibility.
 */
void Display(u8 *ptr, u32 num);

#endif /* DOTMATRIX_H */
