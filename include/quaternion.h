#include "vector.h"
#include "matrix.h"
#include <cmath>

#ifndef QUATERNION_H
#define QUATERNION_H

class quaternion
{
public:
	matrix3 to_matrix();

	quaternion();
	quaternion(float s, vec3 v);
	quaternion(float s, float x, float y, float z);

	quaternion &operator=(const quaternion &q);
	quaternion operator+(const quaternion &q);
	quaternion operator-(const quaternion &q);
	quaternion operator*(const float scalar);
	quaternion operator*(const vec3 &vec);
	quaternion operator*(const quaternion &q);
	quaternion &operator*=(const quaternion &q);
	quaternion &operator+=(const quaternion &q);
	quaternion &operator-=(const quaternion &q);

	vec3 rotate(float delta, vec3 axis, vec3 vector);

	float s;
	float x;
	float y;
	float z;
};

#endif
