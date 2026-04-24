/**
 * @file    gsm.c
 * @brief   GSM module (M660A) interface driver for LPC2148.
 *
 * Provides functions to initialise the GSM modem and to send, read,
 * and delete SMS messages using standard Hayes AT commands over UART0.
 *
 * Initialisation sequence:
 *   AT         → check modem presence
 *   ATE0       → disable echo
 *   AT+CMGF=1  → switch to SMS text mode
 *   AT+CNMI=2,1,0,0,0 → enable new-message notification (+CMTI unsolicited result)
 *
 * All UART communication is handled by the UART0 driver (uart.c/uart.h).
 * The shared `rx_buf` and `index` variables are filled by the UART0 ISR.
 */

#include "uart.h"
#include "types.h"
#include <string.h>
#include <LPC21xx.H>
#include "gsm.h"
#include "dotmatrix.h"
#include "delay.h"

/* rx_buf and index are declared in uart.c and filled by the UART0 ISR.
 * index tracks how many bytes have been received into rx_buf.           */
extern u32 index;

/* -------------------------------------------------------------------------
 * gsm_init
 * ------------------------------------------------------------------------- */

/**
 * @brief  Initialise the GSM module with required AT command sequence.
 *
 * Sends each AT command in sequence and waits for an "OK" response.
 * On success, the dot-matrix display shows "GSM INIT SUCCESS".
 * On any step failure, an error message (GSM INIT ERROR 1–4) is shown
 * indicating which AT command did not respond with "OK".
 *
 * Error codes:
 *   ERROR 1 → AT command failed (modem not responding)
 *   ERROR 2 → ATE0 failed (echo-disable failed)
 *   ERROR 3 → AT+CMGF=1 failed (SMS mode not set)
 *   ERROR 4 → AT+CNMI failed (new message notification not configured)
 */
void gsm_init(void)
{
    /* --- Step 1: Basic modem check (AT) ---------------------------------- */
    index = 0;
    memset(rx_buf, '\0', 300);
    UART0_Str("AT\r\n");
    while (index < 3);                   /* Wait for at least 3 bytes in buffer */
    delay_ms(500);
    rx_buf[index] = '\0';

    if (strstr((char *)rx_buf, "OK"))
    {
        /* --- Step 2: Disable echo (ATE0) --------------------------------- */
        index = 0; memset(rx_buf, '\0', 300);
        UART0_Str("ATE0\r\n");
        while (index < 3);
        delay_ms(500);
        rx_buf[index] = '\0';

        if (strstr((s8 *)rx_buf, "OK"))
        {
            /* --- Step 3: Set SMS text mode (AT+CMGF=1) ------------------- */
            index = 0; memset(rx_buf, '\0', 300);
            UART0_Str("AT+CMGF=1\r\n");
            while (index < 3);
            delay_ms(500);
            rx_buf[index] = '\0';

            if (strstr((s8 *)rx_buf, "OK"))
            {
                /* --- Step 4: Enable new-SMS notification (AT+CNMI) ------- */
                index = 0; memset(rx_buf, '\0', 300);
                UART0_Str("AT+CNMI=2,1,0,0,0\r\n");
                while (index < 4);       /* CNMI response is slightly longer  */
                delay_ms(1100);
                rx_buf[index] = '\0';

                if (strstr((char *)rx_buf, "OK"))
                    Display(" GSM INIT SUCCESS ", 300);
                else
                    Display("  GSM INIT ERROR 4", 100);
            }
            else
                Display("  GSM INIT ERROR 3", 100);
        }
        else
            Display("  GSM INIT ERROR 2", 100);
    }
    else
        Display("  GSM INIT ERROR 1", 100);
}

/* -------------------------------------------------------------------------
 * send_sms
 * ------------------------------------------------------------------------- */

/**
 * @brief  Send an SMS message to the specified number.
 *
 * Uses the AT+CMGS command in text mode.  The message is composed of two
 * string parts (`info` and `msg`) concatenated on-the-fly; this allows a
 * fixed header string and a dynamic body to be sent without pre-formatting.
 *
 * The transmission is terminated with 0x1A (Ctrl+Z), as required by
 * the GSM AT command specification.
 *
 * @param  num   Destination phone number string (e.g., "9951826554").
 * @param  info  First part of the message body (e.g., error description).
 * @param  msg   Second part of the message body (e.g., sender's number).
 */
void send_sms(u8 *num, u8 *info, u8 *msg)
{
    /* Open the AT+CMGS command with the destination number                  */
    index = 0; memset(rx_buf, '\0', 300);
    UART0_Str("AT+CMGS=\"");
    UART0_Str(num);
    UART0_Str("\"\r\n");
    delay_ms(500);                       /* Wait for '>' prompt from modem    */

    /* Send the two-part message body                                         */
    index = 0;
    UART0_Str(info);
    UART0_Str(msg);

    /* Ctrl+Z (0x1A) signals end of SMS content to the GSM module            */
    UART0_Tx(0x1A);
    delay_ms(5000);                      /* Allow up to 5 s for SMS to send   */

    while (index < 3);                   /* Wait for OK/ERROR response        */
    rx_buf[index] = '\0';

    if (strstr((s8 *)rx_buf, "OK"))
    {
        /* SMS successfully sent — no additional action required              */
    }
}

/* -------------------------------------------------------------------------
 * delete_sms
 * ------------------------------------------------------------------------- */

/**
 * @brief  Delete the SMS stored at index 1 in the GSM module memory.
 *
 * Always deletes slot 1 (AT+CMGD=1) because the system reads and
 * processes SMS at index 1 immediately upon receipt, so only that
 * slot ever needs clearing.
 *
 * Called after every SMS has been processed (valid or invalid) to
 * prevent the GSM memory from filling up.
 */
void delete_sms(void)
{
    index = 0; memset(rx_buf, 0, 300);
    UART0_Str("AT+CMGD=1\r\n");
    delay_ms(1000);                      /* Wait for deletion to complete     */
    while (index < 3);
}

/* -------------------------------------------------------------------------
 * Read_sms
 * ------------------------------------------------------------------------- */

/**
 * @brief  Read the SMS stored at index 1 from the GSM module.
 *
 * Sends AT+CMGR=1 and waits for the full response.  If the response
 * contains "OK" (indicating a valid read), the raw buffer content
 * (including header and body) is copied to the caller-provided `sms`
 * buffer for parsing by verify_format() and related functions.
 *
 * @param  sms  Destination buffer (at least 300 bytes) for the raw
 *              AT+CMGR response string.
 */
void Read_sms(u8 *sms)
{
    index = 0;
    memset(rx_buf, '\0', 300);
    UART0_Str("AT+CMGR=1\r\n");
    while (index < 3);                   /* Wait for response bytes           */
    delay_s(2);                          /* Allow full message transfer       */
    rx_buf[index] = '\0';

    if (strstr((s8 *)rx_buf, "OK"))
        strcpy((char *)sms, (char *)rx_buf);  /* Copy raw response to caller */
}
