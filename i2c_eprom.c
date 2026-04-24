/**
 * @file    i2c_eprom.c
 * @brief   AT24C256 I²C EEPROM read/write driver for LPC2148.
 *
 * Provides high-level string read/write functions for the 256 kbit (32 KB)
 * AT24C256 EEPROM connected over the I²C bus.  Also exposes a low-level
 * single-byte write and a random-address single-byte read.
 *
 * The AT24C256 uses a 16-bit (2-byte) word address, transmitted MSB-first
 * after the device's I²C slave address.
 *
 * Write timing: The EEPROM requires up to 10 ms for each byte-write cycle
 * to complete internally.  All write operations therefore include a
 * delay_ms(10) after issuing the STOP condition.
 *
 * String termination convention used in this project:
 *   EEPROM_write() appends a '#' sentinel after the last character so that
 *   EEPROM_Read() can stop reading without needing to know the length.
 */

#include "i2c.h"
#include "delay.h"

/* Forward declaration of the static helper used internally                  */
void i2c_eeprom_bytewrite(u8 slaveAddr, u16 wBufAddr, u8 dat);

/* -------------------------------------------------------------------------
 * EEPROM_write
 * ------------------------------------------------------------------------- */

/**
 * @brief  Write a null-terminated string to the EEPROM, followed by '#'.
 *
 * Iterates over the source string `p` byte-by-byte, writing each character
 * using i2c_eeprom_bytewrite() at consecutive EEPROM addresses starting
 * from `wBufStartAddr`.  A '#' sentinel is written immediately after the
 * last character so that EEPROM_Read() knows where the string ends.
 *
 * @param  slaveAddr      7-bit I²C slave address of the EEPROM (e.g., 0x50).
 * @param  wBufStartAddr  16-bit EEPROM start address for the write operation.
 * @param  p              Pointer to the null-terminated source string.
 * @param  nBytes         Maximum bytes to write (not used; loop stops at '\0').
 */
void EEPROM_write(u8 slaveAddr, u16 wBufStartAddr, u8 *p, u8 nBytes)
{
    int i = 0;

    /* Write each character of the string until the null terminator          */
    for (i = 0; p[i]; i++)
    {
        i2c_eeprom_bytewrite(slaveAddr, wBufStartAddr++, p[i]);
    }

    /* Append '#' as an end-of-string sentinel in EEPROM                    */
    i2c_eeprom_bytewrite(slaveAddr, wBufStartAddr++, '#');
}

/* -------------------------------------------------------------------------
 * EEPROM_Read
 * ------------------------------------------------------------------------- */

/**
 * @brief  Read a '#'-terminated string from the EEPROM.
 *
 * Performs a combined write (to set the address pointer) followed by a
 * repeated-START read.  Reads up to `nBytes` bytes using Master ACK for
 * all bytes except the last, stopping early if the '#' sentinel is found.
 * The buffer is null-terminated after the last read byte.
 *
 * I²C transaction sequence:
 *   START → SLA+W → AddrHigh → AddrLow → RESTART → SLA+R
 *   → [mack × n-1] → [nack on '#' or last byte] → STOP
 *
 * @param  slaveAddr      7-bit I²C slave address (e.g., 0x50).
 * @param  rBufStartAddr  16-bit EEPROM start address to read from.
 * @param  p              Destination buffer for the read string.
 * @param  nBytes         Maximum number of bytes to read.
 */
void EEPROM_Read(u8 slaveAddr, u16 rBufStartAddr, u8 *p, u8 nBytes)
{
    u8 i;

    /* Set EEPROM internal address pointer (write-mode transaction)          */
    i2c_start();
    i2c_write(slaveAddr << 1);          /* SLA+W (write bit = 0)             */
    i2c_write(rBufStartAddr >> 8);      /* High byte of 16-bit word address  */
    i2c_write(rBufStartAddr);           /* Low byte of word address          */

    /* Switch bus to read mode with a repeated-START                         */
    i2c_restart();
    i2c_write(slaveAddr << 1 | 1);      /* SLA+R (read bit = 1)              */

    /* Read bytes one at a time, sending Master ACK to request the next byte */
    for (i = 0; i < nBytes; i++)
    {
        p[i] = i2c_mack();              /* Read byte, send ACK               */

        /* Stop reading when the '#' sentinel is encountered                 */
        if (p[i] == '#')
            break;
    }

    p[i] = '\0';                        /* Null-terminate the output string  */

    i2c_nack();                         /* Send final NACK to signal end     */
    i2c_stop();                         /* Release the I²C bus               */
}

/* -------------------------------------------------------------------------
 * i2c_eeprom_bytewrite  (internal helper)
 * ------------------------------------------------------------------------- */

/**
 * @brief  Write a single byte to a specific EEPROM address.
 *
 * Sends a complete I²C write transaction: START → SLA+W → AddrHigh
 * → AddrLow → Data → STOP.  After the STOP, waits 10 ms for the
 * EEPROM's internal write cycle to complete before returning.
 *
 * @param  slaveAddr  7-bit I²C slave address.
 * @param  wBufAddr   16-bit target EEPROM word address.
 * @param  dat        Data byte to write.
 */
void i2c_eeprom_bytewrite(u8 slaveAddr, u16 wBufAddr, u8 dat)
{
    i2c_start();
    i2c_write(slaveAddr << 1);          /* SLA+W                             */
    i2c_write(wBufAddr >> 8);           /* High address byte                 */
    i2c_write(wBufAddr);               /* Low address byte                  */
    i2c_write(dat);                     /* Data byte                         */
    i2c_stop();
    delay_ms(10);                       /* EEPROM internal write cycle delay */
}

/* -------------------------------------------------------------------------
 * i2c_eeprom_randomread
 * ------------------------------------------------------------------------- */

/**
 * @brief  Read a single byte from a specific EEPROM address (random read).
 *
 * Performs a "dummy write" to set the EEPROM's internal address counter,
 * then issues a repeated-START to switch to read mode and clocks out
 * one byte with a NACK to terminate the transfer.
 *
 * Typically used to read flag bytes (e.g., EE_MSGI, EE_MOBI) that indicate
 * whether valid data has been stored in EEPROM.
 *
 * @param  slaveAddr  7-bit I²C slave address.
 * @param  rBufAddr   16-bit EEPROM address to read from.
 * @return            The byte read from the specified EEPROM address.
 */
u8 i2c_eeprom_randomread(u8 slaveAddr, u8 rBufAddr)
{
    u8 dat;

    /* Dummy write to set the internal address pointer                       */
    i2c_start();
    i2c_write(slaveAddr << 1);          /* SLA+W                             */
    i2c_write(rBufAddr >> 8);           /* High address byte                 */
    i2c_write(rBufAddr);               /* Low address byte                  */

    /* Repeated-START to switch to read mode                                 */
    i2c_restart();
    i2c_write(slaveAddr << 1 | 1);      /* SLA+R                             */

    dat = i2c_nack();                   /* Read byte with NACK (last byte)   */
    i2c_stop();

    return dat;
}
