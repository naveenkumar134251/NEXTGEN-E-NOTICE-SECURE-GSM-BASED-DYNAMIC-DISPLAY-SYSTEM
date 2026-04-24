/**
 * @file    i2c.c
 * @brief   I²C master driver for LPC2148 (hardware I²C peripheral).
 *
 * Implements the fundamental I²C bus operations required to communicate
 * with the AT24C256 EEPROM:
 *   - Bus initialisation
 *   - START / Repeated-START / STOP conditions
 *   - Byte write with ACK polling
 *   - Byte read with Master-ACK (mack) or Master-NACK (nack)
 *
 * Clock speed and pin assignments are configured via i2c_defines.h.
 * At PCLK = 15 MHz and I2C_SPEED = 100 kHz, I2C_DIVIDER = 75 (0x4B).
 *
 * Pin mapping (set via PINSEL0):
 *   P0.2 → SCL  (SCL_0_2 = 0x10)
 *   P0.3 → SDA  (SDA_0_3 = 0x40)
 */

#include "types.h"
#include "i2c_defines.h"
#include <LPC21xx.h>

/* -------------------------------------------------------------------------
 * init_i2c
 * ------------------------------------------------------------------------- */

/**
 * @brief  Initialise the LPC2148 hardware I²C peripheral.
 *
 * 1. Configures P0.2 and P0.3 for I²C function via PINSEL0.
 * 2. Sets the SCL high/low period registers for 100 kHz operation.
 * 3. Enables the I²C interface by setting I2EN in I2CONSET.
 */
void init_i2c(void)
{
    /* Select I²C pin functions on P0.2 (SCL) and P0.3 (SDA)               */
    PINSEL0 |= SCL_0_2 | SDA_0_3;

    /* Configure clock dividers for 100 kHz I²C:
     * I2SCLH and I2SCLL each hold half the SCL period.                    */
    I2SCLH = I2C_DIVIDER;
    I2SCLL = I2C_DIVIDER;

    /* Enable the I²C interface                                             */
    I2CONSET = 1 << I2EN_BIT;
}

/* -------------------------------------------------------------------------
 * i2c_start
 * ------------------------------------------------------------------------- */

/**
 * @brief  Generate an I²C START condition.
 *
 * Sets the STA bit and waits for the SI (serial interrupt) flag to be
 * asserted by hardware, indicating the START has been transmitted.
 * Clears STA afterward to prevent repeated-START on the next transfer.
 */
void i2c_start(void)
{
    I2CONSET = 1 << STA_BIT;                             /* Request START        */
    while (((I2CONSET >> SI_BIT) & 1) == 0);             /* Wait for SI          */
    I2CONCLR = 1 << STAC_BIT;                            /* Clear STA flag       */
}

/* -------------------------------------------------------------------------
 * i2c_restart
 * ------------------------------------------------------------------------- */

/**
 * @brief  Generate an I²C Repeated-START condition.
 *
 * Used to switch from write mode (sending address + register) to read mode
 * without releasing the bus.  Sets STA, clears SI, waits for hardware
 * confirmation, then clears STA.
 */
void i2c_restart(void)
{
    I2CONSET = 1 << STA_BIT;                             /* Request repeated START */
    I2CONCLR = 1 << SIC_BIT;                             /* Clear SI to proceed    */
    while (((I2CONSET >> SI_BIT) & 1) == 0);             /* Wait for SI            */
    I2CONCLR = 1 << STAC_BIT;                            /* Clear STA              */
}

/* -------------------------------------------------------------------------
 * i2c_stop
 * ------------------------------------------------------------------------- */

/**
 * @brief  Generate an I²C STOP condition and release the bus.
 *
 * Sets the STO bit and clears SI simultaneously.  Hardware generates
 * the STOP condition on the bus.
 */
void i2c_stop(void)
{
    I2CONSET = 1 << STO_BIT;         /* Request STOP                       */
    I2CONCLR = 1 << SIC_BIT;         /* Clear SI to allow hardware to proceed */
}

/* -------------------------------------------------------------------------
 * i2c_write
 * ------------------------------------------------------------------------- */

/**
 * @brief  Write one byte to the I²C bus and wait for acknowledgement.
 *
 * Loads `dat` into the I²C data register, clears SI to trigger
 * transmission, then polls SI until the byte has been shifted out and
 * an ACK/NACK has been received from the slave.
 *
 * @param  dat  Byte to transmit (address byte, register byte, or data byte).
 */
void i2c_write(u8 dat)
{
    I2DAT = dat;                             /* Load byte to transmit       */
    I2CONCLR = 1 << SIC_BIT;                /* Clear SI to start transfer  */
    while (((I2CONSET >> SI_BIT) & 1) == 0); /* Wait for SI (transfer done) */
}

/* -------------------------------------------------------------------------
 * i2c_nack
 * ------------------------------------------------------------------------- */

/**
 * @brief  Read one byte from the I²C bus and send a Master NACK.
 *
 * A NACK after the last byte signals to the slave that the master will
 * not request further data.  After this call, i2c_stop() should be called.
 *
 * @return  The byte received from the slave.
 */
u8 i2c_nack(void)
{
    I2CONCLR = 1 << SIC_BIT;                  /* Clear SI — do NOT set AA   */
    while (((I2CONSET >> SI_BIT) & 1) == 0);  /* Wait for byte to be received */
    return I2DAT;                              /* Return the received byte    */
}

/* -------------------------------------------------------------------------
 * i2c_mack
 * ------------------------------------------------------------------------- */

/**
 * @brief  Read one byte from the I²C bus and send a Master ACK.
 *
 * Sets the AA (Assert Acknowledge) bit so the master ACKs the received
 * byte, indicating to the slave that more bytes are wanted.  Clears AA
 * after the byte is received so subsequent calls can choose ACK or NACK.
 *
 * @return  The byte received from the slave.
 */
u8 i2c_mack(void)
{
    I2CONSET = 1 << AA_BIT;                    /* Assert ACK after receive   */
    I2CONCLR = 1 << SIC_BIT;                   /* Clear SI to start receive  */
    while (((I2CONSET >> SI_BIT) & 1) == 0);   /* Wait for byte              */
    I2CONCLR = 1 << AA_BIT;                    /* De-assert AA               */
    return I2DAT;                               /* Return received byte       */
}
