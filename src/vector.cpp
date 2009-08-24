#include "math.h"
#include "vector.h"

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

float vec3::magnitude()
{
	return sqrt(x * x + y * y + z * z);
}

vec3 &vec3::normalize()
{
	float mag;

	mag = this->magnitude();
	if (mag)
	{
		x /= mag;
		y /= mag;
		z /= mag;
	}
	return *this;
}

vec3 &vec3::operator=(const vec3 &vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;

	return *this;
}

vec3 vec3::operator+(const vec3 &vector)
{
	vec3 temp(this->x, this->y, this->z);

	temp.x += vector.x;
	temp.y += vector.y;
	temp.z += vector.z;

	return temp;
}

vec3 vec3::operator-(const vec3 &vector)
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

vec3 vec3::operator*(const float scalar)
{
	return vec3(x * scalar, y * scalar, z * scalar);
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

bool vec3::operator==(const vec3 &vector)
{
	if ((x == vector.x) && (y == vector.y) && (z == vector.z))
		return true;
	else
		return false;
}

