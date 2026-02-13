/******************************************************************************
* Support library which adds dynamic USB Serial Number assignment, and MCU
* UUID fetch/conversion functiality
*
* (c) ADBeta 2026
******************************************************************************/
#ifndef INSOMNIAC_UUID_H
#define INSOMNIAC_UUID_H

#include "stdint.h"

/*** Typedefs and Enums ******************************************************/
/// @brief MCU UUID Type - 3 32bit words or 96bits 
typedef uint32_t mcu_uuid_t[3];

/// @brief 32bit hexadecimal string type - 8 Chars NULL Terminated
typedef char u32_hex_str_t[9];



/*** Function Declarations ***************************************************/
/// @brief Gets the 12 Byte UUID from the Microcontroller ESIG Reg
/// @param mcu_uuid_t uint32_t[3] array to modify
/// @return None
void get_mcu_uuid(mcu_uuid_t id);


/// @brief Converts the 12 Byte (96 bit) UUID into a FNV-1a 32bit value
/// @param mcu_uuid_t to read and convert
/// @return uint32 hashed UUID value
uint32_t conv_uuid_u32(const mcu_uuid_t id);


/// @brief Converts the given 32bit number to a 8 Char hexadecimal string
/// (NULL Terminated)
/// @param uint32_t value to convert
/// @param u32_hex_str_t string to populate
/// @return None
void conv_u32_hex_str(const uint32_t val, u32_hex_str_t hex);


/// @brief Sets the USB Serial String to the given u32_hex string
/// Converts from UTF8 to UTF16-LE
/// @param u32_hex_str_t string to populate into the Serial String
/// @return None
void set_usb_serial_u32_hex(const u32_hex_str_t hex);


/// @brief Does all the steps above in one neat function to assist the user
/// @param None
/// @return None
void set_usb_serial_uuid(void);

#endif
