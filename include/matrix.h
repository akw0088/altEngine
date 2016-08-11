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

	void star(vec3 &vector);
	void normalize();
	matrix3 transpose();

	float m[9];
private:
};

class matrix4
{
public:
	matrix4 &operator=(const matrix4 &q);
	matrix4 &operator=(const float *matrix);
	matrix4 operator+(const matrix4 &q);
	matrix4 operator-(const matrix4 &q);
	matrix4 operator*(const float scalar);
	matrix4 operator*(const float *matrix);
	matrix4 premultiply(const float *mat) const;
	vec4 operator*(const vec4 &vec);
	matrix4 operator*(const matrix4 &q);
	void normalize();
	matrix4 transpose();
	void perspective(float fovy, float aspect, float zNear, float zFar, bool infinite);
	void ortho(float left, float right, float bottom, float top, float near, float far);

	// Generate matrices for point light shadow map/cubemap
	static void mat_forward(matrix4 &mvp, vec3 &position);
	static void mat_right(matrix4 &mvp, vec3 &position);
	static void mat_backward(matrix4 &mvp, vec3 &position);
	static void mat_left(matrix4 &mvp, vec3 &position);
	static void mat_top(matrix4 &mvp, vec3 &position);
	static void mat_bottom(matrix4 &mvp, vec3 &position);
	static void mat_cube(float *cube, vec3 &position);


	float m[16];
private:
};

#endif

