#include "quaternion.h"
//#include <cmath> // for acos
double fsin(double rad);
double fcos(double rad);
double facos(double rad);
float newtonSqrt(float x);


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

quaternion quaternion::conjugate() const
{
	quaternion c;

	c.s = s;
	c.x = -x;
	c.y = -y;
	c.z = -z;
	return c;
}

quaternion &quaternion::compute_w()
{
	float t = 1 - x*x - y*y - z*z;

	if (t < 0.0f)
	{
		s = 0.0f;
	}
	else
	{
		s = -newtonSqrt(t);
	}

	return *this;
}

quaternion &quaternion::normalize()
{
	float inv_norm = 1.0f / magnitude();
	s *= inv_norm;
	x *= inv_norm;
	y *= inv_norm;
	z *= inv_norm;
	return *this;
}

float quaternion::magnitude() const
{
	return newtonSqrt(s*s + x*x + y*y + z*z);
}

matrix3 quaternion::to_matrix()
{
	matrix3 matrix;

	matrix.m[0] = 1 - 2 * (y * y + z * z);
	matrix.m[1] = 2 * (x * y - s * z);
	matrix.m[2] = 2 * (x * z + s * y);

	matrix.m[3] = 2 * ( x * y + s * z);
	matrix.m[4] = 1 - 2 *( x * x + z * z);
	matrix.m[5] = 2 * (y * z -  s * x);

	matrix.m[6] = 2 * (x * z - s * y);
	matrix.m[7] = 2 * (y * z + s * x);
	matrix.m[8] = 1 - 2 * (x * x + y * y);

	return matrix;
}

void quaternion::to_quat(matrix3 &mat)
{
	float		trace;
	int     	i;
	int		j;
	int		k;

	static int 	next[3] = { 1, 2, 0 };

	trace = mat.m[0] + mat.m[4] + mat.m[8];
	if (trace > 0.0f)
	{
		s = newtonSqrt(1.0f + mat.m[0] + mat.m[4] + mat.m[8]) / 2.0f;
		float s4 = (4.0f * s);
		x = (mat.m[7] - mat.m[5]) / s4;
		y = (mat.m[2] - mat.m[6]) / s4;
		z = (mat.m[3] - mat.m[1]) / s4;
	}
	else if ((mat.m[0] > mat.m[4]) & (mat.m[0] > mat.m[8]))
	{
		float root = newtonSqrt(1.0 + mat.m[0] - mat.m[4] - mat.m[8]) * 2; // S=4*qx 
		s = (mat.m[7] - mat.m[5]) / root;
		x = 0.25 * root;
		y = (mat.m[1] + mat.m[3]) / root;
		z = (mat.m[2] + mat.m[6]) / root;
	}
	else if (mat.m[3] > mat.m[8])
	{
		float root = newtonSqrt(1.0f + mat.m[4] - mat.m[0] - mat.m[8]) * 2; // S=4*qy
		s = (mat.m[2] - mat.m[6]) / root;
		x = (mat.m[1] + mat.m[3]) / root;
		y = 0.25 * root;
		z = (mat.m[5] + mat.m[7]) / root;
	}
	else
	{
		float root = newtonSqrt(1.0 + mat.m[8] - mat.m[0] - mat.m[4]) * 2; // S=4*qz
		s = (mat.m[3] - mat.m[1]) / root;
		x = (mat.m[2] + mat.m[6]) / root;
		y = (mat.m[5] + mat.m[7]) / root;
		z = 0.25 * root;
	}
}


vec3 quaternion::rotate(float delta, const vec3 &axis, const vec3 &vector)
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

quaternion quaternion::operator*(const quaternion &q) const
{
	quaternion result;

	//[s1,v1][s2,v2] = [s1s2 - v1 dot v2, s1v2 + s2v1 + v1 cross v2]
	result.s = (s * q.s) - x * q.x - y * q.y - z * q.z;
	result.x = s * q.x + q.s * x + y * q.z - z * q.y;
	result.y = s * q.y + q.s * y + z * q.x - x * q.z;
	result.z = s * q.z + q.s * z + x * q.y - y * q.x;
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

quaternion quaternion::operator*(const vec3 &vec) const
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

quaternion quaternion::operator+(const quaternion &q) const
{
	quaternion result;

	result.s = s + q.s;
	result.x = x + q.x;
	result.y = y + q.y;
	result.z = z + q.z;
	return result;
}

quaternion quaternion::operator-(const quaternion &q) const
{
	quaternion result;

	result.s = s - q.s;
	result.x = x - q.x;
	result.y = y - q.y;
	result.z = z - q.z;
	return result;
}

quaternion quaternion::operator-() const
{
	quaternion result;

	result.s = -s;
	result.x = -x;
	result.y = -y;
	result.z = -z;
	return result;
}

quaternion quaternion::operator*(const float scalar) const
{
	quaternion result;

	result.s = s * scalar;
	result.x = x * scalar;
	result.y = y * scalar;
	result.z = z * scalar;
	return result;
}

quaternion quaternion::operator/(const float scalar) const
{
	quaternion result;

	result.s = s / scalar;
	result.x = x / scalar;
	result.y = y / scalar;
	result.z = z / scalar;
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


void quaternion::slerp(const quaternion &p, const quaternion &q, float time, quaternion &result)
{
	if (time <= 0.0f)
	{
		result = p;
		return;
	}
	if (time >= 1.0f)
	{
		result = q;
		return;
	}

	float cos_omega = p.s * q.s + p.x * q.x + p.y * q.y + p.z * q.z;
	quaternion temp = q;
	
	if (cos_omega < 0.0f)
	{
		temp = -temp;
		cos_omega = -cos_omega;
    }

	/*
	// I really dont think this is faster, but I'll check it out later
	float sin_omega = sqrt(1.0f - cos_omega * cos_omega);
	float omega = atan2(sin_omega, cos_omega);
	float inv_sin = 1.0f / sin_omega;
	float k0 = sin((1.0f - time) * omega) * inv_sin;
	float k1 = sin(time * omega) * inv_sin;
	result = p * k0 + temp * k1;
	*/

	float omega = (float)facos(p.s * q.s + p.x * q.x + p.y * q.y + p.z * q.z);

	result = (p * ((float)fsin(omega * (1.0f - time))) + (q * (float)fsin(omega*time))) / (float)fsin(omega);
}
