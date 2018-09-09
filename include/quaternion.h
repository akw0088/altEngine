//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

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
