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

#ifndef VECTOR_H
#define VECTOR_H
class vec3;
class matrix3;

class vec4
{
public:
	vec4();
	vec4(float x, float y, float z, float w);
	vec4(const vec3 &v, float w);


	float magnitude();
	vec4 &normalize();

	vec4 operator+(const vec4 &vector) const;
	vec4 operator+(const float scalar) const;


	vec4 operator*(const float scalar) const;
	vec4 &operator*=(const vec4 &vec);
	vec4 &operator*=(const float scalar);
	vec4 &operator/=(const float scalar);
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
	float magnitude() const;
	float magnitudeSq();

	vec3 &operator=(const vec3 &vector);
	bool operator==(const vec3 &vector) const;
	vec3 operator+(const vec3 &vector) const;
	vec3 operator-(const vec3 &vector) const;
	vec3 operator-() const;
	float operator*(const vec3 &vector) const;
	vec3 operator*(const matrix3 &mat) const;
	float operator/(const vec3 &vector) const;
	vec3 operator*(const float scalar) const;
	vec3 operator/(const float scalar) const;
	vec3 &operator+=(const vec3 &vec);
	vec3 &operator-=(const vec3 &vec);
	vec3 &operator*=(const float scalar);
	vec3 project(const vec3 &a) const;
	vec3 reflect(const vec3& normal) const;
	float angle(const vec3& r) const;
	

	



	static float dotproduct(const vec3 &VecA, const vec3 &VecB);
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
	vec2 operator+(const vec2 &vector) const;
	vec2 operator-(const vec2 &vector) const;
	vec2 &operator*=(const float scalar);
	vec2 operator*(const float scalar) const;
	float operator*(const vec2 &vector) const;
	vec2 operator/(const float scalar);
	float operator/(const vec2 &vector) const;
	vec2 project(const vec2 &a) const;
	vec2 reflect(const vec2& normal) const;

	vec2 &normalize();
	float magnitude();
	float magnitudeSq();
	float angle(const vec2 &r) const;

	float x, y;
};



// for scalar on left side must be outside class
vec2 operator*(const float &scalar, vec2 const &vec);
vec3 operator*(const float &scalar, vec3 const &vec);
vec4 operator*(const float &scalar, vec4 const &vec);

#endif

