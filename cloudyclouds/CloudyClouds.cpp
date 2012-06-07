#include "stdafx.h"
#include "CloudyClouds.h"


CloudyClouds::CloudyClouds()
{
	if(glfwInit() != GL_TRUE)
		throw std::exception("ERROR: glfwInit() failed!\n");

	// setup window
	// \todo xml config

	// some properties
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 0);
	//glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);	// want opengl 3.3
	//glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);

	// resolution
	backBufferResolutionX = 1024;
	backBufferResolutionY = 768;

#if WIN32
	// try systemres
	HMONITOR hMonitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);	//MONITOR_DEFAULTTONEAREST
	MONITORINFO lpmi;
	lpmi.cbSize = sizeof(lpmi);
	GetMonitorInfo(hMonitor, &lpmi);
	backBufferResolutionX = lpmi.rcMonitor.right;
	backBufferResolutionY = lpmi.rcMonitor.bottom;
#endif
	
	// open
	if(glfwOpenWindow(backBufferResolutionX, backBufferResolutionY, 8, 8, 8, 0, 24, 0, GLFW_WINDOW) != GL_TRUE) // GLFW_FULLSCREEN
		throw std::exception("ERROR: glfwOpenWindow() failed!\n");
	glfwSetWindowTitle("CloudyClouds");
}

CloudyClouds::~CloudyClouds()
{
	glfwTerminate();
}

void CloudyClouds::mainLoop()
{
	double oldTime = glfwGetTime();
	while(true)
	{
		double currentTime = glfwGetTime();
		float frameTime = static_cast<float>(currentTime - oldTime);

		if(!update(frameTime))
			break;
		if(!display(frameTime))
			break;

		oldTime = currentTime;
	}
}

bool CloudyClouds::update(float timeSinceLastFrame)
{
	return true;
}

bool CloudyClouds::display(float timeSinceLastFrame)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers();
	return true;
}

