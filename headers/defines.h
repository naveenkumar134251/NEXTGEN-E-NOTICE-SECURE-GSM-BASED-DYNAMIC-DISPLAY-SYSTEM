/**
 * @file    defines.h
 * @brief   Bit and field manipulation macros for LPC2148 register access.
 *
 * Provides a set of generic, type-independent macros for reading and
 * writing individual bits, nibbles, bytes, and half-words within any
 * 32-bit register or variable.  Used throughout the project wherever
 * direct hardware register manipulation is required (GPIO, UART, I²C, etc.).
 *
 * All macros perform a read-modify-write to avoid disturbing other bits
 * in the target word.
 */

#ifndef DEFINES_H
#define DEFINES_H

/** @brief Set a single bit BP in WORD to 1. */
#define SETBIT(WORD, BP)            WORD |= (1 << BP)

/** @brief Clear a single bit BP in WORD to 0. */
#define CLRBIT(WORD, BP)            WORD &= ~(1 << BP)

/** @brief Toggle (complement) a single bit BP in WORD. */
#define CPLBIT(WORD, BP)            WORD ^= (1 << BP)

/**
 * @brief  Write a single bit value BIT to position BP in WORD.
 * @param  WORD  Target register or variable.
 * @param  BP    Bit position (0–31).
 * @param  BIT   Bit value to write (0 or 1).
 */
#define WRITEBIT(WORD, BP, BIT)     WORD = ((WORD & ~(1 << BP)) | (BIT << BP))

/**
 * @brief  Write a 4-bit nibble value to bit position SBP in WORD.
 * @param  WORD    Target register or variable.
 * @param  SBP     Starting bit position of the nibble (0–28).
 * @param  NIBBLE  4-bit value to write (0x0–0xF).
 */
#define WRITENIBBLE(WORD, SBP, NIBBLE)  WORD = ((WORD & ~(0xf << SBP)) | (NIBBLE << SBP))

/**
 * @brief  Write an 8-bit byte value to bit position SBP in WORD.
 *
 * Commonly used for driving the row lines of the dot-matrix display
 * (e.g., WRITEBYTE(IOPIN0, ROWS, 0xFF) to blank all rows).
 *
 * @param  WORD  Target register or variable.
 * @param  SBP   Starting bit position of the byte field (0–24).
 * @param  BYTE  8-bit value to write (0x00–0xFF).
 */
#define WRITEBYTE(WORD, SBP, BYTE)  WORD = ((WORD & ~(0xff << SBP)) | (BYTE << SBP))

/**
 * @brief  Write a 16-bit half-word value to bit position SBP in WORD.
 * @param  WORD   Target register or variable.
 * @param  SBP    Starting bit position of the half-word field (0–16).
 * @param  HWORD  16-bit value to write (0x0000–0xFFFF).
 */
#define WRITEHWORD(WORD, SBP, HWORD) WORD = ((WORD & ~(0xffff << SBP)) | (HWORD << SBP))

#endif /* DEFINES_H */
