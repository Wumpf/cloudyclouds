#include "stdafx.h"
#include "Vector3.h"
#include "Matrix4.h"

// definition of the "special values"
const Vector3 Vector3::zero (0, 0, 0);
const Vector3 Vector3::unitX(1, 0, 0);
const Vector3 Vector3::unitY(0, 1, 0);
const Vector3 Vector3::unitZ(0, 0, 1);

inline const Vector3& Vector3::operator *= (const Vector3& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

inline const Vector3& Vector3::operator *= (float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

inline const Vector3& Vector3::operator /= (float f)
{
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

float Vector3::length() const
{
	return sqrt(x*x+y*y+z*z);
}
	
float Vector3::lengthSq() const
{
	return x*x+y*y+z*z;
}

inline Vector3 Vector3::normalizeCpy() const
{
	float l = sqrt(x*x+y*y+z*z);
	return Vector3(x/l, y/l, z/l);
}

Vector3 Vector3::cross(const Vector3& v1, const Vector3& v2)
{
	return Vector3( v1.y*v2.z - v2.y*v1.z,
					v1.z*v2.x - v2.z*v1.x,
					v1.x*v2.y - v2.x*v1.y);
}

float Vector3::dot(const Vector3& v1, const Vector3& v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

float Vector3::angle(const Vector3& v1, const Vector3& v2)
{
	return acos(dot(v1,v2) / dot(v1.normalizeCpy(), v2.normalizeCpy()));
}

Vector3 operator * (const Vector3& v, const Matrix4 &m )
	{
		Vector3 r;

		float fInvW = 1.0f / ( m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] );

		r.x = ( m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] ) * fInvW;
		r.y = ( m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] ) * fInvW;
		r.z = ( m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] ) * fInvW;

		return r;
	}