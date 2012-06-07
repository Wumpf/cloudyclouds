#pragma once

#include "Matrix4.h"
#include <memory>

// the program class
class CloudyClouds
{
public:
	CloudyClouds();
	~CloudyClouds();

	void mainLoop();

private:
	bool update(float timeSinceLastFrame);
	bool display(float timeSinceLastFrame);

	unsigned int backBufferResolutionX;
	unsigned int backBufferResolutionY;

	// basic matrices
	Matrix4 cameraMatrix;
	Matrix4 projectionMatrix;
		// ubo for "GlobalMatrices"
	GLuint uboGlobalMatrices;

	// clouds
	std::unique_ptr<class Clouds> clouds;
};