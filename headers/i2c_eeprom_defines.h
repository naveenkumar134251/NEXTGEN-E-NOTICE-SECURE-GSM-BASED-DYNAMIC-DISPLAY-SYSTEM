/**
 * @file    i2c_eeprom_defines.h
 * @brief   I²C slave addresses and EEPROM memory map for the AT24C256.
 *
 * The AT24C256 exposes up to 8 different I²C addresses (0x50–0x57)
 * selectable via the A0–A2 hardware address pins.  This project uses
 * SA1 (0x50) with all address pins tied low.
 *
 * EEPROM Memory Map
 * -----------------
 * Addr 0x00 (EE_MOBI)  — Authorized number presence flag
 *                         '2' = a valid number has been stored; anything
 *                         else (e.g., 0xFF after erase) = use default.
 *
 * Addr 0x01 (EE_MOB)   — Authorized mobile number string (10 ASCII digits
 *                         + '#' sentinel written by EEPROM_write).
 *
 * Addr 0x14 (EE_MSGI)  — Display message presence flag
 *                         '1' = a valid message has been stored.
 *
 * Addr 0x16 (EE_MSG)   — Display message string (variable length,
 *                         terminated by '#' sentinel in EEPROM).
 *
 * The two-byte gap between EE_MOBI (0x00) and EE_MSGI (0x14) provides
 * safe separation so a long phone number cannot overwrite the message flag.
 */

#ifndef I2C_EEPROM_DEFINES_H
#define I2C_EEPROM_DEFINES_H

/* --- AT24C256 I²C slave addresses (7-bit, A2:A1:A0 = 0b000 to 0b111) --- */
#define I2C_EEPROM_SA1   0x50   /**< A2=0, A1=0, A0=0 — default address used in project */
#define I2C_EEPROM_SA2   0x51   /**< A2=0, A1=0, A0=1                                   */
#define I2C_EEPROM_SA3   0x52   /**< A2=0, A1=1, A0=0                                   */
#define I2C_EEPROM_SA4   0x53   /**< A2=0, A1=1, A0=1                                   */
#define I2C_EEPROM_SA5   0x54   /**< A2=1, A1=0, A0=0                                   */
#define I2C_EEPROM_SA6   0x55   /**< A2=1, A1=0, A0=1                                   */
#define I2C_EEPROM_SA7   0x56   /**< A2=1, A1=1, A0=0                                   */
#define I2C_EEPROM_SA8   0x57   /**< A2=1, A1=1, A0=1                                   */

/* --- EEPROM memory map offsets ------------------------------------------ */
#define EE_MOBI   0x00   /**< Flag byte: '2' = valid auth number stored      */
#define EE_MOB    0x01   /**< Auth mobile number string start address        */
#define EE_MSGI   0x14   /**< Flag byte: '1' = valid display message stored  */
#define EE_MSG    0x16   /**< Display message string start address           */

#endif /* I2C_EEPROM_DEFINES_H */
