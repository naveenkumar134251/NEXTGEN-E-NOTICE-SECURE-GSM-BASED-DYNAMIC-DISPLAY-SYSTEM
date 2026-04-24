/**
 * @file    uart.h
 * @brief   Public API for the UART0 driver (uart.c).
 *
 * Provides UART0 initialisation and transmit functions for communicating
 * with the M660A GSM module at 9600-8-N-1.  Reception is interrupt-driven;
 * received data is stored in the shared rx_buf[] array and processed by
 * the UART0 ISR.
 *
 * The rx_buf[] buffer is declared here as extern so that gsm.c and main.c
 * can inspect its contents after waiting for sufficient bytes.
 */

#ifndef UART_H
#define UART_H

#include "types.h"

/** @brief  Initialise UART0 at 9600 baud with VIC interrupt-driven receive. */
void UART0_Init(void);

/**
 * @brief  Transmit a single byte over UART0 (blocking).
 * @param  ch  Byte to send.
 */
void UART0_Tx(u8 ch);

/**
 * @brief  Transmit a null-terminated string over UART0 (blocking).
 * @param  str  Pointer to the string to send.
 */
void UART0_Str(u8 *str);

/**
 * @brief  Receive a single character over UART0 (polling, blocking).
 *
 * Provided as a fallback; the primary receive path is the UART0 ISR
 * which fills rx_buf[] automatically.
 *
 * @return  The received character.
 */
char UART0_Rx(void);

/**
 * @brief  Shared receive buffer filled by the UART0 ISR.
 *
 * Declared extern here so modules that wait on AT command responses
 * (gsm.c, main.c) can inspect the received data directly.
 * Buffer size is 300 bytes (safe for full AT+CMGR SMS responses).
 */
extern u8 rx_buf[300];

#endif /* UART_H */
