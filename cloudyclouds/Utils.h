#pragma once

extern float degToRad(float degree);
extern float radToDeg(float rad);

template<class T> void swap(T& a, T& b)
{
	T temp = b;
	b = a;
	a = temp;
}