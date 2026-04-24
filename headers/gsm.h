/**
 * @file    gsm.h
 * @brief   Public API for the GSM module (M660A) driver (gsm.c).
 *
 * Provides functions for initialising the modem and sending, reading,
 * and deleting SMS messages via AT commands over UART0.
 */

#ifndef GSM_H
#define GSM_H

#include "types.h"

/**
 * @brief  Initialise the GSM module with the required AT command sequence.
 *
 * Sends: AT → ATE0 → AT+CMGF=1 → AT+CNMI=2,1,0,0,0
 * Displays GSM INIT SUCCESS or GSM INIT ERROR 1–4 on the dot-matrix.
 */
void gsm_init(void);

/**
 * @brief  Send an SMS to a specified number.
 *
 * The message body is composed of `info` followed immediately by `msg`.
 * Terminated with 0x1A (Ctrl+Z) as required by the GSM AT specification.
 *
 * @param  num   Destination phone number string (e.g., "9951826554").
 * @param  info  First portion of the SMS body text.
 * @param  msg   Second portion of the SMS body text (appended to info).
 */
void send_sms(u8 *num, u8 *info, u8 *msg);

/**
 * @brief  Delete the SMS at slot index 1 in the GSM module's memory.
 *
 * Called after every SMS is processed to keep the GSM memory free.
 */
void delete_sms(void);

/**
 * @brief  Read the SMS at slot index 1 from the GSM module.
 *
 * Issues AT+CMGR=1 and copies the full response (including header) into
 * the caller-supplied `sms` buffer for parsing by verify_format().
 *
 * @param  sms  Destination buffer (≥300 bytes) for the raw AT+CMGR response.
 */
void Read_sms(u8 *sms);

#endif /* GSM_H */
