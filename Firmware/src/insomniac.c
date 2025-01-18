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
* ADBeta (c) 2025    14 Jan 2025    Ver1.1.0
******************************************************************************/
#include "ch32v003fun.h"
#include "rv003usb.h"
#include "lib_rand.h"
#include <stdio.h>


/*** Types and Definitions ***************************************************/
/// @brief A C-ordinate position struct, X/Y Data can be positive or negative
typedef struct {
	int16_t          x;
	int16_t          y;
} position_t;

/// @brief Remapping of a uint8_t to movement type, 2 nibbles
/// [Up    /   Down]  [Left  /  Right]
///  1100      0011    1100      0011
typedef uint8_t mouse_delta_t;

#define MOUSE_DELTA_U      0b11000000
#define MOUSE_DELTA_D      0b00110000
#define MOUSE_DELTA_L      0b00001100
#define MOUSE_DELTA_R      0b00000011

typedef enum {
	MD_BUFFER_OK             = 0,
	MD_BUFFER_NO_SPACE,             // No Space to append to buffer
	MD_BUFFER_NO_DATA               // No Data to read from the buffer
} md_buffer_status_t;

// User Settings
typedef enum { USER_DISTANCE_125 = 0,  USER_DISTANCE_60 = 1 } user_distance_t;

/*** Globals *****************************************************************/
// Ring Buffer Variables
#define                 MD_BUFFER_SIZE   512
static mouse_delta_t    g_md_buffer[MD_BUFFER_SIZE];
volatile uint32_t       g_md_buffer_head = 0;
volatile uint32_t       g_md_buffer_tail = 0;

// Buffer is ready for another movement flag.
// 0x00 Not Empty
// 0x01 Empty
volatile uint8_t        g_buffer_empty_flag = 0x00;

// User Settings Flags
static user_distance_t   g_user_distance     = USER_DISTANCE_125;


/*** Forward Declarations ****************************************************/
/// @brief Efficient Implimentation of an integer abs() function
/// @param input x
/// @param output abs(x)
uint32_t int_abs(const int32_t x);

/// @brief Generates a random signed integer, limited to a maximum range
/// @param None
/// @return int16_t integer
int16_t int_rand(void);

/// @brief Mouse Delta Ring Buffer Push (Puts data in the buffer)
/// @param Mouse Delta Value
/// @return Mouse Delta Status
md_buffer_status_t md_buffer_push(const mouse_delta_t mdv);

/// @brief Mouse Delta Ring Buffer Pop (Pulls off data from buffer)
/// @param Mouse Delta Pointer
/// @return Mouse Delta Status
md_buffer_status_t md_buffer_pop(mouse_delta_t *mdp);

/// @brief Mouse Delta Ring Buffer Peek (Reads value without incrimenting)
/// @param Mouse Delta Pointer
/// @return Mouse Delta Status
md_buffer_status_t md_buffer_peek(mouse_delta_t *mdp);

/// @brief Mouse Delta Ring Buffer Skip (Skip current buffer data)
/// @param None
/// @return Mouse Delta Status
md_buffer_status_t md_buffer_skip(void);


/// @brief Plots movement to a given co-ordinate point. Appends the movement
/// data to the circuilar buffer to be dispatched to the USB Interrupt
/// @param postion_t endpoint to plot to. Contains X/Y data, can be positive
/// or negative
/// @return md_buffer_state to mirror if the buffer is full
md_buffer_status_t move_to_endpoint(const position_t endpoint);


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


	// Enable the GPIO Channels
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC;

	// Clear PA1 PA2 and PC4, set them to Push/Pull, then write them HIGH
	// INPUT_PULLUP
	GPIOA->CFGLR &= ~((0x0000000F << 4) | (0x0000000F << 8));
	GPIOA->CFGLR |=  ((GPIO_CFGLR_IN_PUPD << 4) | (GPIO_CFGLR_IN_PUPD << 8));
	GPIOA->OUTDR |=  ((0x01 << 1) | (0x01 << 2));

	GPIOC->CFGLR &= ~(0x0000000F << 16);
	GPIOC->CFGLR |=  (GPIO_CFGLR_IN_PUPD << 16);
	GPIOC->OUTDR |=  (0x01 << 4);

	// Read the input states and set settings
	// Change Disatance (Default +-125) if JP1 Set
	if(!(GPIOA->INDR & (0x01 << 2)))      // JP1 PA2
		g_user_distance = USER_DISTANCE_60;

	// Ensures USB re-enumeration after bootloader or reset
	Delay_Ms(1); 
	usb_setup();


	while(1) 
	{
		// NOTE: Prints random values to evaluate random number algorithm
		// printf("%d\n", int_rand());

		// Wait for the flag that the buffer is empty
		if(g_buffer_empty_flag)
		{
			// Generate a random position then push the commands to move to it
			position_t rand_pos = {.x = int_rand(), .y = int_rand()};
			move_to_endpoint(rand_pos);

			// Reset the empty flag, waits until it is done moving
			g_buffer_empty_flag = 0x00;
		}

	} // end of loop
}


