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
	void InitUBOs();

	bool update(float timeSinceLastFrame);
	bool display(float timeSinceLastFrame);

	unsigned int backBufferResolutionX;
	unsigned int backBufferResolutionY;
	

	// global ubos
	GLuint uboScreen;
	GLuint uboView;
	GLuint uboTimings;

	Matrix4 projectionMatrix;	

	// cam
	std::unique_ptr<class Camera> camera;

	// clouds
	std::unique_ptr<class Clouds> clouds;
	// screen aligend tri
	std::unique_ptr<class ScreenAlignedTriangle> screenAlignedTriangle;
	// background
	std::unique_ptr<class Background> background;
	// terrain
	std::unique_ptr<class Terrain> terrain;


	Vector3 lightDirection;
};