/**
 * @file    main.c
 * @brief   Application entry point for the NextGen E-Notice system.
 *
 * Orchestrates the complete system:
 *   1. Initialises all peripherals (UART0, I²C, dot-matrix rows, GSM module).
 *   2. Reads back any previously stored message and authorized number from EEPROM.
 *   3. Enters the main loop, which:
 *        a. Continuously scrolls the stored message on the dot-matrix display.
 *        b. Detects incoming SMS notifications (sms_flag set by UART ISR).
 *        c. Reads and verifies the SMS against the authorized number and format.
 *        d. Performs the appropriate action (update message, update auth number,
 *           or send error/alert reply) and deletes the processed SMS.
 *
 * SMS command dispatch (see fun.c / verify_format()):
 *   Case 1 (D command) → Extract and display new message, store in EEPROM.
 *   Case 2 (M command) → Update authorized mobile number in EEPROM.
 *   Case 3 (bad format, auth sender) → Reply with format-error SMS.
 *   Case 4 (unauthorized sender)     → Reply with alert SMS to owner.
 */

#include <string.h>
#include "uart.h"
#include "types.h"
#include "delay.h"
#include "i2c_eeprom.h"
#include "gsm.h"
#include "fun.h"
#include "i2c_eeprom_defines.h"
#include "dotmatrix.h"
#include "i2c.h"

/* Working buffers -----------------------------------------------------------*/
u8 Invalid[200] = "";                    /* Holds sender number or msg body for error reply  */
u8 stored_msg[200] = "   WAITING FOR YOUR MESSAGE  "; /* Default display message             */
u8 new_msg[300];                         /* Raw AT+CMGR SMS response buffer                  */

/* Default authorized number (overwritten from EEPROM if previously saved)   */
u8 auth_number[] = "9951826554";

/* sms_flag: set to 1 by UART ISR when "+CMTI" notification is received.
 * Signals the main loop that a new SMS is waiting in the GSM module.        */
extern u8 sms_flag;

/* index: tracks how many bytes have been received into rx_buf by the ISR.  */
extern u32 index;

/* -------------------------------------------------------------------------
 * init_sym
 * ------------------------------------------------------------------------- */

/**
 * @brief  Initialise all system peripherals.
 *
 * Called once at startup before entering the main loop.
 * Order matters: UART0 must be ready before gsm_init() can communicate.
 */
void init_sym()
{
    UART0_Init();       /* UART0 @ 9600 baud — required for GSM communication */
    init_i2c();         /* I²C bus @ 100 kHz — required for EEPROM access      */
    Init_dm_rows();     /* GPIO for dot-matrix row/column control              */
    gsm_init();         /* Send AT command sequence; display result on matrix  */
}

/* -------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------- */

/**
 * @brief  Application entry point — never returns.
 *
 * Flow:
 *   - Initialise peripherals.
 *   - If a valid message was previously saved in EEPROM (flag byte == '1'),
 *     restore it into stored_msg.
 *   - If a valid mobile number was previously saved (flag byte == '2'),
 *     restore it into auth_number.
 *   - Loop forever:
 *       1. Scroll stored_msg until an SMS interrupts (sms_flag == 1).
 *       2. Read the SMS from GSM module slot 1.
 *       3. Classify and process the SMS.
 *       4. Delete the SMS and resume scrolling.
 */
int main(void)
{
    s32 N_Bytes = 100;                   /* Max bytes to read/write for the message */

    init_sym();

    /* --- Restore persisted message from EEPROM (if previously stored) --- */
    /* EE_MSGI holds the flag byte; '1' means a valid message is present.   */
    if ((i2c_eeprom_randomread(I2C_EEPROM_SA1, EE_MSGI)) == '1')
        EEPROM_Read(I2C_EEPROM_SA1, EE_MSG, stored_msg, N_Bytes);

    /* --- Restore persisted authorized number from EEPROM (if stored) ---- */
    /* EE_MOBI holds the flag byte; '2' means a valid number is present.    */
    if ((i2c_eeprom_randomread(I2C_EEPROM_SA1, EE_MOBI)) == '2')
        EEPROM_Read(I2C_EEPROM_SA1, EE_MOB, auth_number, 11);

    /* =====================================================================
     * MAIN LOOP
     * ===================================================================== */
    while (1)
    {
        /* Reset UART receive buffer and index for the next SMS              */
        index = 0;
        memset(rx_buf, '\0', 300);

        /* --- Scroll stored_msg until sms_flag is set by the UART ISR ---- */
        do
        {
            Display(stored_msg, 500);
        } while (sms_flag == 0);

        /* New SMS detected — clear flag and process the message            */
        sms_flag = 0;
        Read_sms(new_msg);               /* Read AT+CMGR=1 into new_msg      */

        /* --- Classify and dispatch the received SMS ---------------------- */
        switch (verify_format(new_msg, auth_number))
        {
            case 1:
                /* Valid display-update command: 1212D<message>#            */
                extract_content(new_msg, stored_msg);          /* Parse msg  */
                N_Bytes = mystrlen(stored_msg);
                /* Write message-present flag, then the message itself       */
                i2c_eeprom_bytewrite(I2C_EEPROM_SA1, EE_MSGI, '1');
                EEPROM_write(I2C_EEPROM_SA1, EE_MSG, stored_msg, N_Bytes);
                delete_sms();
                break;

            case 2:
                /* Valid number-update command: 1212M<number>#              */
                extract_num(new_msg, auth_number);             /* Parse num  */
                /* Write number-present flag, then the new phone number      */
                i2c_eeprom_bytewrite(I2C_EEPROM_SA1, EE_MOBI, '2');
                EEPROM_write(I2C_EEPROM_SA1, EE_MOB, auth_number, 11);
                delete_sms();
                break;

            case 3:
                /* Authorized sender but wrong security key or bad syntax   */
                Invalid_msg(new_msg, Invalid);
                send_sms(auth_number,
                         "Not a valid format",
                         "\r\ncorrect format is 1212Dmsg#");
                delete_sms();
                break;

            case 4:
                /* Unauthorized sender — alert owner with the sender's number */
                Invalid_num(new_msg, Invalid);
                send_sms(auth_number,
                         "Invalid access number\r\n",
                         Invalid);
                delete_sms();
                break;
        }
    } /* end while(1) */
}
