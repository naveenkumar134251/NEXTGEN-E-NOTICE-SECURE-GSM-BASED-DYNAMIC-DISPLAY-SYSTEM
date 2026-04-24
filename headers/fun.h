/**
 * @file    fun.h
 * @brief   Public API for SMS format verification and content extraction (fun.c).
 *
 * These functions form the security and parsing layer of the E-Notice system.
 * All incoming SMS messages must pass through verify_format() before any
 * action is taken.  The extract_* functions are only called after a valid
 * command has been confirmed.
 */

#ifndef FUN_H
#define FUN_H

#include "types.h"

/**
 * @brief  Authenticate and classify an incoming SMS.
 *
 * @param  msg        Raw AT+CMGR response buffer.
 * @param  Author_num Authorized phone number stored in EEPROM.
 * @return  1 = valid display-update command (1212D...#)
 *          2 = valid number-update command  (1212M...#)
 *          3 = authorized sender, invalid format/security key
 *          4 = unauthorized sender number
 */
int verify_format(u8 *msg, u8 *Author_num);

/**
 * @brief  Extract the display message payload from a validated 'D' command SMS.
 * @param  msg     Raw SMS buffer containing the 1212D command.
 * @param  output  Destination buffer (≥200 bytes) for the extracted string.
 */
void extract_content(u8 *msg, u8 *output);

/**
 * @brief  Extract the new authorized mobile number from a validated 'M' command SMS.
 * @param  msg  Raw SMS buffer containing the 1212M command.
 * @param  num  Destination buffer (≥11 bytes) for the extracted phone number.
 */
void extract_num(u8 *msg, u8 *num);

/**
 * @brief  Extract the message body from an invalidly-formatted SMS for echo-back.
 * @param  msg          Raw AT+CMGR SMS buffer.
 * @param  invalid_msg  Destination buffer for the extracted body text.
 */
void Invalid_msg(u8 *msg, u8 *invalid_msg);

/**
 * @brief  Extract the unauthorized sender's number from an AT+CMGR response.
 * @param  msg          Raw AT+CMGR SMS buffer.
 * @param  invalid_msg  Destination buffer (≥11 bytes) for the phone number string.
 */
void Invalid_num(u8 *msg, u8 *invalid_msg);

#endif /* FUN_H */
