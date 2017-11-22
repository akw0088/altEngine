#ifndef MATRIX_H
#define MATRIX_H

double fsin(double rad);
double fcos(double rad);

class matrix3
{
public:
	matrix3 &operator=(const matrix3 &q);
	matrix3 operator+(const matrix3 &q);
	matrix3 operator-(const matrix3 &q);
	matrix3 operator*(const float scalar);
	vec3 operator*(const vec3 &vec);
	matrix3 operator*(const matrix3 &q);
	float det();
	matrix3 cofactor();
	matrix3 adjoint();
	matrix3 inverse();

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
	vec4 operator*(const vec4 &vec);
	matrix4 operator*(const matrix4 &q) const;
	void normalize();
	matrix4 transpose();
	void perspective(float fovy, float aspect, float zNear, float zFar, bool infinite);
	void ortho(float left, float right, float bottom, float top, float near, float far);
	matrix4 lookat(const vec3 &eye, vec3 &center, vec3 &up);


	// Generate matrices for point light shadow map/cubemap
	static void mat_forward(matrix4 &mvp, vec3 &position);
	static void mat_right(matrix4 &mvp, vec3 &position);
	static void mat_backward(matrix4 &mvp, vec3 &position);
	static void mat_left(matrix4 &mvp, vec3 &position);
	static void mat_top(matrix4 &mvp, vec3 &position);
	static void mat_bottom(matrix4 &mvp, vec3 &position);
	float det();


	float m[16];
private:
};

#endif

