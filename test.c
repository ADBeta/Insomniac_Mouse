#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
	int16_t    angle;                        // Angle in degrees 0 - 360
	uint16_t   distance;                     // Distance to travel
} euclid_vector;


/// @brief Send Mouse Step commands over USB to draw a line on the screen,
/// based on the input angle and distance using the Bresenham Algorithm
/// @param vect, euclidian vector to use for movement
/// @return None
void move_mouse_by_vector(const euclid_vector *vect)
{
	// Convert the vector angle to radians
	float angle_rad = vect->angle * (M_PI / 180.0);

	// X and Y starting point
	int32_t x_start = 0;
	int32_t y_start = 0;

	// Calculate X and Y end point from distance and angle
	int32_t x_end = round(vect->distance * cos(angle_rad));
	int32_t y_end = round(vect->distance * sin(angle_rad));

	//// Bresenham variables
	// Delta x and y - total distances to cover in x and y direction
	int32_t x_delta = abs(x_end - x_start);
	int32_t y_delta = abs(y_end - y_start);
	// Which direction to step in
	int32_t x_step = (x_start < x_end)   ?   1 : -1;
	int32_t y_step = (y_start < y_end)   ?   1 : -1;
	// Accumulated Error - how far from the ideal line we are
	int32_t err = x_delta - y_delta;


	// Step through the line until the destination is reached
	while(x_start != x_end || y_start != y_end) 
	{
		// Multiply the error by 2 to avoid fractional calculations
		int32_t err2 = err * 2;

		// Step in the X direction - remove vertical error to account for
		// the change in horizontal position
		if(err2 > -y_delta)
		{
			err -= y_delta;
			x_start += x_step;

			// TODO:
			printf("Step: %s\n", (x_step > 0) ? "right" : "left");
		}

		// Step in the Y direction - add the horizontal error to account for
		// the change in vertical position
		if(err2 < x_delta) 
		{
			err += x_delta;
			y_start += y_step;

			// TODO:
			printf("Step: %s\n", (y_step > 0) ? "up" : "down");
		}
	}
}

int main()
{
	printf("0\n");
	move_mouse_by_vector( &(const euclid_vector){0, 10} );


	printf("\n\n45\n");
	move_mouse_by_vector( &(const euclid_vector){45, 10} );



	printf("\n\n60\n");
	move_mouse_by_vector( &(const euclid_vector){60, 10} );
	
	return 0;
}

