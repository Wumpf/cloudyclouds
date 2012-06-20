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

template <class Value, class Interpolation> Value interpolateBilinear(const Value& A,
																		const Value& B,
																		const Value& C,
																		const Value& D,
																		const Interpolation x,
																		const Interpolation y)
{	
	const Value P(A + x * (B - A));
	const Value Q(C + x * (D - C));
	return P + y * (Q - P);	 
}

template <class Value, class Interpolation> Value interpolateLinear(const Value& A, const Value& B, const Interpolation& i)
{	return A + i * (B - A);	 }