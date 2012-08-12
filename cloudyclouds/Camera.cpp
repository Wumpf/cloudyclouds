#include "stdafx.h"
#include "Camera.h"

const float Camera::rotSpeed = 0.01f;
const float Camera::moveSpeed = 16.0f;

Camera::Camera() :
	lastMousePosX(0),
	lastMousePosY(0),
	rotX(0.0f),
	rotY(0.0f),
	cameraPosition(2.f)
{
}

Camera::~Camera()
{
}

void Camera::update(float timeSinceLastFrame)
{
	int newMousePosX, newMousePosY;
	glfwGetMousePos(&newMousePosX, &newMousePosY);

	rotX += -rotSpeed * (newMousePosX - lastMousePosX);
	rotY += rotSpeed * (newMousePosY - lastMousePosY);

	float forward = (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey('w') == GLFW_PRESS) ? 1.0f : 0.0f;
	float back	  = (glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey('s') == GLFW_PRESS) ? 1.0f : 0.0f;
	float left	  = (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey('a') == GLFW_PRESS) ? 1.0f : 0.0f;
	float right	  = (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey('d') == GLFW_PRESS) ? 1.0f : 0.0f;

	if(glfwGetKey(GLFW_KEY_RCTRL))
		forward *= 5.0f;

	cameraDirection = Vector3(sinf(rotX) * cosf(rotY), sinf(rotY), cosf(rotX) * cosf(rotY));
	Vector3 cameraLeft = Vector3::cross(cameraDirection, Vector3(0,1,0));

	cameraPosition += ((forward - back) * cameraDirection + (right -left) * cameraLeft) * moveSpeed * timeSinceLastFrame;


	matrix = Matrix4::camera(cameraPosition, cameraPosition + cameraDirection);
	
	lastMousePosX = newMousePosX;
	lastMousePosY = newMousePosY;
}