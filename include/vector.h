#ifndef VECTOR_H
#define VECTOR_H

class vec4
{
public:
	vec4();
	vec4(float x, float y, float z, float w);

	float magnitude();
	vec4 &normalize();

	vec4 &operator*=(const float scalar);
	vec4 &operator+=(const vec4 &vec);
	vec4 &operator+=(const float scalar);

	float x;
	float y;
	float z;
	float w;
};

class vec3
{
public:
	vec3();
	vec3(float x, float y, float z);
	vec3(vec4 vector);
	vec3 &normalize();
	float magnitude();

	vec3 &operator=(const vec3 &vector);
	bool operator==(const vec3 &vector);
	vec3 operator+(const vec3 &vector) const;
	vec3 operator-(const vec3 &vector) const;
	vec3 operator-() const;
	float operator*(const vec3 &vector);
	float operator/(const vec3 &vector);
	vec3 operator*(const float scalar) const;
	vec3 operator/(const float scalar);
	vec3 &operator+=(const vec3 &vec);
	vec3 &operator-=(const vec3 &vec);
	vec3 &operator*=(const float scalar);

	static vec3 crossproduct(const vec3 &VecA, const vec3 &VecB);

	float x;
	float y;
	float z;
};

class vec2
{
public:
	vec2();
	vec2(float x, float y);
	vec2 &operator=(const vec2 &vector);
	vec2 operator+(const vec2 &vector);
	vec2 operator-(const vec2 &vector);
	vec2 &operator*=(const float scalar);

	float x, y;
};

#endif

