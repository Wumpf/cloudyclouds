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
	
	// ubo for "GlobalMatrices"
	GLuint uboGlobalMatrices;
	Matrix4 projectionMatrix;

	// cam
	std::unique_ptr<class Camera> camera;

	// clouds
	std::unique_ptr<class Clouds> clouds;
};