#include "vector.h"
#include "matrix.h"

#ifndef QUATERNION_H
#define QUATERNION_H

class quaternion
{
public:
	matrix3 to_matrix();
	void to_quat(matrix3 &mat);

	quaternion();
	quaternion(float s, vec3 v);
	quaternion(float s, float x, float y, float z);
	quaternion conjugate() const;
	quaternion &compute_w();
	quaternion &normalize();
	float magnitude() const;

	quaternion &operator=(const quaternion &q);
	quaternion operator+(const quaternion &q) const ;
	quaternion operator-(const quaternion &q) const;
	quaternion operator-() const;
	quaternion operator*(const float scalar) const;
	quaternion operator/(const float scalar) const;
	quaternion operator*(const vec3 &vec) const;
	quaternion operator*(const quaternion &q) const;
	quaternion &operator*=(const quaternion &q);
	quaternion &operator+=(const quaternion &q);
	quaternion &operator-=(const quaternion &q);

	static void slerp(const quaternion &q, const quaternion &p, float time, quaternion &result);

	vec3 rotate(float delta, const vec3 &axis, const vec3 &vector);

	float s;
	float x;
	float y;
	float z;
};

#endif
