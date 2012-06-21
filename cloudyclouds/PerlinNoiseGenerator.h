#pragma once

#include <memory>

class PerlinNoiseGenerator
{
public:
	std::unique_ptr<unsigned char[]> generate(unsigned int width, unsigned int height, unsigned int depth,
											float Frequency, float Amplitude, float Persistance, unsigned int Octaves, float threshhold);
	std::unique_ptr<unsigned char[]> generate(unsigned int width, unsigned int height,
											float Frequency, float Amplitude, float Persistance, unsigned int Octaves, float threshhold);

	/// singleton getter
	static PerlinNoiseGenerator& get()
	{
		static PerlinNoiseGenerator theOnlyOne;
		return theOnlyOne;
	}
private:
	PerlinNoiseGenerator() {}
	~PerlinNoiseGenerator() {}

	std::unique_ptr<unsigned char[]> toByte(float* data, unsigned int size, float threshhold);
	inline float perlinInterpolation(float _x);
};