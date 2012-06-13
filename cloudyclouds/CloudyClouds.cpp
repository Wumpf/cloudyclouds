#include "stdafx.h"
#include "CloudyClouds.h"
#include "Utils.h"
#include "Clouds.h"
#include "Camera.h"


CloudyClouds::CloudyClouds() :
	camera(new Camera())
{
	// some glfw properties
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 0);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);	// want opengl 3.3
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	//glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	// glfw init
	if(glfwInit() != GL_TRUE)
		throw std::exception("ERROR: glfwInit() failed!\n");

	// setup window
	// \todo xml config
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

	// glew init
	GLenum err = glewInit();
	if (GLEW_OK != err)
		throw std::exception((std::string("ERROR: glewInit() failed!\n") + (char*)glewGetErrorString(err)).c_str());

	// projection matrix
	projectionMatrix = Matrix4::projectionPerspective(degToRad(45.0f), static_cast<float>(backBufferResolutionX) / backBufferResolutionY, 0.1f, 100.0f);


	// global Screen ubo
	glGenBuffers(1, &uboScreen);
	glBindBuffer(GL_UNIFORM_BUFFER, uboScreen); 
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4 * 5, NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 4 * 4, projectionMatrix);	// write projection
	float inverseScreenResolution[] = { 1.0f/backBufferResolutionX, 1.0f/backBufferResolutionY };
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4 * 4, sizeof(float) * 2, inverseScreenResolution);	// write inverse screen
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboScreen);	// bind to index 0 - its assumed that ubo-index0-binding will never change
	
	// global View ubo
	glGenBuffers(1, &uboView);
	glBindBuffer(GL_UNIFORM_BUFFER, uboView); 
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4 * 9, NULL, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboView);	// bind to index 1 - its assumed that ubo-index1-binding will never change

	// init cloud rendering
	clouds.reset(new Clouds(backBufferResolutionX, backBufferResolutionY));
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
	glfwPollEvents();
	camera->update(timeSinceLastFrame);

	return true;
}

bool CloudyClouds::display(float timeSinceLastFrame)
{
	// update global matrices
	glBindBuffer(GL_UNIFORM_BUFFER, uboView);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 4 * 4, camera->getViewMatrix());	// update view
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4*4 , sizeof(float) * 4 * 4, camera->getViewMatrix() * projectionMatrix);	// update viewprojection
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4*4 *2, sizeof(float) * 3, camera->getPosition());



	glBindBuffer(GL_UNIFORM_BUFFER, uboScreen);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboScreen);

	glBindBuffer(GL_UNIFORM_BUFFER, uboView);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboView);

	// clear scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	clouds->display(timeSinceLastFrame);

	// next frame
	glfwSwapBuffers();

	return true;
}