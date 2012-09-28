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
	matrix4 premultiply(const float *mat);
	vec4 operator*(const vec4 &vec);
	matrix4 operator*(const matrix4 &q);
	void normalize();
	matrix4 transpose();
	void perspective(float fovy, float aspect, float zNear, float zFar, bool infinite);
	void ortho(float left, float right, float bottom, float top, float near, float far);


	float m[16];
private:
};

#endif

