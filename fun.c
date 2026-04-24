/**
 * @file    fun.c
 * @brief   SMS message format verification and content extraction utilities.
 *
 * This module implements the security and parsing layer of the NextGen
 * E-Notice system.  All incoming SMS messages are passed through
 * verify_format() first.  Only messages that carry the correct security
 * code ("1212"), a valid command character ('D' or 'M'), and the '#'
 * terminator — AND originate from the stored authorized number — result
 * in a display or configuration update.
 *
 * SMS format expected by this module:
 *   Display update : <auth_number_in_body>...1212D<message>#
 *   Number update  : <auth_number_in_body>...1212M<new_number>#
 */

#include <string.h>
#include "fun.h"

/* -------------------------------------------------------------------------
 * verify_format
 * ------------------------------------------------------------------------- */

/**
 * @brief  Authenticate and classify an incoming SMS message.
 *
 * Checks the raw SMS string (as returned by AT+CMGR) for:
 *   1. The authorized sender's number somewhere in the message header.
 *   2. The security key + command code ("1212D" or "1212M").
 *   3. The '#' end-of-content terminator.
 *
 * @param  msg         Raw SMS buffer (full AT+CMGR response).
 * @param  Author_num  Authorized phone number string stored in EEPROM.
 *
 * @return  1  Valid display-update command  (1212D...#)
 *          2  Valid number-update command   (1212M...#)
 *          3  Authorized sender but invalid format / security code
 *          4  Unauthorized sender (number not in header)
 */
int verify_format(u8 *msg, u8 *Author_num)
{
    /* Check if the authorized number appears anywhere in the SMS header */
    if (strstr((s8 *)msg, (s8 *)Author_num))
    {
        /* Authorized sender: check for display-update command              */
        if ((strstr((s8 *)msg, "1212D")) && (strstr((s8 *)msg, "#")))
        {
            return 1;   /* Valid display update */
        }
        /* Authorized sender: check for number-update command               */
        else if ((strstr((s8 *)msg, "1212M")) && (strstr((s8 *)msg, "#")))
        {
            return 2;   /* Valid number update */
        }
        else
        {
            return 3;   /* Authorized sender but wrong format / security key */
        }
    }
    else
    {
        return 4;       /* Unauthorized sender number */
    }
}

/* -------------------------------------------------------------------------
 * extract_content
 * ------------------------------------------------------------------------- */

/**
 * @brief  Extract the display message payload from a validated SMS.
 *
 * Locates the "1212D" token inside the SMS buffer, then copies the
 * characters that follow it (starting at offset 3 relative to "1212D",
 * skipping the first two payload bytes with spaces for display padding)
 * into `output` until the '#' terminator is reached.
 *
 * Two trailing spaces are appended so that the scrolling display has a
 * clean gap before the message wraps.
 *
 * @param  msg1    Raw SMS buffer containing the 1212D command.
 * @param  output  Destination buffer (at least 200 bytes) for the
 *                 extracted, null-terminated display string.
 */
void extract_content(u8 *msg1, u8 *output)
{
    u32 i, j = 0;
    s8 *msg = NULL;

    memset(output, '\0', 200);           /* Clear the output buffer */

    /* Locate the start of the command token                                 */
    msg = strstr((s8 *)msg1, "1212D");

    /* Copy characters after the token; prepend spaces for the first 2 chars
     * (i < 5) to provide a visual lead-in before the actual message text.   */
    for (i = 3; msg[i] != '#'; i++)
    {
        if (i < 5)
            output[j++] = ' ';          /* Lead-in space padding             */
        else
            output[j++] = msg[i];       /* Actual message character          */
    }

    /* Append trailing spaces for display scroll gap                         */
    output[j++] = ' ';
    output[j++] = ' ';
    output[j++] = ' ';
    output[j]   = '\0';
}

/* -------------------------------------------------------------------------
 * extract_num
 * ------------------------------------------------------------------------- */

/**
 * @brief  Extract a new authorized mobile number from a validated SMS.
 *
 * Locates the "1212M" token and copies the digits that follow it into
 * `num` until the '#' terminator.  The result is null-terminated.
 *
 * @param  msg1  Raw SMS buffer containing the 1212M command.
 * @param  num   Destination buffer (at least 11 bytes) for the extracted
 *               phone number string.
 */
void extract_num(u8 *msg1, u8 *num)
{
    u32 i, j = 0;
    s8 *msg = (s8 *)msg1;

    /* Jump to the "1212M" token                                             */
    msg = strstr((s8 *)msg1, "1212M");

    /* Skip the 5-character token ("1212M") and copy the number digits      */
    for (i = 5; msg[i] != '#'; i++)
    {
        num[j++] = msg[i];
    }
    num[j] = '\0';
}

/* -------------------------------------------------------------------------
 * Invalid_msg
 * ------------------------------------------------------------------------- */

/**
 * @brief  Extract the original message body from an invalidly-formatted SMS.
 *
 * The raw AT+CMGR response contains multiple newline-separated fields
 * (status line, header line, message body).  This function skips the
 * first two newlines then copies the message body up to the carriage-return
 * (0x0D) terminator into `invalid_msg`.
 *
 * Used to echo back what the sender actually sent when reporting a
 * format error via reply SMS.
 *
 * @param  msg          Raw AT+CMGR SMS buffer.
 * @param  invalid_msg  Destination buffer for the extracted body text.
 */
void Invalid_msg(u8 *msg, u8 *invalid_msg)
{
    u32 newline = 0, i = 0, j = 0;
    s8 *ptr = NULL;

    ptr = (s8 *)msg;

    /* Skip the first two newline characters to reach the message body      */
    while (1)
    {
        ptr = strchr(ptr, '\n');
        ptr++;
        newline++;
        if (newline == 2)
            break;
    }

    /* Copy from body start up to the carriage-return character              */
    i = ptr - (s8 *)msg;
    for (; msg[i] != 0x0D; i++)
    {
        invalid_msg[j++] = msg[i];
    }
    invalid_msg[j] = '\0';
}

/* -------------------------------------------------------------------------
 * Invalid_num
 * ------------------------------------------------------------------------- */

/**
 * @brief  Extract the unauthorized sender's number from a +CMTI/CMGR response.
 *
 * Locates the "UNREAD" keyword in the AT+CMGR response (which appears in
 * the status field) then reads 10 characters starting 11 bytes after it —
 * the position where the sender's phone number resides in the header.
 *
 * Used to include the unauthorized number in the alert SMS sent to the
 * owner.
 *
 * @param  msg          Raw AT+CMGR SMS buffer.
 * @param  invalid_msg  Destination buffer (at least 11 bytes) for the
 *                      extracted phone number string.
 */
void Invalid_num(u8 *msg, u8 *invalid_msg)
{
    s8 *ptr = NULL;
    u32 i;

    ptr = (s8 *)msg;

    /* Find "UNREAD" in the response, then offset to the sender number field */
    ptr = strstr((s8 *)msg, "UNREAD");
    ptr = ptr + 11;                      /* Sender number starts 11 bytes after */

    /* Copy exactly 10 digits (standard mobile number length)               */
    for (i = 0; i < 10; i++)
    {
        invalid_msg[i] = ptr[i];
    }
    invalid_msg[i] = '\0';
}
