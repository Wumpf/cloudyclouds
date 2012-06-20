#pragma once

#include <memory>

class PerlinNoiseGenerator
{
public:
	std::unique_ptr<unsigned char[]> generate(unsigned int width, unsigned int height, unsigned int depth,
											float Frequency, float Amplitude, float Persistance, int Octaves, float threshhold);


	/// singleton getter
	static PerlinNoiseGenerator& get()
	{
		static PerlinNoiseGenerator theOnlyOne;
		return theOnlyOne;
	}
private:
	PerlinNoiseGenerator() {}
	~PerlinNoiseGenerator() {}

	inline float perlinInterpolation(float _x);
};