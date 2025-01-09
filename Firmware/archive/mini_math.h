/******************************************************************************
* Bare minimim math.h implimentations for the current project (insomniac)
*
* ADBeta (c) 2025
******************************************************************************/
#define M_PI 3.14159265358979323846

/// @brief Aproximate cos() using 5th order Taylor Series
/// @param float x
/// @return float cos(x) (Approximate)
float mini_cos(float x);

/// @biref Approximate sin() using 6th order Taylor Series
/// @param float x
/// return float sin(x) (Approximate)
float mini_sin(float x);

/// @brief very simple implimentation of abs()
/// @param float x
/// @return abs(x)
float mini_abs(const float x);

/// @brief very simple implimentation of round()
/// @param float x
/// @return round(x)
float mini_round(const float x);
