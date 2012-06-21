#include "stdafx.h"
#include "PerlinNoiseGenerator.h"

#include "Utils.h"


std::unique_ptr<unsigned char[]> PerlinNoiseGenerator::generate(unsigned int width, unsigned int height, unsigned int depth,
														float Frequency, float Amplitude, float Persistance, unsigned int Octaves, float threshhold)
{
	unsigned int numValues = width*height*depth;
	std::unique_ptr<float[]> perlinNoiseFloat(new float[numValues]);
	std::unique_ptr<float[]> whiteNoise(new float[numValues]);

	// generate white noise
	for(unsigned int i=0; i<numValues; ++i)
	{
		whiteNoise[i] = random(-1.0f, 1.0f);
		perlinNoiseFloat[i] = 0.0f;
	}

	#define index(__x,__y,__z) ((__x) + (__y)*width + (__z)*(height*width))

	// for every octave...
	for(unsigned int i = 0; i < Octaves; ++i)
    {
		for(unsigned int z=0; z<depth; ++z)
		{
			for(unsigned int y=0; y<height; ++y)
			{
				for(unsigned int x=0; x<width; ++x)
				{
					float xfreq = x * Frequency;
					float yfreq = y * Frequency;
					float zfreq = z * Frequency;

					float FractionX = perlinInterpolation(xfreq - (int)xfreq);
					float FractionY = perlinInterpolation(yfreq - (int)yfreq);
					float FractionZ = perlinInterpolation(zfreq - (int)zfreq);
					int X1 = ((int)xfreq + width) % width;
					int Y1 = ((int)yfreq + height) % height;
					int Z1 = ((int)zfreq + depth) % depth;
					int X2 = ((int)xfreq + width - 1) % width;
					int Y2 = ((int)yfreq + height - 1) % height;
					int Z2 = ((int)zfreq + depth - 1) % depth;

				/*	float finalValue = 0.0f;
					finalValue += FractionX * FractionY				* whiteNoise[index(X1, Y1, Z1)];
					finalValue += FractionX * (1 - FractionY)		* whiteNoise[index(X1, Y2, Z1)];
					finalValue += (1 - FractionX) * FractionY		* whiteNoise[index(X2, Y1, Z1)];
					finalValue += (1 - FractionX) * (1 - FractionY) * whiteNoise[index(X2, Y2, Z1)];
					*/
					float a = interpolateBilinear(whiteNoise[index(X1, Y1, Z2)], whiteNoise[index(X2, Y1, Z2)],
													whiteNoise[index(X1, Y2, Z2)], whiteNoise[index(X2, Y2, Z2)],
													FractionX, FractionY);
				/*	float b = interpolateBilinear(whiteNoise[index(X1, Y1, Z1)], whiteNoise[index(X2, Y1, Z1)],
												  whiteNoise[index(X1, Y2, Z1)], whiteNoise[index(X2, Y2, Z1)],
													FractionX, FractionY);
					float finalValue = interpolateLinear(b, a, FractionZ);*/

					perlinNoiseFloat[index(x,y,z)] += a;// finalValue;
				}
			}
		}

		Frequency *= 2.0f;
        Amplitude *= Persistance;
	}

	std::unique_ptr<unsigned char[]> perlinNoise(new unsigned char[numValues]);

	float min = std::numeric_limits<float>::infinity();
	float max = -std::numeric_limits<float>::infinity();
	for(unsigned int i=0; i<numValues; ++i)
	{
		min = std::min(min, perlinNoiseFloat[i]);
		max = std::max(max, perlinNoiseFloat[i]);
	}
	float area = max - min;


	for(unsigned int i=0; i<numValues; ++i)
	{
		float f = (perlinNoiseFloat[i] - min) / area;
		f = (f - threshhold) / (1.0f - threshhold);
		if(f<0) f = 0.0f;
		perlinNoise[i] = (unsigned char)(f * 255);
	}

	return perlinNoise;
}

std::unique_ptr<unsigned char[]> PerlinNoiseGenerator::generate(unsigned int width, unsigned int height, 
														float Frequency, float Amplitude, float Persistance, unsigned int Octaves, float threshhold)
{
	unsigned int numValues = width*height;
	std::unique_ptr<float[]> perlinNoiseFloat(new float[numValues]);
	std::unique_ptr<float[]> whiteNoise(new float[numValues]);

	// generate white noise
	for(unsigned int i=0; i<numValues; ++i)
	{
		whiteNoise[i] = random(-1.0f, 1.0f);
		perlinNoiseFloat[i] = 0.0f;
	}

	#undef index
	#define index(__x,__y) ((__x) + (__y)*width)

	// for every octave...
	for(unsigned int i = 0; i < Octaves; ++i)
    {
		for(unsigned int y=0; y<height; ++y)
		{
			for(unsigned int x=0; x<width; ++x)
			{
				float xfreq = x * Frequency;
				float yfreq = y * Frequency;

				float FractionX = perlinInterpolation(xfreq - (int)xfreq);
				float FractionY = perlinInterpolation(yfreq - (int)yfreq);
				int X1 = ((int)xfreq + width) % width;
				int X2 = ((int)xfreq + width - 1) % width;
				int Y1 = ((int)yfreq + height) % height;
				int Y2 = ((int)yfreq + height - 1) % height;

				float finalValue = 0.0f;
				finalValue += FractionX * FractionY				* whiteNoise[index(X1, Y1)];
				finalValue += FractionX * (1 - FractionY)		* whiteNoise[index(X1, Y2)];
				finalValue += (1 - FractionX) * FractionY		* whiteNoise[index(X2, Y1)];
				finalValue += (1 - FractionX) * (1 - FractionY) * whiteNoise[index(X2, Y2)];
				perlinNoiseFloat[index(x,y)] += finalValue;
			}
		}

		Frequency *= 2.0f;
        Amplitude *= Persistance;
	}

	return toByte(perlinNoiseFloat.get(), numValues, threshhold);
}

std::unique_ptr<unsigned char[]> PerlinNoiseGenerator::toByte(float* data, unsigned int size, float threshhold)
{
	std::unique_ptr<unsigned char[]> perlinNoise(new unsigned char[size]);

	float min = std::numeric_limits<float>::infinity();
	float max = -std::numeric_limits<float>::infinity();
	for(unsigned int i=0; i<size; ++i)
	{
		min = std::min(min, data[i]);
		max = std::max(max, data[i]);
	}
	float area = max - min;


	for(unsigned int i=0; i<size; ++i)
	{
		float f = (data[i] - min) / area;
		//f = (f - threshhold) / (1.0 - threshhold);
		//if(f<0) f = 0.0f;
		perlinNoise[i] = (unsigned char)(f * 255);
	}

	return perlinNoise;
}


float PerlinNoiseGenerator::perlinInterpolation(float _x)
{
    // C2-continual implementation
    // For details see "Burger-GradientNoiseGerman-2008".
    return _x * _x * _x * (_x * (_x * 6.0f - 15.0f) + 10.0f);
}