/******************************************************************************
*
******************************************************************************/
#include "ch32v003fun.h"
#include <stdio.h>

/*** Types and Definitions ***************************************************/
typedef struct {
	int16_t          x;
	int16_t          y;
} position_t;

/** Forward Declarations *****************************************************/

/// TODO:
void move_to_endpoint(const position_t endpoint);


/*** Main ********************************************************************/
int main(void)
{
	SystemInit();
	
	printf("5,0\n");
	move_to_endpoint( (const position_t){-5, 0});

	printf("\n\n0,5\n");
	move_to_endpoint( (const position_t){0, -5});

	printf("\n\n5,5\n");
	move_to_endpoint( (const position_t){-5, -5});

}


/*** Functions ***************************************************************/
uint32_t int_abs(const int32_t x)
{
	uint32_t mask = x >> 31; // Extract the sign bit
	return (x ^ mask) - mask;
}

void move_to_endpoint(const position_t endpoint)
{
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

			// TODO:
			printf("Step: %s\n", (x_step > 0) ? "right" : "left");
		}

		// Step in the Y direction - add the horizontal error to account for
		// the change in vertical position
		if(err2 < x_delta) 
		{
			err += x_delta;
			startpoint.y += y_step;

			// TODO:
			printf("Step: %s\n", (y_step > 0) ? "up" : "down");
		}
	}
}
