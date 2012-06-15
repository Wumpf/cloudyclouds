#pragma once

extern float degToRad(float degree);
extern float radToDeg(float rad);

template<class T> void swap(T& a, T& b)
{
	T temp = b;
	b = a;
	a = temp;
}

bool checkGLError(const char* Title);

float random(float min = 0.0f, float max = 1.0f);