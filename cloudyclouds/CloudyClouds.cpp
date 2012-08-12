#include "stdafx.h"
#include "CloudyClouds.h"
#include "Utils.h"
#include "Clouds.h"
#include "Camera.h"
#include "ScreenAlignedTriangle.h"
#include "Background.h"

#include <iomanip>

bool quit = false;
int onClose()
{
	quit = true;
	return GL_TRUE;
}

CloudyClouds::CloudyClouds() :
	camera(new Camera())
{
	// some glfw properties
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 0);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);	// want opengl 3.3
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	// glfw init
	if(glfwInit() != GL_TRUE)
		throw std::exception("ERROR: glfwInit() failed!\n");

	// setup window
	// \todo xml config
	// resolution
	//backBufferResolutionX = 1024;
	//backBufferResolutionY = 768;
	
#if WIN32
	// try systemres
	HMONITOR hMonitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);	//MONITOR_DEFAULTTONEAREST
	MONITORINFO lpmi;
	lpmi.cbSize = sizeof(lpmi);
	GetMonitorInfo(hMonitor, &lpmi);
	backBufferResolutionX = lpmi.rcMonitor.right;
	backBufferResolutionY = lpmi.rcMonitor.bottom;
#endif
	
	glDepthFunc(GL_LEQUAL);

	// open
	if(glfwOpenWindow(backBufferResolutionX, backBufferResolutionY, 8, 8, 8, 0, 24, 0, GLFW_WINDOW) != GL_TRUE) // GLFW_FULLSCREEN
		throw std::exception("ERROR: glfwOpenWindow() failed!\n");
	glfwSetWindowCloseCallback(&onClose);
	
	// glew init
	GLenum err = glewInit();
	if (GLEW_OK != err)
		throw std::exception((std::string("ERROR: glewInit() failed!\n") + (char*)glewGetErrorString(err)).c_str());

	// projection matrix
	const float farPlaneDistance = 300.0f;
	projectionMatrix = Matrix4::projectionPerspective(degToRad(45.0f), static_cast<float>(backBufferResolutionX) / backBufferResolutionY, 0.1f, farPlaneDistance);

	// uniform buffers
	InitUBOs();

	// init screen aligned tri
	screenAlignedTriangle.reset(new ScreenAlignedTriangle());

	// init cloud rendering
	clouds.reset(new Clouds(backBufferResolutionX, backBufferResolutionY, farPlaneDistance, *screenAlignedTriangle.get()));

	// init background rendering
	background.reset(new Background(*screenAlignedTriangle.get()));

	// start position
	camera->setPosition(Vector3(0,50,0));
}

CloudyClouds::~CloudyClouds()
{
	glDeleteBuffers(1, &uboScreen);
	glDeleteBuffers(1, &uboView);
	glDeleteBuffers(1, &uboTimings);

	glfwCloseWindow();
	glfwTerminate();
}

void CloudyClouds::InitUBOs()
{
	// Screen ubo
	glGenBuffers(1, &uboScreen);
	glBindBuffer(GL_UNIFORM_BUFFER, uboScreen); 
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4 * 5, NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 4 * 4, projectionMatrix);	// write projection
	float inverseScreenResolution[] = { 1.0f/backBufferResolutionX, 1.0f/backBufferResolutionY };
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4 * 4, sizeof(float) * 2, inverseScreenResolution);	// write inverse screen
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboScreen);	// bind to index 0 - its assumed that ubo-index0-binding will never change
	
	// view ubo
	glGenBuffers(1, &uboView);
	glBindBuffer(GL_UNIFORM_BUFFER, uboView); 
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 4 * 16, NULL, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboView);	// bind to index 1 - its assumed that ubo-index1-binding will never change

	// timings ubo
	glGenBuffers(1, &uboTimings);
	glBindBuffer(GL_UNIFORM_BUFFER, uboTimings); 
	glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 2, NULL, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboTimings);	// bind to index 1 - its assumed that ubo-index1-binding will never change
}

void CloudyClouds::mainLoop()
{
	double oldTime = glfwGetTime();
	while(!quit)
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

	// show fps in titel
	std::stringstream stringstream;
	stringstream << "CloudyClouds " << "FPS: " << std::setprecision(4) << 1.0f / timeSinceLastFrame << " ms: " << timeSinceLastFrame* 1000.0f;
	glfwSetWindowTitle(stringstream.str().c_str());

	return true;
}

bool CloudyClouds::display(float timeSinceLastFrame)
{
	// update global matrices
	int offset = 0;
	Matrix4 viewProjection = camera->getViewMatrix() * projectionMatrix;
	glBindBuffer(GL_UNIFORM_BUFFER, uboView);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(float) * 4 * 4, camera->getViewMatrix());	offset += sizeof(float) * 4 * 4;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(float) * 4 * 4, viewProjection);			offset += sizeof(float) * 4 * 4;
	Matrix4 inverseViewProjection(viewProjection.invert());
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(float) * 4 * 4, inverseViewProjection);	offset += sizeof(float) * 4 * 4;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(float) * 3, camera->getPosition());		offset += sizeof(float) * 4;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(float) * 3, Vector3(camera->getViewMatrix().m11, camera->getViewMatrix().m21, camera->getViewMatrix().m31)); offset += sizeof(float) * 4;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(float) * 3, Vector3(camera->getViewMatrix().m12, camera->getViewMatrix().m22, camera->getViewMatrix().m32)); offset += sizeof(float) * 4;
	glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(float) * 3, -Vector3(camera->getViewMatrix().m13, camera->getViewMatrix().m23, camera->getViewMatrix().m33)); offset += sizeof(float) * 4;


	// update timings
	glBindBuffer(GL_UNIFORM_BUFFER, uboTimings);
	float timings[2] = { static_cast<float>(glfwGetTime()), timeSinceLastFrame };
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 2, timings);	// update view

	// clear scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	Vector3 lightDirection(1, -0.6f, 0);
	lightDirection.normalize();


	background->display(lightDirection, clouds->getLightViewProjection(), clouds->getLightDistancePlane_Norm(), clouds->getFOMTexture0(), clouds->getFOMTexture1(), clouds->getFOMSampler());
	clouds->display(timeSinceLastFrame, inverseViewProjection, camera->getViewMatrix(), camera->getDirection(), camera->getPosition(), lightDirection);

	// next frame
	glfwSwapBuffers();

	return true;
}