/**
 * @file    i2c_eeprom.h
 * @brief   Public API for the AT24C256 I²C EEPROM driver (i2c_eprom.c).
 *
 * Provides high-level string read/write access and low-level single-byte
 * operations for the 256 kbit (32 KB) AT24C256 EEPROM connected at
 * I²C slave address 0x50 (I2C_EEPROM_SA1).
 *
 * String storage convention:
 *   EEPROM_write() automatically appends a '#' sentinel after the string.
 *   EEPROM_Read() stops reading when it encounters '#', ensuring correct
 *   recovery without needing to store or know the string length.
 */

#ifndef I2C_EEPROM_H
#define I2C_EEPROM_H

#include "types.h"

/**
 * @brief  Write a null-terminated string to EEPROM followed by a '#' sentinel.
 *
 * @param  slaveAddr       7-bit I²C slave address (e.g., I2C_EEPROM_SA1 = 0x50).
 * @param  wBufStartAddr   16-bit EEPROM start address for the write.
 * @param  p               Pointer to the null-terminated source string.
 * @param  nBytes          Max bytes to write (loop stops at null terminator).
 */
void EEPROM_write(u8 slaveAddr,
                  u16 wBufStartAddr,
                  u8 *p,
                  u8 nBytes);

/**
 * @brief  Read a '#'-terminated string from EEPROM into a buffer.
 *
 * Performs a combined write (address set) + repeated-START read.
 * Stops at '#' sentinel or `nBytes`, whichever comes first.
 * The result is always null-terminated.
 *
 * @param  slaveAddr       7-bit I²C slave address.
 * @param  rBufStartAddr   16-bit EEPROM start address to read from.
 * @param  p               Destination buffer for the read string.
 * @param  nBytes          Maximum number of bytes to read.
 */
void EEPROM_Read(u8 slaveAddr,
                 u16 rBufStartAddr,
                 u8 *p,
                 u8 nBytes);

/**
 * @brief  Read a single byte from a random EEPROM address.
 *
 * Used to read flag bytes (EE_MSGI, EE_MOBI) that indicate whether
 * valid data has been previously saved to EEPROM.
 *
 * @param  slaveAddr  7-bit I²C slave address.
 * @param  rBufAddr   16-bit EEPROM address to read.
 * @return            Byte value at the specified EEPROM address.
 */
u8 i2c_eeprom_randomread(u8 slaveAddr, u8 rBufAddr);

/**
 * @brief  Write a single byte to a specific EEPROM address.
 *
 * Used to write flag bytes (e.g., '1' at EE_MSGI, '2' at EE_MOBI)
 * that mark whether valid message or number data has been stored.
 * Includes a 10 ms delay after STOP for the EEPROM write cycle.
 *
 * @param  slaveAddr  7-bit I²C slave address.
 * @param  wBufAddr   16-bit EEPROM target address.
 * @param  dat        Byte value to write.
 */
void i2c_eeprom_bytewrite(u8 slaveAddr, u8 wBufAddr, u8 dat);

#endif /* I2C_EEPROM_H */
