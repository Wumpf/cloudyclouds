#pragma once

class Vector3
{
public:
	// value
	union
	{
		// as standard geometric vector
		struct
		{
			float x;
			float y;
			float z;
		};

		// as texture cordinate
		struct
		{
			float u;
			float v;
			float w;
		};

		// as float color
		struct
		{
			float r;
			float g;
			float b;
		};

		// as array
		float a[3];
	};

	// constructors
	Vector3() : x(0), y(0), z(0) {}
	Vector3(const Vector3& v) : x(v.x), y(v.y), z(v.z) {}
	Vector3(float f) : x(f), y(f), z(f) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vector3(const float* components) : x(components[0]), y(components[1]), z(components[2])	{}


	// --- operators ---

	// assignment operators
	inline const Vector3& operator =  (const Vector3& v);
	inline const Vector3& operator += (const Vector3& v);
	inline const Vector3& operator -= (const Vector3& v);
	inline const Vector3& operator *= (const Vector3& v);	// nonsense for real vectors.. but what about colors?
	inline const Vector3& operator *= (float f);
	inline const Vector3& operator /= (float f);
		
	// arithmethic operators
	const Vector3 operator - () const					{ return Vector3(-x, -y, -z); }
	const Vector3 operator - (const Vector3& rk) const	{ return Vector3(x - rk.x, y - rk.y, z - rk.z); }
	const Vector3 operator + (const Vector3& rk) const	{ return Vector3(x + rk.x, y + rk.y, z + rk.z); }
	const Vector3 operator * (const Vector3& rk) const	{ return Vector3(x * rk.x, y * rk.y, z * z); }
	const Vector3 operator * (float f) const			{ return Vector3(x * f, y * f, z * f); }
	const Vector3 operator / (const Vector3& rk) const	{ return Vector3(x / rk.x, y / rk.y, z / rk.z); }
	const Vector3 operator / (float f) const			{ return Vector3(x / f, y / f, z / f); }

	// comparing operators
	const bool operator == (const Vector3& rk) { return x == rk.x && y == rk.y && z == rk.z; }
	const bool operator != (const Vector3& rk) { return x != rk.x || y != rk.y || z != rk.z; }

	// casting
	operator float* ()		 		{ return static_cast<float*>(a); }
	operator const float* () const	{ return static_cast<const float*>(a); }


	// --- functions ---

	// functions with one vector
	inline float			length() const;
	inline float			lengthSq() const;
	inline Vector3			normalizeCpy() const;
	inline const Vector3&	normalize();

	// functions with 2 vectors
	static Vector3	cross(const Vector3& v1, const Vector3& v2);
	static float	dot(const Vector3& v1, const Vector3& v2);
	static float	angle(const Vector3& v1, const Vector3& v2);


	// --- special values ---
	static const Vector3 zero;
	static const Vector3 unitX;
	static const Vector3 unitY;
	static const Vector3 unitZ;
};

// more arithmetic operators
inline Vector3 operator * (float f, const Vector3& v) { return Vector3(v.x*f, v.y*f, v.z*f); }
inline Vector3 operator / (float f, const Vector3& v) { return Vector3(v.x/f, v.y/f, v.z/f); }