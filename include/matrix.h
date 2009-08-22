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

	void normalize();

	float m[9];
private:
};

#endif

