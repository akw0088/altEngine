#include "vector.h"
#include "matrix.h"
#include <cmath>

void matrix3::normalize()
{
	vec3 x,y,z;

	x.x = m[0];
	x.y = m[1];
	x.z = m[2];

	y.x = m[3];
	y.y = m[4];
	y.z = m[5];

	x.normalize();
	z = vec3::crossproduct(x, y).normalize();
	y = vec3::crossproduct(z, x).normalize();

	m[0] = x.x;
	m[1] = x.y;
	m[2] = x.z;

	m[3] = y.x;
	m[4] = y.y;
	m[5] = y.z;

	m[6] = z.x;
	m[7] = z.y;
	m[8] = z.z;
}

matrix3 &matrix3::operator=(const matrix3 &mat)
{
	m[0] = mat.m[0];
	m[1] = mat.m[1];
	m[2] = mat.m[2];

	m[3] = mat.m[3];
	m[4] = mat.m[4];
	m[5] = mat.m[5];
	
	m[6] = mat.m[6];
	m[7] = mat.m[7];
	m[8] = mat.m[8];
	return *this;
}

matrix3 matrix3::operator+(const matrix3 &mat)
{
	matrix3 result;

	result.m[0] = m[0] + mat.m[0];
	result.m[1] = m[1] + mat.m[1];
	result.m[2] = m[2] + mat.m[2];

	result.m[3] = m[3] + mat.m[3];
	result.m[4] = m[4] + mat.m[4];
	result.m[5] = m[5] + mat.m[5];
	
	result.m[6] = m[6] + mat.m[6];
	result.m[7] = m[7] + mat.m[7];
	result.m[8] = m[8] + mat.m[8];

	return result;
}

matrix3 matrix3::operator-(const matrix3 &mat)
{
	matrix3 result;

	result.m[0] = m[0] - mat.m[0];
	result.m[1] = m[1] - mat.m[1];
	result.m[2] = m[2] - mat.m[2];

	result.m[3] = m[3] - mat.m[3];
	result.m[4] = m[4] - mat.m[4];
	result.m[5] = m[5] - mat.m[5];
	
	result.m[6] = m[6] - mat.m[6];
	result.m[7] = m[7] - mat.m[7];
	result.m[8] = m[8] - mat.m[8];
	return result;
}

matrix3 matrix3::operator*(const float scalar)
{
	matrix3 result;

	result.m[0] = m[0] * scalar;
	result.m[1] = m[1] * scalar;
	result.m[2] = m[2] * scalar;

	result.m[3] = m[3] * scalar;
	result.m[4] = m[4] * scalar;
	result.m[5] = m[5] * scalar;
	
	result.m[6] = m[6] * scalar;
	result.m[7] = m[7] * scalar;
	result.m[8] = m[8] * scalar;
	return result;
}

vec3 matrix3::operator*(const vec3 &vec)
{
	vec3 result;

	result.x = vec.x * m[0] + vec.y * m[3] + vec.z * m[6];
	result.y = vec.x * m[1] + vec.y * m[4] + vec.z * m[7];
	result.x = vec.x * m[2] + vec.y * m[5] + vec.z * m[8];
	return result;
}

matrix3 matrix3::operator*(const matrix3 &mat)
{
	matrix3 result;

	result.m[0] = m[0] * mat.m[0] + m[1] * mat.m[3] + m[2] * mat.m[6];
	result.m[1] = m[0] * mat.m[1] + m[1] * mat.m[4] + m[2] * mat.m[7];
	result.m[2] = m[0] * mat.m[2] + m[1] * mat.m[5] + m[2] * mat.m[8];

	result.m[3] = m[3] * mat.m[0] + m[4] * mat.m[3] + m[5] * mat.m[6];
	result.m[4] = m[3] * mat.m[1] + m[4] * mat.m[4] + m[5] * mat.m[7];
	result.m[5] = m[3] * mat.m[2] + m[4] * mat.m[5] + m[5] * mat.m[8];

	result.m[6] = m[6] * mat.m[0] + m[7] * mat.m[3] + m[8] * mat.m[6];
	result.m[7] = m[6] * mat.m[1] + m[7] * mat.m[4] + m[8] * mat.m[7];
	result.m[8] = m[6] * mat.m[2] + m[7] * mat.m[5] + m[8] * mat.m[8];
	return result;
}

