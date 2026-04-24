/**
 * @file    delay.h
 * @brief   Public API for software busy-wait delay routines (delay.c).
 *
 * All delays are calibrated for the LPC2148 running at FOSC = 12 MHz.
 * These are blocking delays; the CPU does not service interrupts while
 * waiting (interrupts ARE serviced if they are enabled, but the caller
 * does not regain control until the delay expires).
 */

#ifndef DELAY_H
#define DELAY_H

/** @brief  Wait for approximately `delay` microseconds. */
void delay_us(unsigned int delay);

/** @brief  Wait for approximately `delay` milliseconds. */
void delay_ms(unsigned int delay);

/** @brief  Wait for approximately `delay` seconds.
 *  @warning Argument must be small (≤3) to avoid 32-bit overflow. */
void delay_s(unsigned int delay);

#endif /* DELAY_H */
