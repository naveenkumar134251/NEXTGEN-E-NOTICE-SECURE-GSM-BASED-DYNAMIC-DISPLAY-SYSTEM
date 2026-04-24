/**
 * @file    types.h
 * @brief   Portable fixed-width type aliases for the NextGen E-Notice project.
 *
 * Defines short, consistent type names used throughout all source files.
 * These aliases map to the natural C types for the LPC2148 (ARM7TDMI-S),
 * where int is 32 bits and short is 16 bits.
 *
 * Using these aliases makes the code portable and avoids relying on
 * implicit integer sizes which can vary across compilers and architectures.
 */

#ifndef TYPES_H
#define TYPES_H

typedef unsigned char       u8;   /**< Unsigned  8-bit integer (0 to 255)          */
typedef char                s8;   /**< Signed    8-bit integer (-128 to 127)        */
typedef unsigned int        u32;  /**< Unsigned 32-bit integer (0 to 4,294,967,295) */
typedef unsigned short int  u16;  /**< Unsigned 16-bit integer (0 to 65,535)        */
typedef signed int          s32;  /**< Signed   32-bit integer                      */
typedef float               f32;  /**< 32-bit IEEE 754 single-precision float       */
typedef double              f64;  /**< 64-bit IEEE 754 double-precision float       */

/**
 * @brief  Custom strlen for u8 (unsigned char) strings.
 *
 * Declared here (defined in dot_matrix.c) so any module that includes
 * types.h can call mystrlen() without a separate header dependency.
 *
 * @param  mstr  Pointer to a null-terminated u8 string.
 * @return       Length of the string (number of characters before '\0').
 */
u32 mystrlen(u8 *mstr);

#endif /* TYPES_H */
