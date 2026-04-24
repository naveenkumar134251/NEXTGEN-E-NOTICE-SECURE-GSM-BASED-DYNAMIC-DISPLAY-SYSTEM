/**
 * @file    sipo_74hc164.h
 * @brief   Public API for the 74HC164 SIPO shift register driver (sipo.c).
 *
 * Declares the initialisation function and four independent byte-shift
 * functions, one per 74HC164 IC (one IC per 8×8 dot-matrix panel).
 *
 * Each SIPO_74HC164_x() function shifts 8 bits, MSB first, into the
 * corresponding shift register using bit-banged GPIO.  The shift register
 * outputs (Q0–Q7) drive the column lines of the associated dot-matrix panel.
 *
 * Column bitmaps are active-low: a '0' bit turns the column LED on.
 */

#ifndef SIPO_74HC164_H
#define SIPO_74HC164_H

/**
 * @brief  Configure all SIN and CP GPIO pins for the four shift registers as outputs.
 *         Must be called once before any SIPO_74HC164_x() is used.
 */
void Init_SIPO_74HC164(void);

/**
 * @brief  Shift one column-bitmap byte into the 74HC164 driving Display 1.
 * @param  sDat  8-bit column pattern (bit 7 → Q0/COL1, bit 0 → Q7/COL8).
 */
void SIPO_74HC164_1(unsigned char sDat);

/**
 * @brief  Shift one column-bitmap byte into the 74HC164 driving Display 2.
 * @param  sDat  8-bit column pattern.
 */
void SIPO_74HC164_2(unsigned char sDat);

/**
 * @brief  Shift one column-bitmap byte into the 74HC164 driving Display 3.
 * @param  sDat  8-bit column pattern.
 */
void SIPO_74HC164_3(unsigned char sDat);

/**
 * @brief  Shift one column-bitmap byte into the 74HC164 driving Display 4.
 * @param  sDat  8-bit column pattern.
 */
void SIPO_74HC164_4(unsigned char sDat);

#endif /* SIPO_74HC164_H */
