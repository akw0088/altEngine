#ifndef MATRIX_H
#define MATRIX_H

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
	matrix4 operator+(const matrix4 &q);
	matrix4 operator-(const matrix4 &q);
	matrix4 operator*(const float scalar);
	vec4 operator*(const vec4 &vec);
	matrix4 operator*(const matrix4 &q);
	void normalize();
	matrix4 transpose();

	float m[16];
private:
};

#endif

