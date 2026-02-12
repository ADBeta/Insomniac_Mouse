/******************************************************************************
* Insomiac Mouse Emulator
* Connects over USB as a generic HID Mouse, then moves continuously to keep
* PCs awake, or just to waste some time watching it dance!
* See the GitHub: https://github.com/ADBeta/Insomniac_Mouse
*
* Pinout:
* 	USB+   PC1
* 	USB-   PC2
* 	JP1    PA2
* 	JP2    PA1
* 	JP3    PC4
*
* ADBeta (c) 2025-2026    12 Feb 2026    Ver1.4.0
******************************************************************************/
#include "ch32v003fun.h"
#include "rv003usb.h"
#include "lib_rand.h"
//#include <stdio.h>          // NOTE: Comment out when net debugging


/*** Types and Definitions ***************************************************/
/// @brief A C-ordinate position struct, X/Y Data can be positive or negative
typedef struct {
	int16_t          x;
	int16_t          y;
} position_t;


/// @brief Remapping of a uint8_t to movement instruction, 2 nibbles
/// [Up    /   Down]  [Left  /  Right]
///  1100      0011    1100      0011
typedef uint8_t mouse_instr_t;
#define MOUSE_INSTR_U      0b11000000
#define MOUSE_INSTR_D      0b00110000
#define MOUSE_INSTR_L      0b00001100
#define MOUSE_INSTR_R      0b00000011


typedef enum {
	MI_BUFFER_OK             = 0,
	MI_BUFFER_NO_SPACE,             // No Space to append to buffer
	MI_BUFFER_NO_DATA               // No Data to read from the buffer
} mi_buffer_status_t;


// User Mode Selection from the Jumpers - Reads the jumpers in binary on boot
typedef enum {
	USER_MODE_NORMAL     = 0b000,
	USER_MODE_HI_RES     = 0b001,
	USER_MODE_JITTER     = 0b010,
	USER_MODE_CALM       = 0b011
} user_mode_t;




/*** Globals *****************************************************************/
// Ring Buffer Variables
#define                 MI_BUFFER_SIZE   512
static mouse_instr_t    g_mi_buffer[MI_BUFFER_SIZE];
volatile uint32_t       g_mi_buffer_head = 0;
volatile uint32_t       g_mi_buffer_tail = 0;


// Buffer is ready for another movement flag.
// 0x00 Not Empty
// 0x01 Empty
volatile uint8_t        g_buffer_empty_flag = 0x00;


// User Settings Flags
static user_mode_t   g_user_mode     = USER_MODE_NORMAL;



/*** Forward Declarations ****************************************************/
/// @brief Efficient Implimentation of an integer abs() function
/// @param input x
/// @param output abs(x)
uint32_t int_abs(const int32_t x);


/// @brief Generates a random signed integer, limited to a maximum range
/// @param None
/// @return int16_t integer
int16_t int_rand(void);


/// @brief Mouse Instruction Ring Buffer Push (Puts data in the buffer)
/// @param Mouse Instruction
/// @return Mouse Instruction Status
mi_buffer_status_t mi_buffer_push(const mouse_instr_t inst);


/// @brief Mouse Instruction Ring Buffer Pop (Pulls off data from buffer)
/// @param Mouse Instruction Pointer
/// @return Mouse Instruction Status
mi_buffer_status_t mi_buffer_pop(mouse_instr_t *instr);


/// @brief Mouse Instruction Ring Buffer Peek (Reads value without incrimenting)
/// @param Mouse Instruction Pointer
/// @return Mouse Instruction Status
mi_buffer_status_t mi_buffer_peek(mouse_instr_t *instr);


/// @brief Mouse Instruction Ring Buffer Skip (Skip current buffer data)
/// @param None
/// @return Mouse Instruction Status
mi_buffer_status_t mi_buffer_skip(void);


/// @brief Plots movement to a given co-ordinate point. Appends the movement
/// data to the circuilar buffer to be dispatched to the USB Interrupt
/// @param postion_t endpoint to plot to. Contains X/Y data, can be positive
/// or negative
/// @return Mouse Inscription buffer status - if push fails
mi_buffer_status_t move_to_endpoint(const position_t endpoint);


/// @brief sets HID mouse bytes from an input Mouse Movement Instruction
/// @param buffer to modify
/// @param instruction to parse
/// @return None
void set_mouse_instr_bytes(uint8_t *buffer, mouse_instr_t instr);






#include "rv003usb/usb_config.h"


uint8_t usb_serial[USB_SERIAL_BYTES] = {
	USB_SERIAL_BYTES,    // bLength
	3,                   // bDescriptorType
	'F', 0x00,           // wString
	'E', 0x00,
	'D', 0x00,
	'C', 0x00,
	'B', 0x00,
	'A', 0x00,
	'9', 0x00,
	'8', 0x00,
};




