#include "quaternion.h"

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

