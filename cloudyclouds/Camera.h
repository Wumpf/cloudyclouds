#pragma once

#include "Matrix4.h"

class Camera
{
public:
	Camera();
	~Camera();

	void update(float frameTime);

	const Matrix4& getViewMatrix() const { return matrix; }

private:
	Matrix4 matrix;

	Vector3 cameraDirection;
	Vector3 cameraPosition;

	int lastMousePosX;
	int lastMousePosY;

	float rotX;
	float rotY;

	static const float rotSpeed;
	static const float moveSpeed;
};

