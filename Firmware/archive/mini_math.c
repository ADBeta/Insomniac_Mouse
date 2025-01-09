/******************************************************************************
* Bare minimim math.h implimentations for the current project (insomniac)
*
* ADBeta (c) 2025
******************************************************************************/
#include "mini-math.h"

// Function to calculate cosine using an improved Taylor series with 5 terms
float mini_cos(float x)
{
	// Normalize the input to the range -PI to PI for better accuracy
	while (x > M_PI)    x -= 2 * M_PI;
	while (x < -M_PI)   x += 2 * M_PI;

	// Compute powers of x (x^2, x^4, x^6, x^8, x^10)
	float x2 =    x  *   x;
	float x4 =   x2  *  x2;
	float x6 =   x4  *  x2;
	float x8 =   x6  *  x2;
	float x10 =   x8 *  x2;

	// Compute the cosine using the first five terms of the Taylor series
	float result = 1.0f - (x2 / 2.0f) + (x4 / 24.0f) - (x6 / 720.0f) + (x8 / 40320.0f) - (x10 / 3628800.0f);

	return result;
}


float mini_sin(float x)
{
	// Normalize the input to the range -PI to PI for better accuracy
	while (x > M_PI)    x -= 2 * M_PI;
	while (x < -M_PI)   x += 2 * M_PI;

	// Compute powers of x (x^3, x^5, x^7, x^9, x^11)
	float x2 =    x  *   x;
	float x3 =   x2  *   x;
	float x5 =   x3  *  x2;
	float x7 =   x5  *  x2;
	float x9 =   x7  *  x2;
	float x11 =  x9  *  x2;

	// Compute the sine using the first six terms of the Taylor series
	float result = x - (x3 / 6.0f) + (x5 / 120.0f) - (x7 / 5040.0f) + (x9 / 362880.0f) - (x11 / 39916800.0f);

	return result;
}


float mini_abs(const float x)
{
	// If x is negative, return -x; otherwise, return x
	return (x < 0.0f) ? -x : x;
}


float mini_round(const float x) 
{
	// Add 0.5 if x is positive, subtract 0.5 if x is negative, then truncate
	return (x > 0.0f) ? (int)(x + 0.5f) : (int)(x - 0.5f);
}
