/******************************************************************************
* Support library which adds dynamic USB Serial Number assignment, and MCU
* UUID fetch/conversion functiality
*
* (c) ADBeta 2026
******************************************************************************/
#include "serial_uuid.h"
#include "rv003usb/usb_config.h"

#include "stdint.h"


/*** rv003usb variables ******************************************************/
// Initialise the USB Serial string to all 0's to be assigned later
// Serial string is 8Chars (32bit Hex)
// NOTE: The Serial String is in UTF16-LE, and '0' is used as a safe value
// incase the value is not changed later
uint8_t usb_serial[USB_SERIAL_BYTES] = {
	USB_SERIAL_BYTES,    // bLength
	3,                   // bDescriptorType
	'0', 0x00,           // wString
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
	'0', 0x00,
};


/*** Static Variables / Functions ********************************************/
// Pointer to the first byte of the usable char of the USB Serial String
static const uint8_t *usb_serial_str_ptr = &usb_serial[2];

// Table of Hexadecimal Characters to be pushed to the Serial Number String
static char hex_char_table[16] = "0123456789ABCDEF";



/*** Public Functions ********************************************************/
void get_mcu_uuid(mcu_uuid_t id)
{
	// Cast the first ESIG Register to a pointer so we can copy it to the UUID
	volatile uint32_t *esig = (volatile uint32_t *) 0x1FFFF7E8;

	id[0] = esig[0];
	id[1] = esig[1];
	id[2] = esig[2];
}


uint32_t conv_uuid_u32(const mcu_uuid_t id)
{
	const uint32_t FNV_PRIME = 16777619U;

	uint32_t hash = 2166136261U;   // FNV-1a pre-given offset
	
	// Cast the UUID Bytes to a uint8_t array
	const uint8_t *bytes = (const uint8_t *)id;

	// Iterate over all bytes in the UUID and hash it
	for(uint8_t b = 0; b < 12; b++)
	{
		hash ^= bytes[b];   // XOR Byte in
		hash *= FNV_PRIME;  // Multiple by Prime
	}

	return hash;
}


void conv_u32_hex_str(const uint32_t val, u32_hex_str_t hex)
{
	// Convert the uint32 to an array of 8bit values
	const uint8_t *bytes = (const uint8_t *)&val;

	// Convert each byte into two nibble hex chars
	uint8_t chr = 0;
	for(int8_t b = 3; b >= 0; b--)
	{
		uint8_t u_nibble = bytes[b] >> 4;
		uint8_t l_nibble = bytes[b] & 0x0F;

		hex[chr++] = hex_char_table[u_nibble];
		hex[chr++] = hex_char_table[l_nibble];
	}

	hex[chr] = '\0';
}


void set_usb_serial_u32_hex(const u32_hex_str_t hex)
{
	uint8_t *ptr = (uint8_t *)usb_serial_str_ptr;

	for(uint8_t chr = 0; chr < 8; chr++)
	{
		*ptr++ = hex[chr];
		*ptr++ = 0x00;
	}
}





void set_usb_serial_uuid(void)
{
	mcu_uuid_t     uuid;
	u32_hex_str_t  hex_str;

	get_mcu_uuid(uuid);
	conv_u32_hex_str(conv_uuid_u32(uuid), hex_str);

	set_usb_serial_u32_hex(hex_str);
}
