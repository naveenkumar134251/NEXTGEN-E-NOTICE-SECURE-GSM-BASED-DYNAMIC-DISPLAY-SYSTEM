/**
 * @file    dot_matrix.c
 * @brief   Dot-matrix LED display driver for LPC2148.
 *
 * Drives four 8×8 dot-matrix LED panels arranged side by side using:
 *   - Four 74HC164 SIPO shift registers for column data (one per panel).
 *   - A 74HC573 latch (via P0.16–P0.23, alias ROWS at bit 16) for row
 *     multiplexing (one row active at a time).
 *
 * The display is refreshed by scanning each of the 8 rows in turn,
 * loading the appropriate column bitmap from the font table via the
 * shift registers, then enabling that row briefly (~40 µs).
 *
 * Font data is defined in dot_mat_def.h as a 96-entry × 8-byte array
 * covering printable ASCII characters (space = index 0, '~' = index 94).
 */

#include "types.h"
#include <LPC21xx.H>
#include <string.h>
#include "delay.h"
#include "defines.h"
#include "sipo_74hc164.h"
#include "dot_mat_def.h"
#include "dotmatrix.h"

/* sms_flag is set by the UART ISR when a +CMTI notification is received.
 * Checked here so that ongoing display operations can be interrupted
 * immediately when a new SMS arrives.                                       */
extern u8 sms_flag;

/* -------------------------------------------------------------------------
 * Init_dm_rows
 * ------------------------------------------------------------------------- */

/**
 * @brief  Initialise the dot-matrix row driver and shift register GPIOs.
 *
 * Configures the SIPO shift register pins as outputs (via Init_SIPO_74HC164),
 * then sets the row control pins (P0.16–P0.23, ROWS bitmask) as outputs.
 * All rows start LOW (display off).
 */
void Init_dm_rows(void)
{
    Init_SIPO_74HC164();                  /* Configure column driver GPIO pins */
    WRITEBYTE(IODIR0, ROWS, 0xFF);        /* Set row pins as outputs           */
}

/* -------------------------------------------------------------------------
 * display_str
 * ------------------------------------------------------------------------- */

/**
 * @brief  Display a 4-character string on the four dot-matrix panels.
 *
 * Performs `delay` full refresh cycles.  Each cycle scans all 8 rows:
 *   1. Blank all rows (prevent ghosting).
 *   2. Load column bitmaps for each of the 4 characters into their
 *      respective 74HC164 shift registers.
 *   3. Enable the current row for ~40 µs.
 *
 * Characters are looked up in the `font` array using ASCII offset 32
 * (i.e., font[ch - 32][row]).
 *
 * @param  str    Pointer to a 4-character (minimum) string buffer.
 * @param  delay  Number of complete refresh cycles to display the string.
 *                Higher values = longer display time.
 */
void display_str(u8 *str, u32 delay)
{
    u8  i;     /* Row index (0–7)          */
    u32 d;     /* Delay loop counter       */

    for (d = 0; d < delay; d++)
    {
        for (i = 0; i < 8; i++)           /* Scan all 8 rows                   */
        {
            /* Abort display immediately if a new SMS has been detected          */
            if (sms_flag == 1)
                break;

            WRITEBYTE(IOPIN0, ROWS, 0x00);            /* Blank rows (active low) */

            /* Load column data for each of the 4 panels via their shift regs   */
            SIPO_74HC164_1(font[str[0] - 32][i]);     /* Panel 1 column bitmap   */
            SIPO_74HC164_2(font[str[1] - 32][i]);     /* Panel 2 column bitmap   */
            SIPO_74HC164_3(font[str[2] - 32][i]);     /* Panel 3 column bitmap   */
            SIPO_74HC164_4(font[str[3] - 32][i]);     /* Panel 4 column bitmap   */

            WRITEBYTE(IOPIN0, ROWS, (1 << i));        /* Enable row i            */
            delay_us(40);                              /* Hold row for 40 µs      */
        }
    }
}

/* -------------------------------------------------------------------------
 * Display
 * ------------------------------------------------------------------------- */

/**
 * @brief  Scroll an arbitrary-length string across the four-panel display.
 *
 * Implements a software scrolling effect by repeatedly calling display_str()
 * with a sliding 4-character window over the input string.  Each window
 * position is displayed for 500 refresh cycles before advancing by one
 * character.
 *
 * Scrolling stops immediately if sms_flag is set (new SMS received).
 *
 * @param  ptr  Pointer to the null-terminated string to scroll.
 * @param  num  Unused (reserved for future extension; kept for API compat).
 */
void Display(u8 *ptr, u32 num)
{
    s32 j;
    u32 len;

    len = mystrlen(ptr);                  /* Determine string length           */

    /* Slide the 4-character window from position 0 to (len - 4)              */
    for (j = 0; j < len - 3; j++)
    {
        if (sms_flag == 1)                /* Stop scrolling on new SMS         */
            break;
        display_str(ptr + j, 500);       /* Display 4 chars at offset j       */
    }
}

/* -------------------------------------------------------------------------
 * mystrlen
 * ------------------------------------------------------------------------- */

/**
 * @brief  Custom strlen for u8 (unsigned char) strings.
 *
 * Standard strlen operates on signed char; this version avoids implicit
 * sign-conversion warnings when working with the project's u8 type alias.
 *
 * @param  str  Pointer to a null-terminated u8 string.
 * @return      Number of characters before the null terminator.
 */
u32 mystrlen(u8 *str)
{
    u32 i, cnt = 0;
    for (i = 0; str[i] != '\0'; i++)
        cnt++;
    return cnt;
}
