#include "vector.h"
#include <float.h>

float InvSqrt(float x);
float newtonSqrt(float x);
double facos(double x);

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
	return newtonSqrt(x * x + y * y + z * z);
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
	return newtonSqrt(x * x + y * y + z * z);
}

float vec3::magnitudeSq()
{
	return x * x + y * y + z * z;
}

vec3 &vec3::normalize()
{
	float invmag = InvSqrt(x * x + y * y + z * z);
	if (invmag < FLT_MAX && invmag > FLT_MIN)
	{
		x *= invmag;
		y *= invmag;
		z *= invmag;
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

vec3 vec3::operator+(const vec3 &vector) const
{
	vec3 temp(x, y, z);

	temp.x += vector.x;
	temp.y += vector.y;
	temp.z += vector.z;

	return temp;
}

vec3 vec3::operator-(const vec3 &vector) const
{
	vec3 temp(x, y, z);

	temp.x -= vector.x;
	temp.y -= vector.y;
	temp.z -= vector.z;

	return temp;
}

vec3 vec3::operator-() const
{
	vec3 temp(-x, -y, -z);
	return temp;
}

float vec3::operator*(const vec3 &vector) const
{
	return (x * vector.x + y * vector.y + z * vector.z);
}

float vec3::operator/(const vec3 &vector) const
{
	return (x / vector.x + y / vector.y + z / vector.z);
}

vec3 vec3::operator*(const float scalar) const
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

float abs32(float);

bool vec3::operator==(const vec3 &vector) const
{
	if ((abs32(x - vector.x) < 0.0001f && (abs32(y - vector.y) < 0.0001f) && (abs32(z - vector.z) < 0.0001f)))
		return true;
	else
		return false;
}

vec3 vec3::project(const vec3 &a) const
{
	const vec3 *b = this;
	vec3 result;
	// dot product yield a float, so really scaling B by length of A
	// [a dot b / (b dot b)] times B
	result = (*b) * (a * (*b)) / ((*b) * (*b));
	return result;
}

vec3 &vec3::operator*=(const float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

float vec3::angle(const vec3 &r) const
{
	float m = newtonSqrt( (x * x + y * y + z * z) * (r.x * r.x + r.y * r.y + r.z * r.z));
	float angle = facos((*this) * r / m);
	return angle;
}


// reflect = vec - 2 (vec dot norm) * norm
vec3 vec3::reflect(const vec3& normal) const
{
	const vec3 *vec = this;

	float d = ((*vec) * normal) * 2.0f;
	return (*vec) - normal * d;
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

vec2 vec2::operator/(const float scalar)
{
	return vec2(x / scalar, y / scalar);
}

float vec2::operator/(const vec2 &vector) const
{
	return (x / vector.x + y / vector.y);
}

vec2 vec2::operator+(const vec2 &vector) const
{
	vec2 temp(x, y);

	temp.x += vector.x;
	temp.y += vector.y;

	return temp;
}

vec2 vec2::operator-(const vec2 &vector) const
{
	vec2 temp(x, y);

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

vec2 &vec2::normalize()
{
	float invmag = InvSqrt(x * x + y * y );
	if (invmag < FLT_MAX && invmag > FLT_MIN)
	{
		x *= invmag;
		y *= invmag;
	}
	return *this;
}

float vec2::magnitude()
{
	return newtonSqrt(x * x + y * y);
}

float vec2::magnitudeSq()
{
	return x * x + y * y;
}

vec2 vec2::operator*(const float scalar) const
{
	return vec2(x * scalar, y * scalar);
}

float vec2::operator*(const vec2 &vector) const
{
	return (x * vector.x + y * vector.y);
}


float vec2::angle(const vec2 &r) const
{
	float m = newtonSqrt( (x * x + y * y) * (r.x * r.x + r.y * r.y) );
	float angle = facos((*this) * r / m);
	return angle;
}

// reflect = vec - 2 (vec dot norm) * norm
vec2 vec2::reflect(const vec2& normal) const
{
	const vec2 *vec = this;

	float d = ((*vec) * normal) * 2.0f;
	return (*vec) - normal * d;
}


vec2 vec2::project(const vec2 &a) const
{
	const vec2 *b = this;
	vec2 result;
	// dot product yield a float, so really scaling B by length of A
	// [a dot b / (b dot b)] times B
	result = (*b) * (a * (*b)) / ((*b) * (*b));
	return result;
}