/*** Main ********************************************************************/
int main(void)
{
	SystemInit();

	// Grab two words from the near-top of RAM - Should be uninitialised noise
	uint32_t ram_val =   *((volatile uint32_t*)0x20000700) 
		               ^ *((volatile uint32_t*)0x200007AA);
	// Set the LFSR Seed to the RAM Value, provided it is not 0x00.. or 0xFF..
	// If this doesn't seed the default will be used
	if(ram_val != 0x00000000 && ram_val != 0xFFFFFFFF)
		seed(ram_val);


	/*** GPIO *******************************************************************/
	// Enable the GPIO Channels
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC;

	// Clear PA1 PA2 and PC4, set them to Push/Pull, then write PULLUP
	GPIOA->CFGLR &= ~((0x0000000F << 4) | (0x0000000F << 8));
	GPIOA->CFGLR |=  ((GPIO_CFGLR_IN_PUPD << 4) | (GPIO_CFGLR_IN_PUPD << 8));
	GPIOA->OUTDR |=  ((0x01 << 1) | (0x01 << 2));

	GPIOC->CFGLR &= ~(0x0000000F << 16);
	GPIOC->CFGLR |=  (GPIO_CFGLR_IN_PUPD << 16);
	GPIOC->OUTDR |=  (0x01 << 4);

	// Read the Jumpers to set the user_mode
	if(!((GPIOA->INDR >> 2) & 0x01)) g_user_mode |= 0x01;      // JP1 PA2
	if(!((GPIOA->INDR >> 1) & 0x01)) g_user_mode |= 0x02;      // JP2 PA1
	if(!((GPIOC->INDR >> 4) & 0x01)) g_user_mode |= 0x04;      // JP3 PC4


	/*** USB ********************************************************************/
	// Ensures USB re-enumeration after bootloader or reset
	Delay_Ms(1); 
	usb_setup();


	while(1) 
	{
		// NOTE: Prints random values to evaluate random number algorithm
		//printf("%d\n", int_rand());

		// Wait for the flag that the buffer is empty
		if(g_buffer_empty_flag)
		{
			// Generate a random position then push the commands to move to it
			position_t rand_pos = {.x = int_rand(), .y = int_rand()};
			move_to_endpoint(rand_pos);

			// Reset the empty flag, waits until it is done moving
			g_buffer_empty_flag = 0x00;



		}

	} 
	// end of loop
	

	return 0;
}


// rv003usb HID Function
void usb_handle_user_in_request( struct usb_endpoint * e, uint8_t * scratchpad, int endp, uint32_t sendtok, struct rv003usb_internal * ist )
{
	static mouse_instr_t crnt_mouse_instr;
	static mouse_instr_t next_mouse_instr;

	// Handle the USB Mouse messages
	if(endp == 1)
	{
		// Define an empty mouse bytes array
		uint8_t mouse_bytes[4] = {0x00, 0x00, 0x00, 0x00};

		// Get the current and next data chunks
		mi_buffer_status_t crnt_buffer_status = mi_buffer_pop(&crnt_mouse_instr);
		mi_buffer_status_t next_buffer_status = mi_buffer_peek(&next_mouse_instr);

		// If the current status is OK, set the data bytes
		if(crnt_buffer_status == MI_BUFFER_OK)
		{
			set_mouse_instr_bytes(mouse_bytes, crnt_mouse_instr);
		}

		// If it's empty, set the flag and leave the buffer unset
		else
		{
			g_buffer_empty_flag = 0x01;
		}


		// If the next instruction is valid, parse it (Allows for diagonal movement)
		if(next_buffer_status == MI_BUFFER_OK)
		{
			// If this instruction is LEFT or RIGHT, and the previous was UP or DOWN,
			// add this instruction to the data
			if((next_mouse_instr == MOUSE_INSTR_L || next_mouse_instr == MOUSE_INSTR_R)
			&& (crnt_mouse_instr == MOUSE_INSTR_U || crnt_mouse_instr == MOUSE_INSTR_D))
			{
				set_mouse_instr_bytes(mouse_bytes, next_mouse_instr);
				mi_buffer_skip();
			}

			// Likewise with UP and DOWN, append if last instruction was LEFT or RIGHT
			if((next_mouse_instr == MOUSE_INSTR_U || next_mouse_instr == MOUSE_INSTR_D)
			&& (crnt_mouse_instr == MOUSE_INSTR_L || crnt_mouse_instr == MOUSE_INSTR_R))
			{
				set_mouse_instr_bytes(mouse_bytes, next_mouse_instr);
				mi_buffer_skip();
			}
		}
		

		// Send the data to the USB Controller - Either 0x00's or has Delta data
		usb_send_data(mouse_bytes, 4, 0, sendtok);
	}
	else
	{
		// If it's a control transfer, empty it.
		usb_send_empty(sendtok);
	}
}


void set_mouse_instr_bytes(uint8_t buffer[], mouse_instr_t instr)
{
	// signed 8 bit ints for movement, using Unsigned representation
	// [1] is (0xFF)L  (0x01)R
	// [2] is (0xFF)U  (0x01)D
	switch(instr)
	{
		case MOUSE_INSTR_L:
			buffer[1] |= 0xFF;
			break;
		case MOUSE_INSTR_R:
			buffer[1] |= 0x01;
			break;

		case MOUSE_INSTR_U:
			buffer[2] |= 0xFF;
			break;
		case MOUSE_INSTR_D:
			buffer[2] |= 0x01;
			break;
	}
}


