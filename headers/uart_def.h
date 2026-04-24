/**
 * @file    uart_def.h
 * @brief   UART0 hardware configuration constants for LPC2148.
 *
 * Contains baud rate calculation, pin-function select values, VIC channel
 * assignment, and U0LCR / U0IER bit definitions used by uart.c.
 *
 * Baud rate derivation:
 *   PCLK     = CCLK / 4 = 60 MHz / 4 = 15 MHz
 *   DIVISOR  = PCLK / (16 × BAUD) = 15,000,000 / (16 × 9600) = 97.65…
 *            → rounded to 97  (≈0.35% error, within UART tolerance)
 *   Written:  U0DLL = 97, U0DLM = 0
 */

#ifndef UART_DEF_H
#define UART_DEF_H

/* --- Debug LED pin assignments (optional, not used in main flow) --------- */
#define TX_LED   16    /**< P1.16 — TX activity LED (optional)               */
#define RX_LED   17    /**< P1.17 — RX activity LED (optional)               */

/* --- Clock configuration ------------------------------------------------- */
#define FOSC     12000000UL          /**< Crystal frequency = 12 MHz          */
#define CCLK     (5 * FOSC)          /**< CPU clock = 60 MHz (PLL ×5)         */
#define PCLK     (CCLK / 4)          /**< Peripheral clock = 15 MHz           */
#define BAUD     9600                 /**< Target baud rate                    */
#define DIVISOR  (PCLK / (16 * BAUD)) /**< DLL value ≈ 97 for 9600 baud      */

/* --- VIC channel number for UART0 --------------------------------------- */
#define UART0_CHNO   6   /**< UART0 is VIC channel 6 on LPC2148              */

/* --- PINSEL0 bit values to enable UART0 pins ----------------------------- */
#define TXD0_EN   (1 << 0)   /**< PINSEL0[1:0] = 01 → P0.0 = TXD0           */
#define RXD0_EN   (1 << 2)   /**< PINSEL0[3:2] = 01 → P0.1 = RXD0           */

/* --- U0LCR word-length selection ----------------------------------------- */
#define _8BIT          3    /**< WLS bits = 11 → 8-bit character length       */
#define WORD_LEN_SEL   _8BIT
#define DLAB_BIT       7    /**< Divisor Latch Access Bit position in U0LCR   */

/* --- U0LSR status bit positions ------------------------------------------ */
#define DR_BIT     0    /**< Data Ready — a complete character is in U0RBR    */
#define THRE_BIT   5    /**< THR Empty — Transmit Holding Register is empty   */
#define TEMT_BIT   6    /**< Transmitter Empty — both THR and TSR are empty   */

/* --- U0IER interrupt enable bit masks ------------------------------------ */
#define RBR_INTERRUPT_EN    (1 << 0)  /**< Enable Receive Data Available IRQ  */
#define THRE_INTERRUPT_EN   (1 << 1)  /**< Enable THRE (transmit empty) IRQ   */

#endif /* UART_DEF_H */
