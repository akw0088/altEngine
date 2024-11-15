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

#ifndef MATRIX_H
#define MATRIX_H

float fsin(float rad);
float fcos(float rad);

class matrix4;

class matrix3
{
public:
	matrix3();
	matrix3(float m0, float m1, float m2,
		float m3, float m4, float m5,
		float m6, float m7, float m8);

	void matrix4to3(matrix4 &mat);
	matrix3 &operator=(const matrix3 &q);
	matrix3 operator+(const matrix3 &q);
	matrix3 operator-(const matrix3 &q);
	matrix3 operator*(const float scalar);
	vec3 operator*(const vec3 &vec) const;
	matrix3 operator*(const matrix3 &q);
	float det() const;
	matrix3 cofactor() const;
	matrix3 adjoint() const;
	matrix3 inverse() const;

	void star(vec3 &vector);
	void normalize();
	matrix3 transpose();

	float m[9];
private:
};

class matrix4
{
public:
	matrix4();
	matrix4(float m0, float m1, float m2, float m3,
		float m4, float m5, float m6, float m7,
		float m8, float m9, float m10, float m11,
		float m12, float m13, float m14, float m15);

	matrix4 &operator=(const matrix4 &q);
	matrix4 &operator=(const float *matrix);
	matrix4 operator+(const matrix4 &q);
	matrix4 operator-(const matrix4 &q);
	matrix4 operator*(const float scalar);
	matrix4 operator*(const float *matrix);
	matrix4 premultiply(const float *mat) const;
	vec4 operator*(const vec4 &vec) const;
	matrix4 operator*(const matrix4 &q) const;
	void normalize();
	matrix4 transpose();
	void perspective(float fovy, float aspect, float zNear, float zFar, bool infinite);
	void ortho(float left, float right, float bottom, float top, float near, float far);
	void lookat(const vec3 &position, vec3 &center, vec3 &up);


	// Generate matrices for point light shadow map/cubemap
	static void mat_forward(matrix4 &mvp, vec3 &position);
	static void mat_right(matrix4 &mvp, vec3 &position);
	static void mat_backward(matrix4 &mvp, vec3 &position);
	static void mat_left(matrix4 &mvp, vec3 &position);
	static void mat_top(matrix4 &mvp, vec3 &position);
	static void mat_bottom(matrix4 &mvp, vec3 &position);
	float det() const;
	matrix4 cofactor() const;
	matrix4 adjoint() const;
	matrix4 inverse() const;


	float m[16];
private:
};


// for scalar on left side must be outside class
matrix3 operator*(const float scalar, const matrix3 &mat);
matrix4 operator*(const float scalar, const matrix4 &mat);

#endif

