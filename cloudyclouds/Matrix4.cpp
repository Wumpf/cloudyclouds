#include "stdafx.h"
#include "Matrix4.h"
#include <math.h>


Matrix4 Matrix4::projectionPerspective(float FOV, float aspect, float nearPlane, float farPlane)
{
	// http://wiki.delphigl.com/index.php/gluPerspective
	float yScale = 1.0f / std::tan(FOV);
	float xScale = yScale / aspect;
	float nearSubFar = nearPlane - farPlane;
	return Matrix4(xScale,     0,          0,               0,
					0,        yScale,       0,               0,
					0,          0,       (farPlane+nearPlane)/nearSubFar, -1,
					0,          0,       2*nearPlane*farPlane/nearSubFar,     0);
}
/*
Matrix4 Matrix4::projectionOrthogonal(float width, float height, float nearPlane, float farPlane)
{
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb205346%28v=vs.85%29.aspx
	float farSubNear = farPlane - nearPlane;
	return Matrix4(2/width,  0,    0,           0,
					0,    2/height,  0,           0,
					0,    0,    1/farSubNear,   0,
					0,    0,   -nearPlane/farSubNear,  1);

}*/

Matrix4 Matrix4::camera(const Vector3& vPos, const Vector3& vLockAt, const Vector3& vUp)
{
	Vector3 zaxis = (vLockAt - vPos).normalizeCpy();
	Vector3 xaxis = Vector3::cross(zaxis, vUp).normalizeCpy();
	Vector3 yaxis = Vector3::cross(xaxis, zaxis);
    return Matrix4(	 xaxis.x,           yaxis.x,          -zaxis.x,        0,
					 xaxis.y,           yaxis.y,          -zaxis.y,        0,
					 xaxis.z,           yaxis.z,          -zaxis.z,        0,
					 -Vector3::dot(xaxis, vPos),  -Vector3::dot(yaxis, vPos),  -Vector3::dot(-zaxis, vPos),  1);
}