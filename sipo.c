/**
 * @file    sipo.c
 * @brief   74HC164 Serial-In / Parallel-Out shift register driver for LPC2148.
 *
 * The 74HC164 is an 8-bit SIPO shift register used to drive the column
 * lines of the 8×8 dot-matrix LED panels.  Four independent 74HC164 ICs
 * are used, one per display panel.
 *
 * Each IC is controlled by two GPIO pins:
 *   SIN (DSA) — serial data input (MSB shifted in first)
 *   CP        — clock pulse (data clocked on rising edge)
 *
 * GPIO pin mapping (all on Port 0):
 *   Display 1:  SIN=P0.8,  CP=P0.9
 *   Display 2:  SIN=P0.10, CP=P0.11
 *   Display 3:  SIN=P0.12, CP=P0.13
 *   Display 4:  SIN=P0.14, CP=P0.15
 *
 * Bit order: MSB first (bit 7 shifted in first so Q0 receives bit 7).
 * Column output is active-low (a '0' turns the column LED on).
 */

#include <LPC21xx.H>
#include "delay.h"
#include "defines.h"
#include "types.h"

/* --- Pin definitions for 74HC164 shift register #1 (Display 1) ---------- */
#define SIN_1  8    /* P0.8  — Serial data input for panel 1               */
#define CP_1   9    /* P0.9  — Clock pulse for panel 1                     */

/* --- Pin definitions for 74HC164 shift register #2 (Display 2) ---------- */
#define SIN_2  10   /* P0.10 — Serial data input for panel 2               */
#define CP_2   11   /* P0.11 — Clock pulse for panel 2                     */

/* --- Pin definitions for 74HC164 shift register #3 (Display 3) ---------- */
#define SIN_3  12   /* P0.12 — Serial data input for panel 3               */
#define CP_3   13   /* P0.13 — Clock pulse for panel 3                     */

/* --- Pin definitions for 74HC164 shift register #4 (Display 4) ---------- */
#define SIN_4  14   /* P0.14 — Serial data input for panel 4               */
#define CP_4   15   /* P0.15 — Clock pulse for panel 4                     */

/* -------------------------------------------------------------------------
 * Init_SIPO_74HC164
 * ------------------------------------------------------------------------- */

/**
 * @brief  Configure all 8 SIPO GPIO pins (SIN + CP for all 4 ICs) as outputs.
 *
 * Must be called once before any SIPO_74HC164_x() function is used.
 * Uses the SETBIT macro from defines.h to set individual bits in IODIR0.
 */
void Init_SIPO_74HC164(void)
{
    SETBIT(IODIR0, SIN_1);   /* P0.8  as output */
    SETBIT(IODIR0, CP_1);    /* P0.9  as output */
    SETBIT(IODIR0, SIN_2);   /* P0.10 as output */
    SETBIT(IODIR0, CP_2);    /* P0.11 as output */
    SETBIT(IODIR0, SIN_3);   /* P0.12 as output */
    SETBIT(IODIR0, CP_3);    /* P0.13 as output */
    SETBIT(IODIR0, SIN_4);   /* P0.14 as output */
    SETBIT(IODIR0, CP_4);    /* P0.15 as output */
}

/* -------------------------------------------------------------------------
 * SIPO_74HC164_1
 * ------------------------------------------------------------------------- */

/**
 * @brief  Shift one byte into the 74HC164 driving Display 1 columns.
 *
 * Clocks 8 bits, MSB first, into the shift register on SIN_1/CP_1.
 * Each bit is held for 1 µs before the rising clock edge to satisfy
 * the 74HC164 setup time.
 *
 * @param  sDat  Column bitmap byte for the current row of Display 1.
 *               Bit 7 → Q0 (COL1), Bit 0 → Q7 (COL8).
 *               Active-low: '0' = LED column on, '1' = LED column off.
 */
void SIPO_74HC164_1(u8 sDat)
{
    u8 i;
    for (i = 0; i < 8; i++)
    {
        WRITEBIT(IOPIN0, SIN_1, ((sDat >> (7 - i)) & 1)); /* Put bit on SIN */
        IOCLR0 = 1 << CP_1;       /* CP low                                 */
        delay_us(1);
        IOSET0 = 1 << CP_1;       /* CP high — data clocked into register    */
        delay_us(1);
    }
}

/* -------------------------------------------------------------------------
 * SIPO_74HC164_2
 * ------------------------------------------------------------------------- */

/**
 * @brief  Shift one byte into the 74HC164 driving Display 2 columns.
 *
 * Identical operation to SIPO_74HC164_1() but uses SIN_2/CP_2 pins.
 *
 * @param  sDat  Column bitmap byte for the current row of Display 2.
 */
void SIPO_74HC164_2(u8 sDat)
{
    u8 i;
    for (i = 0; i < 8; i++)
    {
        WRITEBIT(IOPIN0, SIN_2, ((sDat >> (7 - i)) & 1));
        IOCLR0 = 1 << CP_2;
        delay_us(1);
        IOSET0 = 1 << CP_2;
        delay_us(1);
    }
}

/* -------------------------------------------------------------------------
 * SIPO_74HC164_3
 * ------------------------------------------------------------------------- */

/**
 * @brief  Shift one byte into the 74HC164 driving Display 3 columns.
 *
 * Identical operation to SIPO_74HC164_1() but uses SIN_3/CP_3 pins.
 *
 * @param  sDat  Column bitmap byte for the current row of Display 3.
 */
void SIPO_74HC164_3(u8 sDat)
{
    u8 i;
    for (i = 0; i < 8; i++)
    {
        WRITEBIT(IOPIN0, SIN_3, ((sDat >> (7 - i)) & 1));
        IOCLR0 = 1 << CP_3;
        delay_us(1);
        IOSET0 = 1 << CP_3;
        delay_us(1);
    }
}

/* -------------------------------------------------------------------------
 * SIPO_74HC164_4
 * ------------------------------------------------------------------------- */

/**
 * @brief  Shift one byte into the 74HC164 driving Display 4 columns.
 *
 * Identical operation to SIPO_74HC164_1() but uses SIN_4/CP_4 pins.
 *
 * @param  sDat  Column bitmap byte for the current row of Display 4.
 */
void SIPO_74HC164_4(u8 sDat)
{
    u8 i;
    for (i = 0; i < 8; i++)
    {
        WRITEBIT(IOPIN0, SIN_4, ((sDat >> (7 - i)) & 1));
        IOCLR0 = 1 << CP_4;
        delay_us(1);
        IOSET0 = 1 << CP_4;
        delay_us(1);
    }
}
