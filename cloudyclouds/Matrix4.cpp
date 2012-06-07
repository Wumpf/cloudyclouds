#include "stdafx.h"
#include "Matrix4.h"
#include <math.h>


Matrix4 Matrix4::projectionPerspective(float FOV, float aspect, float nearPlane, float farPlane)
{
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb205350%28v=vs.85%29.aspx
	float yScale = 1.0f / std::tan(FOV/2);
	float xScale = yScale / aspect;
	float farSubNear = farPlane - nearPlane;
	return Matrix4(xScale,     0,          0,               0,
					0,        yScale,       0,               0,
					0,          0,       farPlane/farSubNear,         1,
					0,          0,       -nearPlane*farPlane/farSubNear,     0);
}

Matrix4 Matrix4::projectionOrthogonal(float width, float height, float nearPlane, float farPlane)
{
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb205346%28v=vs.85%29.aspx
	float farSubNear = farPlane - nearPlane;
	return Matrix4(2/width,  0,    0,           0,
					0,    2/height,  0,           0,
					0,    0,    1/farSubNear,   0,
					0,    0,   -nearPlane/farSubNear,  1);

}

Matrix4 Matrix4::camera(const Vector3& vPos, const Vector3& vLockAt, const Vector3& vUp)
{
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb205342%28v=vs.85%29.aspx

	Vector3 zaxis = (vLockAt - vPos).normalizeCpy();
	Vector3 xaxis = Vector3::cross(vUp, zaxis).normalizeCpy();
	Vector3 yaxis = Vector3::cross(zaxis, xaxis);
    return Matrix4(
					 xaxis.x,           yaxis.x,           zaxis.x,          0,
					 xaxis.y,           yaxis.y,           zaxis.y,          0,
					 xaxis.z,           yaxis.z,           zaxis.z,          0,
					-Vector3::dot(xaxis, vLockAt),  -Vector3::dot(yaxis, vLockAt),  -Vector3::dot(zaxis, vLockAt),  1);
}