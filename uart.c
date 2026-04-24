/**
 * @file    uart.c
 * @brief   UART0 driver for LPC2148 with interrupt-based SMS detection.
 *
 * Implements UART0 initialisation, interrupt-driven receive, and blocking
 * transmit for communication with the M660A GSM module at 9600 baud.
 *
 * Key design decisions:
 *   - Reception is interrupt-driven (UART0_interrupt ISR) to avoid missing
 *     characters while the main loop is busy scrolling the display.
 *   - The ISR scans the receive buffer for "+CMTI" (new-SMS notification)
 *     and sets sms_flag = 1 so the main loop can react asynchronously.
 *   - Transmission is blocking (polls the THRE flag) via UART0_Tx().
 *
 * Baud rate calculation (see uart_def.h):
 *   PCLK = 15 MHz, BAUD = 9600, DIVISOR = 15000000 / (16 × 9600) = 97
 *   → U0DLL = 97, U0DLM = 0
 *
 * Shared variables (also referenced in gsm.c and main.c):
 *   rx_buf[]   — circular receive buffer (400 bytes)
 *   index      — number of bytes received so far in rx_buf
 *   sms_flag   — set to 1 when "+CMTI" is detected in rx_buf
 */

#include "uart_def.h"
#include <LPC21xx.H>
#include "types.h"
#include <string.h>
#include "delay.h"
#include "fun.h"

/* --- Shared global buffers and flags ------------------------------------ */
u8  rx_buf[400];        /* UART receive buffer — filled by ISR              */
u32 index;              /* Write index into rx_buf (bytes received so far)  */
u8  sms_flag = 0;       /* 1 = new SMS notification received via +CMTI      */
u8  t_flag;             /* 1 = THRE interrupt fired (transmit buffer empty) */
u8  ch;                 /* Temporary storage for the received character      */
u8  new_var;            /* Holds U0IIR value read at start of ISR           */

/* -------------------------------------------------------------------------
 * UART0_interrupt  (ISR)
 * ------------------------------------------------------------------------- */

/**
 * @brief  UART0 Vectored Interrupt Service Routine.
 *
 * Handles two UART0 interrupt sources:
 *
 * 1. RDA (Receive Data Available, IIR = 0x04):
 *    Reads one character from the receive holding register (U0RBR) and
 *    appends it to rx_buf[].  After each byte, searches rx_buf for the
 *    "+CMTI" unsolicited result code that the GSM module sends when a
 *    new SMS is received.  Sets sms_flag = 1 if found.
 *
 * 2. THRE (Transmit Holding Register Empty, IIR = 0x02):
 *    Sets t_flag = 1 so that UART0_Tx() knows the byte was sent and
 *    can return.
 *
 * The function is declared __irq to ensure correct entry/exit in ARM
 * mode (saves/restores SPSR, adjusts LR).  VICVectAddr must be written
 * with 0 at the end to signal end-of-interrupt to the VIC.
 */
void UART0_interrupt(void) __irq
{
    new_var = U0IIR;               /* Read Interrupt Identification Register */

    if (new_var & 0x04)            /* RDA — receive data available           */
    {
        ch = U0RBR;                /* Read the received character            */

        if (index < 300)           /* Guard against rx_buf overflow          */
        {
            rx_buf[index++] = ch;  /* Store character in buffer              */

            /* Check if a new-SMS notification has just arrived.
             * "+CMTI" is sent by the GSM module as an unsolicited result
             * code whenever a new SMS is stored in its memory.              */
            if (strstr((s8 *)rx_buf, "+CMTI"))
            {
                sms_flag = 1;      /* Signal main loop to process new SMS    */
            }
        }
    }
    else if (new_var & 0x02)       /* THRE — transmit holding register empty */
    {
        t_flag = 1;                /* Signal UART0_Tx() that byte was sent   */
    }

    VICVectAddr = 0;               /* Acknowledge interrupt to VIC           */
}

