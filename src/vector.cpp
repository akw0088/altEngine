#include "vector.h"
float InvSqrt(float x);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

vec4::vec4()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 0.0f;
}

vec4::vec4(float x, float y, float z, float w)
{
	vec4::x = x;
	vec4::y = y;
	vec4::z = z;
	vec4::w = w;
}

float vec4::magnitude()
{
	return (1.0f / InvSqrt(x * x + y * y + z * z));
}

vec4 &vec4::normalize()
{
	float invmag = InvSqrt(x * x + y * y + z * z);
	x *= invmag;
	y *= invmag;
	z *= invmag;
	return *this;
}

vec4 &vec4::operator*=(const float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;
	return *this;
}

vec4 &vec4::operator+=(const vec4 &vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;
	w += vec.w;
	return *this;
}

vec4 &vec4::operator+=(const float scalar)
{
	x += scalar;
	y += scalar;
	z += scalar;
	w += scalar;
	return *this;
}


vec3::vec3()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

vec3::vec3(float x, float y, float z)
{
	vec3::x = x;
	vec3::y = y;
	vec3::z = z;
}

vec3::vec3(vec4 vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

float vec3::magnitude()
{
	return (1.0f / InvSqrt(x * x + y * y + z * z));
}

vec3 &vec3::normalize()
{
	float invmag = InvSqrt(x * x + y * y + z * z);
	x *= invmag;
	y *= invmag;
	z *= invmag;
	return *this;
}

vec3 &vec3::operator=(const vec3 &vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;

	return *this;
}

vec3 vec3::operator+(const vec3 &vector) const
{
	vec3 temp(this->x, this->y, this->z);

	temp.x += vector.x;
	temp.y += vector.y;
	temp.z += vector.z;

	return temp;
}

vec3 vec3::operator-(const vec3 &vector) const
{
	vec3 temp(this->x, this->y, this->z);

	temp.x -= vector.x;
	temp.y -= vector.y;
	temp.z -= vector.z;

	return temp;
}

vec3 vec3::operator-() const
{
	vec3 temp(-this->x, -this->y, -this->z);
	return temp;
}

float vec3::operator*(const vec3 &vector)
{
	return (x * vector.x + y * vector.y + z * vector.z);
}

float vec3::operator/(const vec3 &vector)
{
	return (x / vector.x + y / vector.y + z / vector.z);
}

vec3 vec3::operator*(const float scalar)
{
	return vec3(x * scalar, y * scalar, z * scalar);
}

vec3 vec3::operator/(const float scalar)
{
	return vec3(x / scalar, y / scalar, z / scalar);
}

vec3 &vec3::operator+=(const vec3 &vec)
{
	x += vec.x;
	y += vec.y;
	z += vec.z;
	return *this;
}

vec3 &vec3::operator-=(const vec3 &vec)
{
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	return *this;
}

vec3 vec3::crossproduct(const vec3 &VecA, const vec3 &VecB)
{
	vec3	result;

	result.x = VecA.y * VecB.z - VecA.z * VecB.y;
	result.y = VecA.z * VecB.x - VecA.x * VecB.z;
	result.z = VecA.x * VecB.y - VecA.y * VecB.x;
	return result;
}

bool vec3::operator==(const vec3 &vector)
{
	if ((x == vector.x) && (y == vector.y) && (z == vector.z))
		return true;
	else
		return false;
}

vec3 &vec3::operator*=(const float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}


vec2::vec2()
{
	x = 0.0f;
	y = 0.0f;
}

vec2::vec2(float x, float y)
{
	vec2::x = x;
	vec2::y = y;
}

vec2 &vec2::operator=(const vec2 &vector)
{
	x = vector.x;
	y = vector.y;

	return *this;
}

vec2 vec2::operator+(const vec2 &vector)
{
	vec2 temp(this->x, this->y);

	temp.x += vector.x;
	temp.y += vector.y;

	return temp;
}

vec2 vec2::operator-(const vec2 &vector)
{
	vec2 temp(this->x, this->y);

	temp.x -= vector.x;
	temp.y -= vector.y;

	return temp;
}

vec2 &vec2::operator*=(const float scalar)
{
	x *= scalar;
	y *= scalar;
	return *this;
}
