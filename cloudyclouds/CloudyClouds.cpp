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


	// init cloud renderin
	clouds.reset(new Clouds());

	// matrix temp

	// test with glu
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, static_cast<float>(backBufferResolutionX) / backBufferResolutionY,
														0.1f, 100.0f);
	glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);



	// global matrices ubo
	glGenBuffers(1, &uboGlobalMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboGlobalMatrices); 
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4*4 * 3, NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 4 * 4, projectionMatrix);	// write projection
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboGlobalMatrices);	// bind to index 0 - its assumed that ubo-index0-binding will never change
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
	glBindBuffer(GL_UNIFORM_BUFFER, uboGlobalMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 4 * 4, projectionMatrix);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4*4, sizeof(float) * 4 * 4, camera->getViewMatrix());	// update view
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4*4 * 2, sizeof(float) * 4 * 4, camera->getViewMatrix() * projectionMatrix);	// update viewprojection

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboGlobalMatrices);

	// clear scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	clouds->display(timeSinceLastFrame);

	// next frame
	glfwSwapBuffers();

	return true;
}