/* -------------------------------------------------------------------------
 * UART0_Init
 * ------------------------------------------------------------------------- */

/**
 * @brief  Initialise UART0 for 9600-8-N-1 operation with VIC interrupts.
 *
 * Steps:
 *   1. Select UART0 pin functions on P0.0 (TXD0) and P0.1 (RXD0).
 *   2. Set word length to 8 bits and enable DLAB for baud rate access.
 *   3. Write divisor (97) to DLL/DLM for 9600 baud at PCLK = 15 MHz.
 *   4. Clear DLAB to restore normal register access.
 *   5. Configure VIC: assign ISR address, set channel priority, enable.
 *   6. Enable RBR (receive) and THRE (transmit) interrupt sources.
 */
void UART0_Init()
{
    /* Step 1: Enable UART0 TX and RX pin functions via PINSEL0             */
    PINSEL0 |= (TXD0_EN | RXD0_EN);

    /* Step 2: 8-bit word length + DLAB=1 to access divisor registers       */
    U0LCR = (WORD_LEN_SEL | (1 << DLAB_BIT));

    /* Step 3: Set baud rate divisor — 9600 baud at PCLK 15 MHz            */
    U0DLL = 97;                    /* Low byte of divisor                    */
    U0DLM = 0;                     /* High byte of divisor                   */

    /* Step 4: Clear DLAB to re-enable RBR/THR/IER registers               */
    U0LCR &= ~(1 << DLAB_BIT);

    /* Step 5: Register ISR in VIC slot 0 with UART0 channel number        */
    VICVectAddr0  = (unsigned)UART0_interrupt; /* ISR address               */
    VICVectCntl0  = 0x20 | UART0_CHNO;        /* Enable slot + channel 6   */
    VICIntEnable  = 1 << UART0_CHNO;          /* Enable UART0 interrupt     */

    /* Step 6: Enable RBR (receive) and THRE (transmit empty) interrupts   */
    U0IER = RBR_INTERRUPT_EN | THRE_INTERRUPT_EN;
}

/* -------------------------------------------------------------------------
 * UART0_Tx
 * ------------------------------------------------------------------------- */

/**
 * @brief  Transmit a single byte over UART0 (blocking).
 *
 * Clears t_flag, loads the byte into the Transmit Holding Register,
 * then busy-waits until the THRE ISR sets t_flag = 1 (byte transmitted).
 *
 * @param  ch  The byte to transmit.
 */
void UART0_Tx(u8 ch)
{
    t_flag = 0;                    /* Clear transmit-complete flag           */
    U0THR  = ch;                   /* Write byte to Transmit Holding Reg     */
    while (t_flag == 0);           /* Wait until THRE ISR confirms sent      */
}

/* -------------------------------------------------------------------------
 * UART0_Str
 * ------------------------------------------------------------------------- */

/**
 * @brief  Transmit a null-terminated string over UART0.
 *
 * Iterates over each character in `str` and calls UART0_Tx() for each
 * one.  Returns when the null terminator is reached.
 *
 * @param  str  Pointer to the null-terminated string to transmit.
 */
void UART0_Str(u8 *str)
{
    while (*str)
        UART0_Tx(*str++);
}

/* -------------------------------------------------------------------------
 * UART0_Rx
 * ------------------------------------------------------------------------- */

/**
 * @brief  Receive a single character from UART0 (polling, blocking).
 *
 * Polls the DR (Data Ready) bit in U0LSR until a character is available,
 * then reads and returns it from U0RBR.
 *
 * @note   This polling function is provided as a fallback; the main system
 *         uses interrupt-driven reception (UART0_interrupt ISR) instead.
 *
 * @return  The received character.
 */
char UART0_Rx(void)
{
    while (!(U0LSR & 0x01));       /* Wait until DR=1 (data ready)          */
    return (U0RBR);                /* Return received character              */
}
