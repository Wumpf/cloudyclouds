#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

float degToRad(float degree)
{
	return static_cast<float>(degree * (M_PI / 360));
}

float radToDeg(float rad)
{
	return static_cast<float>(rad / (M_PI / 360));
}