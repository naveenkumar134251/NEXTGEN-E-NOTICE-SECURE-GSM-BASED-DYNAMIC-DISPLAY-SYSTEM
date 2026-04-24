/**
 * @file    security.h
 * @brief   Security and SMS parsing function prototypes (mirrors fun.h).
 *
 * This header duplicates the declarations from fun.h and is provided as
 * an alias entry point for modules that reference the security layer by
 * a different header name.  Both security.h and fun.h declare the same
 * set of functions, which are defined in fun.c.
 *
 * In a production refactor, this file could be removed and all includes
 * updated to use fun.h directly.
 *
 * Functions:
 *   verify_format()   — Authenticate sender and classify command type.
 *   extract_content() — Parse display message from a valid 'D' SMS.
 *   extract_num()     — Parse new mobile number from a valid 'M' SMS.
 *   Invalid_msg()     — Extract message body for error-reply echo.
 *   Invalid_num()     — Extract unauthorized sender number for alert SMS.
 */

#ifndef SECURITY_H
#define SECURITY_H

#include "types.h"

/**
 * @brief  Authenticate and classify an incoming SMS.
 * @return  1=display update, 2=number update, 3=bad format, 4=unauthorized sender.
 */
int verify_format(u8 *msg, u8 *Author_num);

/** @brief  Extract the display message payload from a '1212D' command SMS. */
void extract_content(u8 *msg, u8 *output);

/** @brief  Extract the new phone number from a '1212M' command SMS. */
void extract_num(u8 *msg, u8 *num);

/** @brief  Extract the message body from an invalid-format SMS for echo-back. */
void Invalid_msg(u8 *msg, u8 *invalid_msg);

/** @brief  Extract the unauthorized sender's number from an AT+CMGR response. */
void Invalid_num(u8 *msg, u8 *invalid_msg);

#endif /* SECURITY_H */