/*** Functions ***************************************************************/
int16_t int_rand(void)
{
	// Generate a number between 0 - MAXIMUM, then subtract MAXIMUM/2 to get 
	// it in the correct range
	int16_t rand_num = 0x7FFF;

	// Generate different Ranges based on user setup
	switch(g_user_mode)
	{
		// +- 125 Units
		case USER_MODE_NORMAL:
			while(rand_num > 250) rand_num = rand() & 0x00FF;
			rand_num -= 125;
			break;
		

		// +- 250 Units
		case USER_MODE_HI_RES:
			while(rand_num > 500) rand_num = rand() & 0x01FF;
			rand_num -= 250;
			break;


		// +- 20 Units
		case USER_MODE_JITTER:
			while(rand_num > 40) rand_num = rand() & 0x003F;
			rand_num -= 20;
			break;


		// +- 1 Units
		case USER_MODE_CALM:
			rand_num = (rand() & 0x01) ? 1 : -1;
			break;


		default:
			rand_num = 0x00;
			break;
	}

    return rand_num;
}


uint32_t int_abs(const int32_t x)
{
	// Extract the sign bit
	uint32_t mask = x >> 31;
	// Invert the 2's compliment
	return (x ^ mask) - mask;
}


mi_buffer_status_t mi_buffer_push(const mouse_instr_t instr)
{
	// Calculate the next head position
	uint32_t next_head = (g_mi_buffer_head + 1) % MI_BUFFER_SIZE;
	// If there is no space left in the buffer, reject incomming data
	if(next_head == g_mi_buffer_tail) return MI_BUFFER_NO_SPACE;

	// Append the data to the current head position
	g_mi_buffer[g_mi_buffer_head] = instr;
	// Update the current head position
	g_mi_buffer_head = next_head;

	return MI_BUFFER_OK;
}


mi_buffer_status_t mi_buffer_pop(mouse_instr_t *instr)
{
	// Exit if there is no more data to be popped off
	if(g_mi_buffer_head == g_mi_buffer_tail) return MI_BUFFER_NO_DATA;
	// Set the data pointer from the buffer
	*instr = g_mi_buffer[g_mi_buffer_tail];

	// Update the Tail Position
	g_mi_buffer_tail = (g_mi_buffer_tail + 1) % MI_BUFFER_SIZE;

	return MI_BUFFER_OK;
}


mi_buffer_status_t mi_buffer_peek(mouse_instr_t *instr)
{
	// Exit if there is no more data to be popped off
	if(g_mi_buffer_head == g_mi_buffer_tail) return MI_BUFFER_NO_DATA;
	// Set the data pointer from the buffer
	*instr = g_mi_buffer[g_mi_buffer_tail];

	return MI_BUFFER_OK;
}


mi_buffer_status_t mi_buffer_skip(void)
{
	// Update the Tail Position
	g_mi_buffer_tail = (g_mi_buffer_tail + 1) % MI_BUFFER_SIZE;
	return MI_BUFFER_OK;
}


mi_buffer_status_t move_to_endpoint(const position_t endpoint)
{
	mi_buffer_status_t mi_return = MI_BUFFER_OK;
 
	position_t startpoint = {0, 0};

	// Bresenham variables
	// Delta x and y - total distances to cover in x and y direction
	int32_t x_delta = int_abs(endpoint.x - startpoint.x);
	int32_t y_delta = int_abs(endpoint.y - startpoint.y);
	// Which direction to step in
	int32_t x_step = (startpoint.x < endpoint.x)  ?  1 : -1;
	int32_t y_step = (startpoint.y < endpoint.y)  ?  1 : -1;
	// Accumulated Error - how far from the ideal line we are
	int32_t err = x_delta - y_delta;


	// Step through the line until the destination is reached
	while(startpoint.x != endpoint.x || startpoint.y != endpoint.y) 
	{
		// Multiply the error by 2 to avoid fractional calculations
		int32_t err2 = err * 2;

		// Step in the X direction - remove vertical error to account for
		// the change in horizontal position
		if(err2 > -y_delta)
		{
			err -= y_delta;
			startpoint.x += x_step;

			// Push Left or Right instructions to the Mouse Delta Buffer
			mi_return = mi_buffer_push( (x_step > 0) ? MOUSE_INSTR_R : MOUSE_INSTR_L);
			// If the buffer is full, exit early
			if(mi_return != MI_BUFFER_OK)  return mi_return;
		}

		// Step in the Y direction - add the horizontal error to account for
		// the change in vertical position
		if(err2 < x_delta) 
		{
			err += x_delta;
			startpoint.y += y_step;

			// Push Up and Down Instructions to the Mouse Delta Buffer
			mi_return = mi_buffer_push( (y_step > 0) ? MOUSE_INSTR_U : MOUSE_INSTR_D);
			// If the buffer is full, exit early
			if(mi_return != MI_BUFFER_OK)  return mi_return;
		}
	}

	return mi_return;
}
