#include "quaternion.h"
#include "sin_table.h"

quaternion::quaternion()
{
	s = 0;
	x = 0.0f;
	y = 1.0f;
	z = 0.0f;
}

quaternion::quaternion(float s, vec3 v)
{
	quaternion::s = s;
	x = v.x;
	y = v.y;
	z = v.z;
}

quaternion::quaternion(float s, float x, float y, float z)
{
	quaternion::s = s;
	quaternion::x = x;
	quaternion::y = y;
	quaternion::z = z;
}

matrix3 quaternion::to_matrix()
{
	matrix3 matrix;

	matrix.m[0] = 1 - (2 * y * y) - (2 * z * z);
	matrix.m[1] = (2 * x * y) - (2 * s * z);
	matrix.m[2] = (2 * x * z) + (2 * s * y);

	matrix.m[3] = (2 * x * y) + (2 * s * z);
	matrix.m[4] = 1 - (2 * x * x) - (2 * z * z);
	matrix.m[5] = (2 * y * z) - (2 * s * x);

	matrix.m[6] = (2 * x * z) - (2 * s * y);
	matrix.m[7] = (2 * y * z) + (2 * s * x);
	matrix.m[8] = 1 - (2 * x * x) - (2 * y * y);

	return matrix;
}

vec3 quaternion::rotate(float delta, vec3 axis, vec3 vector)
{
	quaternion	v(0.0f, vector);
	quaternion	result_quaternion;
	vec3		result_vector;
	float		cosval = (float)fcos(delta / 2);
	float		sinval = (float)fsin(delta / 2);

	s = cosval;
	x = sinval * axis.x;
	y = sinval * axis.y;
	z = sinval * axis.z;

	quaternion qinv(s, -x, -y, -z);
	result_quaternion = (*this * v * qinv);
	result_vector.x = result_quaternion.x;
	result_vector.y = result_quaternion.y;
	result_vector.z = result_quaternion.z;
	return result_vector;
}

quaternion quaternion::operator*(const quaternion &q)
{
	quaternion result;

	//[s1,v1][s2,v2] = [s1s2 - v1 dot v2, s1v2 + s2v1 + v1 cross v2]
	result.s = (s * q.s) - (x * q.x + y * q.y + z * q.z);
	result.x = (s * q.x) + (q.s * x) + (y * q.z - z * q.y);
	result.y = (s * q.y) + (q.s * y) + (z * q.x - x * q.z);
	result.z = (s * q.z) + (q.s * z) + (x * q.y - y * q.x);
	return result;
}

quaternion &quaternion::operator*=(const quaternion &q)
{
	//[s1,v1][s2,v2] = [s1s2 - v1 dot v2, s1v2 + s2v1 + v1 cross v2]
	s = (s * q.s) - (x * q.x + y * q.y + z * q.z);
	x = (s * q.x) + (q.s * x) + (y * q.z - z * q.y);
	y = (s * q.y) + (q.s * y) + (z * q.x - x * q.z);
	z = (s * q.z) + (q.s * z) + (x * q.y - y * q.x);
	return *this;
}

quaternion quaternion::operator*(const vec3 &vec)
{
	quaternion q;

	q.s = 0;
	q.x = vec.x;
	q.y = vec.y;
	q.z = vec.z;

	return q * (*this);
}

quaternion &quaternion::operator=(const quaternion &q)
{
	s = q.s;
	x = q.x;
	y = q.y;
	z = q.z;

	return *this;
}

quaternion quaternion::operator+(const quaternion &q)
{
	quaternion result;

	result.s = s + q.s;
	result.x = x + q.x;
	result.y = y + q.y;
	result.z = z + q.z;
	return result;
}

quaternion quaternion::operator-(const quaternion &q)
{
	quaternion result;

	result.s = s - q.s;
	result.x = x - q.x;
	result.y = y - q.y;
	result.z = z - q.z;
	return result;
}

quaternion quaternion::operator*(const float scalar)
{
	quaternion result;

	result.s = s * scalar;
	result.x = x * scalar;
	result.y = y * scalar;
	result.z = z * scalar;
	return result;
}
quaternion &quaternion::operator+=(const quaternion &q)
{
	s += q.s;
	x += q.x;
	y += q.y;
	z += q.z;
	return *this;
}

quaternion &quaternion::operator-=(const quaternion &q)
{
	s -= q.s;
	x -= q.x;
	y -= q.y;
	z -= q.z;
	return *this;
}

