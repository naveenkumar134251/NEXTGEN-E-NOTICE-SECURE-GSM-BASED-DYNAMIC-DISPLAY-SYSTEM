/**
 * @file    i2c.h
 * @brief   Public API for the LPC2148 hardware I²C master driver (i2c.c).
 *
 * Exposes the fundamental I²C bus operations needed to communicate with
 * the AT24C256 EEPROM.  All functions operate synchronously (polling SI flag).
 *
 * Typical usage for a write transaction:
 *   i2c_start() → i2c_write(SLA+W) → i2c_write(addrH) → i2c_write(addrL)
 *   → i2c_write(data) → i2c_stop()
 *
 * Typical usage for a read transaction:
 *   i2c_start() → i2c_write(SLA+W) → i2c_write(addrH) → i2c_write(addrL)
 *   → i2c_restart() → i2c_write(SLA+R)
 *   → i2c_mack() [for each byte except last] → i2c_nack() → i2c_stop()
 */

#ifndef I2C_H
#define I2C_H

#include "types.h"

/** @brief  Initialise the I²C peripheral at 100 kHz on P0.2 (SCL) / P0.3 (SDA). */
void init_i2c(void);

/** @brief  Generate an I²C START condition and wait for hardware confirmation. */
void i2c_start(void);

/** @brief  Generate an I²C Repeated-START (for read-after-write without bus release). */
void i2c_restart(void);

/** @brief  Generate an I²C STOP condition and release the bus. */
void i2c_stop(void);

/**
 * @brief  Write one byte to the bus and wait for ACK/NACK.
 * @param  dat  Byte to transmit (address, register, or data byte).
 */
void i2c_write(u8 dat);

/**
 * @brief  Read one byte from the bus and send a Master NACK (last byte of transfer).
 * @return  Received byte.
 */
u8 i2c_nack(void);

/**
 * @brief  Read one byte from the bus and send a Master ACK (more bytes to follow).
 * @return  Received byte.
 */
u8 i2c_mack(void);

#endif /* I2C_H */
