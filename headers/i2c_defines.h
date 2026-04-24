/**
 * @file    i2c_defines.h
 * @brief   I²C peripheral configuration constants for LPC2148.
 *
 * Contains pin-function select values, clock divider calculation, and
 * bit-position definitions for the I²C control registers (I2CONSET / I2CONCLR).
 *
 * Clock configuration:
 *   FOSC  = 12 MHz  (crystal oscillator)
 *   CCLK  = 60 MHz  (5× PLL multiplier)
 *   PCLK  = 15 MHz  (CCLK / 4 via APB divider)
 *   I2C_SPEED = 100 kHz (standard mode)
 *   I2C_DIVIDER = PCLK / (I2C_SPEED × 2) = 75
 *     → Written to both I2SCLH and I2SCLL for a symmetric 50% duty cycle.
 */

#ifndef I2C_DEFINES_H
#define I2C_DEFINES_H

/* --- PINSEL0 values to enable I²C pin functions ------------------------- */
#define SCL_0_2   0x00000010   /**< PINSEL0 bits [5:4] = 01 → P0.2 = SCL0  */
#define SDA_0_3   0x00000040   /**< PINSEL0 bits [7:6] = 01 → P0.3 = SDA0  */

/* --- Clock configuration ------------------------------------------------ */
#define FOSC        12000000UL          /**< Crystal oscillator frequency (Hz) */
#define CCLK        (5 * FOSC)          /**< CPU clock = 60 MHz (PLL ×5)       */
#define PCLK        (CCLK / 4)          /**< Peripheral clock = 15 MHz          */
#define I2C_SPEED   100000UL            /**< Target I²C bus speed = 100 kHz     */

/** SCL high/low period register value (written to I2SCLH and I2SCLL). */
#define I2C_DIVIDER   ((PCLK / I2C_SPEED) / 2)

/* --- I2CONSET bit positions ---------------------------------------------- */
#define   AA_BIT   2    /**< Assert Acknowledge — set to ACK received bytes   */
#define   SI_BIT   3    /**< Serial Interrupt flag — set when transfer done   */
#define  STO_BIT   4    /**< STOP flag — set to generate STOP condition       */
#define  STA_BIT   5    /**< START flag — set to generate START condition     */
#define I2EN_BIT   6    /**< I²C Interface Enable                             */

/* --- I2CONCLR bit positions (write 1 to clear corresponding I2CONSET bit) */
#define   AAC_BIT   2   /**< Clear AA bit                                     */
#define   SIC_BIT   3   /**< Clear SI flag (required to advance state machine)*/
#define  STAC_BIT   5   /**< Clear STA bit after START has been generated     */
#define I2ENC_BIT   6   /**< Clear I2EN (disable I²C interface)               */

#endif /* I2C_DEFINES_H */
