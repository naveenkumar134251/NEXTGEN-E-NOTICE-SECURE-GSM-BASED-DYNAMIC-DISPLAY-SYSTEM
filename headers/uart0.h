/**
 * @file    uart0.h
 * @brief   Alternate minimal UART0 prototype declarations.
 *
 * This header provides a simplified set of UART0 function prototypes
 * using plain unsigned char (rather than the u8 typedef from types.h).
 * It is intended for use in standalone test modules or early-stage
 * development before types.h is available in the build.
 *
 * For the main project, prefer including uart.h (which uses u8 typedefs
 * and also exposes the extern rx_buf[] declaration).
 *
 * @note    The function signatures here must match the definitions in uart.c.
 */

#ifndef UART0_H
#define UART0_H

/** @brief  Initialise UART0 (9600-8-N-1 with VIC interrupt). */
void UART0_init(void);

/**
 * @brief  Transmit a single byte over UART0 (blocking).
 * @param  ch  Byte to transmit.
 */
void UART0_Tx(unsigned char ch);

/**
 * @brief  Transmit a null-terminated string over UART0 (blocking).
 * @param  str  Pointer to the string to send.
 */
void UART0_Str(unsigned char *str);

#endif /* UART0_H */