// rv003usb HID Function
void usb_handle_user_in_request( struct usb_endpoint * e, uint8_t * scratchpad, int endp, uint32_t sendtok, struct rv003usb_internal * ist )
{
	static mouse_delta_t mouse_delta;

	// Handle the USB Mouse messages
	if(endp == 1)
	{
		// Define an empty mouse data array
		uint8_t mouse_data[4] = {0x00, 0x00, 0x00, 0x00};

		// Modify the buffer if there is data ready in the Mouse Delta Buffer 
		if(md_buffer_pop(&mouse_delta) == MD_BUFFER_OK)
		{
			// signed 8 bit ints for movement, using Unsigned representation
			// [1] is (0xFF)L  (0x01)R
			// [2] is (0xFF)U  (0x01)D

			switch(mouse_delta)
			{
				case MOUSE_DELTA_L:
					mouse_data[1] = 0xFF;
					break;
				case MOUSE_DELTA_R:
					mouse_data[1] = 0x01;
					break;

				case MOUSE_DELTA_U:
					mouse_data[2] = 0xFF;
					break;
				case MOUSE_DELTA_D:
					mouse_data[2] = 0x01;
					break;
			}
		}
		else
		{
			// Flag that the buffer is ready for new movement data
			g_buffer_empty_flag = 0x01;
		}

		// Send the data to the USB Controller - Either 0x00's or has Delta data
		usb_send_data(mouse_data, 4, 0, sendtok);
	}
	else
	{
		// If it's a control transfer, empty it.
		usb_send_empty(sendtok);
	}
}


/*** Functions ***************************************************************/
int16_t int_rand(void)
{
	// Generate a number between 0 - MAXIMUM, then subtract MAXIMUM/2 to get 
	// it in the correct range
	int16_t rand_num = 0x7FFF;

	// Generate different Ranges based on user setup
	switch(g_user_distance)
	{
		case USER_DISTANCE_125:
			while(rand_num > 250) rand_num = rand() & 0x00FF;
			rand_num -= 125;
			break;
		
		case USER_DISTANCE_60:
			while(rand_num > 120) rand_num = rand() & 0x07F;
			rand_num -= 60;
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


md_buffer_status_t md_buffer_push(const mouse_delta_t mdv)
{
	// Calculate the next head position
	uint32_t next_head = (g_md_buffer_head + 1) % MD_BUFFER_SIZE;
	// If there is no space left in the buffer, reject incomming data
	if(next_head == g_md_buffer_tail) return MD_BUFFER_NO_SPACE;

	// Append the data to the current head position
	g_md_buffer[g_md_buffer_head] = mdv;
	// Update the current head position
	g_md_buffer_head = next_head;

	return MD_BUFFER_OK;
}


md_buffer_status_t md_buffer_pop(mouse_delta_t *mdp)
{
	// Exit if there is no more data to be popped off
	if(g_md_buffer_head == g_md_buffer_tail) return MD_BUFFER_NO_DATA;
	// Set the data pointer from the buffer
	*mdp = g_md_buffer[g_md_buffer_tail];

	// Update the Tail Position
	g_md_buffer_tail = (g_md_buffer_tail + 1) % MD_BUFFER_SIZE;

	return MD_BUFFER_OK;
}


md_buffer_status_t md_buffer_peek(mouse_delta_t *mdp)
{
	// Exit if there is no more data to be popped off
	if(g_md_buffer_head == g_md_buffer_tail) return MD_BUFFER_NO_DATA;
	// Set the data pointer from the buffer
	*mdp = g_md_buffer[g_md_buffer_tail];

	return MD_BUFFER_OK;
}


md_buffer_status_t md_buffer_skip(void)
{
	// Update the Tail Position
	g_md_buffer_tail = (g_md_buffer_tail + 1) % MD_BUFFER_SIZE;
	return MD_BUFFER_OK;
}


md_buffer_status_t move_to_endpoint(const position_t endpoint)
{
	md_buffer_status_t md_return = MD_BUFFER_OK;
 
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
			md_return = md_buffer_push( (x_step > 0) ? MOUSE_DELTA_R : MOUSE_DELTA_L);
			// If the buffer is full, exit early
			if(md_return != MD_BUFFER_OK)  return md_return;
		}

		// Step in the Y direction - add the horizontal error to account for
		// the change in vertical position
		if(err2 < x_delta) 
		{
			err += x_delta;
			startpoint.y += y_step;

			// Push Up and Down Instructions to the Mouse Delta Buffer
			md_return = md_buffer_push( (y_step > 0) ? MOUSE_DELTA_U : MOUSE_DELTA_D);
			// If the buffer is full, exit early
			if(md_return != MD_BUFFER_OK)  return md_return;
		}
	}

	return md_return;
}
