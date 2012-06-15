#pragma once

#include "Matrix4.h"

class Camera
{
public:
	Camera();
	~Camera();

	void update(float frameTime);

	void setPosition(const Vector3& pos) 	{ cameraPosition = pos; }

	const Matrix4& getViewMatrix() const	{ return matrix; }
	const Vector3 getPosition() const		{ return cameraPosition; }

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

