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

#include "include.h"

#include <math.h> // for sin/cos

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


matrix3::matrix3()
{

}

matrix3::matrix3(float m0, float m1, float m2,
	float m3, float m4, float m5,
	float m6, float m7, float m8)
{
	m[0] = m0;
	m[1] = m1;
	m[2] = m2;

	m[3] = m3;
	m[4] = m4;
	m[5] = m5;

	m[6] = m6;
	m[7] = m7;
	m[8] = m8;
}

void matrix3::matrix4to3(matrix4 &mat)
{
	m[0] = mat.m[0];
	m[1] = mat.m[1];
	m[2] = mat.m[2];

	m[3] = mat.m[4];
	m[4] = mat.m[5];
	m[5] = mat.m[6];

	m[6] = mat.m[8];
	m[7] = mat.m[9];
	m[8] = mat.m[10];

}

void matrix3::normalize()
{
	vec3 x,y,z;

	x.x = m[0];
	x.y = m[1];
	x.z = m[2];

	y.x = m[3];
	y.y = m[4];
	y.z = m[5];

	x.normalize();
	z = vec3::crossproduct(x, y).normalize();
	y = vec3::crossproduct(z, x).normalize();

	m[0] = x.x;
	m[1] = x.y;
	m[2] = x.z;

	m[3] = y.x;
	m[4] = y.y;
	m[5] = y.z;

	m[6] = z.x;
	m[7] = z.y;
	m[8] = z.z;
}

void matrix4::normalize()
{
	vec3 x,y,z;

	x.x = m[0];
	x.y = m[1];
	x.z = m[2];

	y.x = m[4];
	y.y = m[5];
	y.z = m[6];

	x.normalize();
	z = vec3::crossproduct(x, y).normalize();
	y = vec3::crossproduct(z, x).normalize();

	m[0] = x.x;
	m[1] = x.y;
	m[2] = x.z;

	m[4] = y.x;
	m[5] = y.y;
	m[6] = y.z;

	m[8] = z.x;
	m[9] = z.y;
	m[10] = z.z;
}

void matrix3::star(vec3 &vector)
{
	m[0] = 0.0f;
	m[1] = -vector.z;
	m[2] = vector.y;

	m[3] = vector.z;
	m[4] = 0.0f;
	m[5] = -vector.x;

	m[6] = -vector.y;
	m[7] = vector.x;
	m[8] = 0.0f;
}

matrix3 matrix3::transpose()
{
	matrix3 trans;

	trans.m[0] = m[0];
	trans.m[1] = m[3];
	trans.m[2] = m[6];

	trans.m[3] = m[1];
	trans.m[4] = m[4];
	trans.m[5] = m[7];

	trans.m[6] = m[2];
	trans.m[7] = m[5];
	trans.m[8] = m[8];

	return trans;
}

matrix3 &matrix3::operator=(const matrix3 &mat)
{
	m[0] = mat.m[0];
	m[1] = mat.m[1];
	m[2] = mat.m[2];

	m[3] = mat.m[3];
	m[4] = mat.m[4];
	m[5] = mat.m[5];
	
	m[6] = mat.m[6];
	m[7] = mat.m[7];
	m[8] = mat.m[8];
	return *this;
}

matrix3 matrix3::operator+(const matrix3 &mat)
{
	matrix3 result;

	result.m[0] = m[0] + mat.m[0];
	result.m[1] = m[1] + mat.m[1];
	result.m[2] = m[2] + mat.m[2];

	result.m[3] = m[3] + mat.m[3];
	result.m[4] = m[4] + mat.m[4];
	result.m[5] = m[5] + mat.m[5];
	
	result.m[6] = m[6] + mat.m[6];
	result.m[7] = m[7] + mat.m[7];
	result.m[8] = m[8] + mat.m[8];

	return result;
}

matrix3 matrix3::operator-(const matrix3 &mat)
{
	matrix3 result;

	result.m[0] = m[0] - mat.m[0];
	result.m[1] = m[1] - mat.m[1];
	result.m[2] = m[2] - mat.m[2];

	result.m[3] = m[3] - mat.m[3];
	result.m[4] = m[4] - mat.m[4];
	result.m[5] = m[5] - mat.m[5];
	
	result.m[6] = m[6] - mat.m[6];
	result.m[7] = m[7] - mat.m[7];
	result.m[8] = m[8] - mat.m[8];
	return result;
}

matrix3 matrix3::operator*(const float scalar)
{
	matrix3 result;

	result.m[0] = m[0] * scalar;
	result.m[1] = m[1] * scalar;
	result.m[2] = m[2] * scalar;

	result.m[3] = m[3] * scalar;
	result.m[4] = m[4] * scalar;
	result.m[5] = m[5] * scalar;
	
	result.m[6] = m[6] * scalar;
	result.m[7] = m[7] * scalar;
	result.m[8] = m[8] * scalar;
	return result;
}


vec3 matrix3::operator*(const vec3 &vec) const
{
	vec3 result;

	result.x = vec.x * m[0] + vec.y * m[1] + vec.z * m[2];
	result.y = vec.x * m[3] + vec.y * m[4] + vec.z * m[5];
	result.z = vec.x * m[6] + vec.y * m[7] + vec.z * m[8];
	return result;
}

matrix3 matrix3::operator*(const matrix3 &mat)
{
	matrix3 result;

	result.m[0] = m[0] * mat.m[0] + m[1] * mat.m[3] + m[2] * mat.m[6];
	result.m[1] = m[0] * mat.m[1] + m[1] * mat.m[4] + m[2] * mat.m[7];
	result.m[2] = m[0] * mat.m[2] + m[1] * mat.m[5] + m[2] * mat.m[8];

	result.m[3] = m[3] * mat.m[0] + m[4] * mat.m[3] + m[5] * mat.m[6];
	result.m[4] = m[3] * mat.m[1] + m[4] * mat.m[4] + m[5] * mat.m[7];
	result.m[5] = m[3] * mat.m[2] + m[4] * mat.m[5] + m[5] * mat.m[8];

	result.m[6] = m[6] * mat.m[0] + m[7] * mat.m[3] + m[8] * mat.m[6];
	result.m[7] = m[6] * mat.m[1] + m[7] * mat.m[4] + m[8] * mat.m[7];
	result.m[8] = m[6] * mat.m[2] + m[7] * mat.m[5] + m[8] * mat.m[8];
	return result;
}

float matrix3::det() const
{
	return	 m[0] * ((m[4] * m[8]) - (m[5]*m[7])) +
		-m[1] * ((m[3] * m[8]) - (m[5]*m[6])) +
		 m[2] * ((m[3] * m[7]) - (m[4]*m[6]));
}

matrix3 matrix3::cofactor() const
{
	matrix3 result;
	
	result.m[0] =  (m[4] * m[8]) - (m[5] * m[7]);
	result.m[1] = -((m[3] * m[8]) - (m[5] * m[6]));
	result.m[2] =  (m[3] * m[7]) - (m[4] * m[6]);
	
	result.m[3] = -((m[1] * m[8]) - (m[2] * m[7]));
	result.m[4] =  (m[0] * m[8]) - (m[2] * m[6]);
	result.m[5] = -((m[0] * m[7]) - (m[1] * m[6]));
	
	result.m[6] =  (m[1] * m[5]) - (m[2] * m[4]);
	result.m[7] = -((m[0] * m[5]) - (m[2] * m[3]));
	result.m[8] =  (m[0] * m[4]) - (m[1] * m[3]);
	
	return result;
}


matrix3 matrix3::adjoint() const
{
	matrix3 result;
	
	result = cofactor();
	return result.transpose();
}


matrix3 matrix3::inverse() const
{
	return adjoint() * (1.0f /  det());
}


matrix4 matrix4::cofactor() const
{
	matrix4 result;


	result.m[0] =	 m[5] * (m[10] * m[15] - m[11] * m[14]) +
			-m[6] * (m[ 9] * m[15] - m[11] * m[13]) +
			 m[7] * (m[ 9] * m[14] - m[10] * m[13]);
		
	result.m[1] =  	 -(m[4] * (m[10] * m[15] - m[11] * m[14]) +
			-m[6] * (m[ 8] * m[15] - m[11] * m[12]) +
			 m[7] * (m[ 8] * m[14] - m[10] * m[12]));

	result.m[2] =	 m[4] * (m[ 9] * m[15] - m[11] * m[13]) +
			-m[5] * (m[ 8] * m[15] - m[11] * m[12]) +
			 m[7] * (m[ 8] * m[13] - m[ 9] * m[12]);
		 
	result.m[3] =	 -(m[4] * (m[ 9] * m[14] - m[10] * m[13]) +
			-m[5] * (m[ 8] * m[14] - m[10] * m[12]) +
			 m[6] * (m[ 8] * m[13] - m[ 9] * m[12]));
		 
	result.m[4] =  	 -(m[1] * (m[10] * m[15] - m[11] * m[14]) +
			-m[2] * (m[ 9] * m[15] - m[11] * m[13]) +
			 m[3] * (m[ 9] * m[14] - m[10] * m[13]));
		 
	result.m[5] =  	 m[0] * (m[10] * m[15] - m[11] * m[14]) +
			-m[2] * (m[ 8] * m[15] - m[11] * m[12]) +
			 m[3] * (m[ 8] * m[14] - m[10] * m[12]);
		 
	result.m[6] =  	 -(m[0] * (m[ 9] * m[15] - m[11] * m[13]) +
			-m[1] * (m[ 8] * m[15] - m[11] * m[12]) +
			 m[3] * (m[ 8] * m[13] - m[ 9] * m[12]));
		 
	result.m[7] =	 m[0] * (m[ 9] * m[14] - m[10] * m[13]) +
			-m[1] * (m[ 8] * m[14] - m[10] * m[12]) +
			 m[2] * (m[ 8] * m[13] - m[ 9] * m[12]);
		 
	result.m[8] = 	 m[1] * (m[ 6] * m[15] - m[ 7] * m[14]) +
			-m[2] * (m[ 5] * m[15] - m[ 7] * m[13]) +
			 m[3] * (m[ 5] * m[14] - m[ 6] * m[13]);
		 
	result.m[9] =  	 -(m[0] * (m[ 6] * m[15] - m[ 7] * m[14]) +
			-m[2] * (m[ 4] * m[15] - m[ 7] * m[12]) +
			 m[3] * (m[ 4] * m[14] - m[ 6] * m[12]));
		 
	result.m[10] = 	 m[0] * (m[ 5] * m[15] - m[ 7] * m[13]) +
			-m[1] * (m[ 4] * m[15] - m[ 7] * m[12]) +
			 m[3] * (m[ 4] * m[13] - m[ 5] * m[12]);
		 
	result.m[11] = 	 -(m[0] * (m[ 5] * m[14] - m[ 6] * m[13]) +
			-m[1] * (m[ 4] * m[14] - m[ 6] * m[12]) +
			 m[2] * (m[ 4] * m[13] - m[ 5] * m[12]));
		 
	result.m[12] = 	 -(m[1] * (m[ 6] * m[11] - m[ 7] * m[10]) +
			-m[2] * (m[ 5] * m[11] - m[ 7] * m[ 9]) +
			 m[3] * (m[ 5] * m[10] - m[ 6] * m[ 9]));
		 
	result.m[13] = 	 m[0] * (m[ 6] * m[11] - m[ 7] * m[10]) +
			-m[2] * (m[ 4] * m[11] - m[ 7] * m[ 8]) +
			 m[3] * (m[ 4] * m[10] - m[ 6] * m[ 8]);
		 
	result.m[14] = 	 -(m[0] * (m[ 5] * m[11] - m[ 7] * m[ 9]) +
			-m[1] * (m[ 4] * m[11] - m[ 7] * m[ 8]) +
			 m[3] * (m[ 4] * m[ 9] - m[ 5] * m[ 8]));
		 
	result.m[15] = 	 m[0] * (m[ 5] * m[10] - m[ 6] * m[ 9]) +
			-m[1] * (m[ 4] * m[10] - m[ 6] * m[ 8]) +
			 m[2] * (m[ 4] * m[ 9] - m[ 5] * m[ 8]);

	return result;
}

matrix4 matrix4::adjoint() const
{
	matrix4 result;

	result = cofactor();
	return result.transpose();
}


matrix4 matrix4::inverse() const
{
	return adjoint() * (1.0f / det());
}

matrix4::matrix4()
{

}

matrix4::matrix4(float m0, float m1, float m2, float m3,
	float m4, float m5, float m6, float m7,
	float m8, float m9, float m10, float m11,
	float m12, float m13, float m14, float m15)
{
	m[0] = m0;
	m[1] = m1;
	m[2] = m2;
	m[3] = m3;

	m[4] = m4;
	m[5] = m5;
	m[6] = m6;
	m[7] = m7;

	m[8] = m8;
	m[9] = m9;
	m[10] = m10;
	m[11] = m11;

	m[12] = m12;
	m[13] = m13;
	m[14] = m14;
	m[15] = m15;
}

matrix4 &matrix4::operator=(const matrix4 &mat)
{
	m[0] = mat.m[0];
	m[1] = mat.m[1];
	m[2] = mat.m[2];
	m[3] = mat.m[3];

	m[4] = mat.m[4];
	m[5] = mat.m[5];
	m[6] = mat.m[6];
	m[7] = mat.m[7];

	m[8] = mat.m[8];
	m[9] = mat.m[9];
	m[10] = mat.m[10];
	m[11] = mat.m[11];
	
	m[12] = mat.m[12];
	m[13] = mat.m[13];
	m[14] = mat.m[14];
	m[15] = mat.m[15];
	return *this;
}

matrix4 &matrix4::operator=(const float *mat)
{
	m[0]  = mat[0];
	m[1]  = mat[1];
	m[2]  = mat[2];
	m[3]  = mat[3];

	m[4]  = mat[4];
	m[5]  = mat[5];
	m[6]  = mat[6];
	m[7]  = mat[7];

	m[8]  = mat[8];
	m[9]  = mat[9];
	m[10] = mat[10];
	m[11] = mat[11];
	
	m[12] = mat[12];
	m[13] = mat[13];
	m[14] = mat[14];
	m[15] = mat[15];
	return *this;
}

matrix4 matrix4::operator+(const matrix4 &mat)
{
	matrix4 result;

	result.m[0] = m[0] + mat.m[0];
	result.m[1] = m[1] + mat.m[1];
	result.m[2] = m[2] + mat.m[2];
	result.m[3] = m[3] + mat.m[3];

	result.m[4] = m[4] + mat.m[4];
	result.m[5] = m[5] + mat.m[5];
	result.m[6] = m[6] + mat.m[6];
	result.m[7] = m[7] + mat.m[7];

	result.m[8] = m[8] + mat.m[8];
	result.m[9] = m[9] + mat.m[9];
	result.m[10] = m[10] + mat.m[10];
	result.m[11] = m[11] + mat.m[11];
	
	result.m[12] = m[12] + mat.m[12];
	result.m[13] = m[13] + mat.m[13];
	result.m[14] = m[14] + mat.m[14];
	result.m[15] = m[15] + mat.m[15];

	return result;
}

matrix4 matrix4::operator-(const matrix4 &mat)
{
	matrix4 result;

	result.m[0] = m[0] - mat.m[0];
	result.m[1] = m[1] - mat.m[1];
	result.m[2] = m[2] - mat.m[2];
	result.m[3] = m[3] - mat.m[3];

	result.m[4] = m[4] - mat.m[4];
	result.m[5] = m[5] - mat.m[5];
	result.m[6] = m[6] - mat.m[6];
	result.m[7] = m[7] - mat.m[7];

	result.m[8] = m[8] - mat.m[8];
	result.m[9] = m[9] - mat.m[9];
	result.m[10] = m[10] - mat.m[10];
	result.m[11] = m[11] - mat.m[11];
	
	result.m[12] = m[12] - mat.m[12];
	result.m[13] = m[13] - mat.m[13];
	result.m[14] = m[14] - mat.m[14];
	result.m[15] = m[15] - mat.m[15];

	return result;
}

matrix4 matrix4::operator*(const float scalar)
{
	matrix4 result;

	result.m[0] = m[0] * scalar;
	result.m[1] = m[1] * scalar;
	result.m[2] = m[2] * scalar;
	result.m[3] = m[3] * scalar;

	result.m[4] = m[4] * scalar;
	result.m[5] = m[5] * scalar;
	result.m[6] = m[6] * scalar;
	result.m[7] = m[7] * scalar;

	result.m[8] = m[8] * scalar;
	result.m[9] = m[9] * scalar;
	result.m[10] = m[10] * scalar;
	result.m[11] = m[11] * scalar;

	result.m[12] = m[12] * scalar;
	result.m[13] = m[13] * scalar;
	result.m[14] = m[14] * scalar;
	result.m[15] = m[15] * scalar;
	return result;
}

vec4 matrix4::operator*(const vec4 &vec) const
{
	vec4 result;

	result.x = vec.x * m[0] + vec.y * m[4] + vec.z * m[8] + vec.w * m[12];
	result.y = vec.x * m[1] + vec.y * m[5] + vec.z * m[9] + vec.w * m[13];
	result.z = vec.x * m[2] + vec.y * m[6] + vec.z * m[10] + vec.w * m[14];
	result.w = vec.x * m[3] + vec.y * m[7] + vec.z * m[11] + vec.w * m[15];

	return result;
}

matrix4 matrix4::operator*(const matrix4 &mat) const
{
	matrix4 result;

	result.m[0]  = m[0] *  mat.m[0] + m[1] *  mat.m[4] + m[2] *  mat.m[8] +  m[3] *  mat.m[12];
	result.m[1]  = m[0] *  mat.m[1] + m[1] *  mat.m[5] + m[2] *  mat.m[9] +  m[3] *  mat.m[13];
	result.m[2]  = m[0] *  mat.m[2] + m[1] *  mat.m[6] + m[2] *  mat.m[10] +  m[3] *  mat.m[14];
	result.m[3]  = m[0] *  mat.m[3] + m[1] *  mat.m[7] + m[2] *  mat.m[11] +  m[3] *  mat.m[15];

	result.m[4]  = m[4] *  mat.m[0] + m[5] *  mat.m[4] + m[6] *  mat.m[8] +  m[7] *  mat.m[12];
	result.m[5]  = m[4] *  mat.m[1] + m[5] *  mat.m[5] + m[6] *  mat.m[9] +  m[7] *  mat.m[13];
	result.m[6]  = m[4] *  mat.m[2] + m[5] *  mat.m[6] + m[6] *  mat.m[10] +  m[7] *  mat.m[14];
	result.m[7]  = m[4] *  mat.m[3] + m[5] *  mat.m[7] + m[6] *  mat.m[11] +  m[7] *  mat.m[15];

	result.m[8]  = m[8] *  mat.m[0] + m[9] *  mat.m[4] + m[10] * mat.m[8] + m[11] * mat.m[12];
	result.m[9]  = m[8] *  mat.m[1] + m[9] *  mat.m[5] + m[10] * mat.m[9] + m[11] * mat.m[13];
	result.m[10] = m[8] *  mat.m[2] + m[9] *  mat.m[6] + m[10] * mat.m[10] + m[11] * mat.m[14];
	result.m[11] = m[8] *  mat.m[3] + m[9] *  mat.m[7] + m[10] * mat.m[11] + m[11] * mat.m[15];

	result.m[12] = m[12] * mat.m[0] + m[13] * mat.m[4] + m[14] * mat.m[8] + m[15] * mat.m[12];
	result.m[13] = m[12] * mat.m[1] + m[13] * mat.m[5] + m[14] * mat.m[9] + m[15] * mat.m[13];
	result.m[14] = m[12] * mat.m[2] + m[13] * mat.m[6] + m[14] * mat.m[10] + m[15] * mat.m[14];
	result.m[15] = m[12] * mat.m[3] + m[13] * mat.m[7] + m[14] * mat.m[11] + m[15] * mat.m[15];

	return result;
}

matrix4 matrix4::operator*(const float *mat)
{
	matrix4 result;

	result.m[0]  = m[0] *  mat[0] + m[1] *  mat[4] + m[2]  * mat[8] +  m[3] *  mat[12];
	result.m[1]  = m[0] *  mat[1] + m[1] *  mat[5] + m[2]  * mat[9] +  m[3] *  mat[13];
	result.m[2]  = m[0] *  mat[2] + m[1] *  mat[6] + m[2]  * mat[10] + m[3] *  mat[14];
	result.m[3]  = m[0] *  mat[3] + m[1] *  mat[7] + m[2]  * mat[11] + m[3] *  mat[15];

	result.m[4]  = m[4] *  mat[0] + m[5] *  mat[4] + m[6]  * mat[8] +  m[7] *  mat[12];
	result.m[5]  = m[4] *  mat[1] + m[5] *  mat[5] + m[6]  * mat[9] +  m[7] *  mat[13];
	result.m[6]  = m[4] *  mat[2] + m[5] *  mat[6] + m[6]  * mat[10] + m[7] *  mat[14];
	result.m[7]  = m[4] *  mat[3] + m[5] *  mat[7] + m[6]  * mat[11] + m[7] *  mat[15];

	result.m[8]  = m[8] *  mat[0] + m[9] *  mat[4] + m[10] * mat[8] +  m[11] * mat[12];
	result.m[9]  = m[8] *  mat[1] + m[9] *  mat[5] + m[10] * mat[9] +  m[11] * mat[13];
	result.m[10] = m[8] *  mat[2] + m[9] *  mat[6] + m[10] * mat[10] + m[11] * mat[14];
	result.m[11] = m[8] *  mat[3] + m[9] *  mat[7] + m[10] * mat[11] + m[11] * mat[15];

	result.m[12] = m[12] * mat[0] + m[13] * mat[4] + m[14] * mat[8] +  m[15] * mat[12];
	result.m[13] = m[12] * mat[1] + m[13] * mat[5] + m[14] * mat[9] +  m[15] * mat[13];
	result.m[14] = m[12] * mat[2] + m[13] * mat[6] + m[14] * mat[10] + m[15] * mat[14];
	result.m[15] = m[12] * mat[3] + m[13] * mat[7] + m[14] * mat[11] + m[15] * mat[15];

	return result;
}


matrix4 matrix4::premultiply(const float *mat) const
{
	matrix4 result;

	result.m[0]  = mat[0] *  m[0] + mat[1] *  m[4] + mat[2]  * m[8] +  mat[3] *  m[12];
	result.m[1]  = mat[0] *  m[1] + mat[1] *  m[5] + mat[2]  * m[9] +  mat[3] *  m[13];
	result.m[2]  = mat[0] *  m[2] + mat[1] *  m[6] + mat[2]  * m[10] + mat[3] *  m[14];
	result.m[3]  = mat[0] *  m[3] + mat[1] *  m[7] + mat[2]  * m[11] + mat[3] *  m[15];

	result.m[4]  = mat[4] *  m[0] + mat[5] *  m[4] + mat[6]  * m[8] +  mat[7] *  m[12];
	result.m[5]  = mat[4] *  m[1] + mat[5] *  m[5] + mat[6]  * m[9] +  mat[7] *  m[13];
	result.m[6]  = mat[4] *  m[2] + mat[5] *  m[6] + mat[6]  * m[10] + mat[7] *  m[14];
	result.m[7]  = mat[4] *  m[3] + mat[5] *  m[7] + mat[6]  * m[11] + mat[7] *  m[15];

	result.m[8]  = mat[8] *  m[0] + mat[9] *  m[4] + mat[10] * m[8] +  mat[11] * m[12];
	result.m[9]  = mat[8] *  m[1] + mat[9] *  m[5] + mat[10] * m[9] +  mat[11] * m[13];
	result.m[10] = mat[8] *  m[2] + mat[9] *  m[6] + mat[10] * m[10] + mat[11] * m[14];
	result.m[11] = mat[8] *  m[3] + mat[9] *  m[7] + mat[10] * m[11] + mat[11] * m[15];

	result.m[12] = mat[12] * m[0] + mat[13] * m[4] + mat[14] * m[8] +  mat[15] * m[12];
	result.m[13] = mat[12] * m[1] + mat[13] * m[5] + mat[14] * m[9] +  mat[15] * m[13];
	result.m[14] = mat[12] * m[2] + mat[13] * m[6] + mat[14] * m[10] + mat[15] * m[14];
	result.m[15] = mat[12] * m[3] + mat[13] * m[7] + mat[14] * m[11] + mat[15] * m[15];

	return result;
}

matrix4 matrix4::transpose()
{
	matrix4 trans;

	trans.m[0] = m[0];
	trans.m[1] = m[4];
	trans.m[2] = m[8];
	trans.m[3] = m[12];

	trans.m[4] = m[1];
	trans.m[5] = m[5];
	trans.m[6] = m[9];
	trans.m[7] = m[13];

	trans.m[8] = m[2];
	trans.m[9] = m[6];
	trans.m[10] = m[10];
	trans.m[11] = m[14];

	trans.m[12] = m[3];
	trans.m[13] = m[7];
	trans.m[14] = m[11];
	trans.m[15] = m[15];

	return trans;
}

void matrix4::perspective(float fovy, float aspect, float zNear, float zFar, bool infinite)
{
    double radians = fovy / 2 * (3.14159265358979323846) / 180;
    double cotangent = cos(radians) / sin(radians);
	float deltaZ = zFar - zNear;
	float epsilon = 0.001f;

	m[0] = (float)(cotangent / aspect);
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = (float)cotangent;
	m[6] = 0.0f;
	m[7] = 0.0f;
	
	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = -(zFar + zNear) / deltaZ;
	m[11] = -1.0f;
	
	m[12] = 0.0f;
	m[13] = 0.0f;
	m[14] = -2 * zNear * zFar / deltaZ;
	m[15] = 0.0f;

	if (infinite)
	{
		m[10] =  epsilon - 1.0f;
		m[14] =  zNear * (epsilon - 2.0f);
	}
}

void matrix4::ortho(float left, float right, float bottom, float top, float nearval, float farval)
{
	m[0] = 2.0f / (right - left);
	m[1] = 0.0f;
	m[2] = 0.0f;
	m[3] = 0.0f;

	m[4] = 0.0f;
	m[5] = 2.0f / (top - bottom);
	m[6] = 0.0f;
	m[7] = 0.0f;

	m[8] = 0.0f;
	m[9] = 0.0f;
	m[10] = -2.0f / (farval - nearval);
	m[11] = 0.0f;

	m[12] = -(right + left) / (right - left);
	m[13] = -(top + bottom) / (top - bottom);
	m[14] = -(farval + nearval) / (farval - nearval);
	m[15] = 1.0f;
}


void matrix4::mat_backward(matrix4 &mvp, vec3 &position)
{
	vec3 right(-1.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 forward(0.0f, 0.0f, -1.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = up.x;
	mvp.m[2] = forward.x;
	mvp.m[3] = 0.0f;

	mvp.m[4] = right.y;
	mvp.m[5] = up.y;
	mvp.m[6] = forward.y;
	mvp.m[7] = 0.0f;

	mvp.m[8] = right.z;
	mvp.m[9] = up.z;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = -right * position;
	mvp.m[13] = -up * position;
	mvp.m[14] = -forward * position;
	mvp.m[15] = 1.0f;
}

void matrix4::mat_left(matrix4 &mvp, vec3 &position)
{
	vec3 right(0.0f, 0.0f, -1.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 forward(1.0f, 0.0f, 0.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = up.x;
	mvp.m[2] = forward.x;
	mvp.m[3] = 0.0f;

	mvp.m[4] = right.y;
	mvp.m[5] = up.y;
	mvp.m[6] = forward.y;
	mvp.m[7] = 0.0f;

	mvp.m[8] = right.z;
	mvp.m[9] = up.z;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = -right * position;
	mvp.m[13] = -up * position;
	mvp.m[14] = -forward * position;
	mvp.m[15] = 1.0f;
}

void matrix4::mat_forward(matrix4 &mvp, vec3 &position)
{
	vec3 right(1.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 forward(0.0f, 0.0f, 1.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = up.x;
	mvp.m[2] = forward.x;
	mvp.m[3] = 0.0f;

	mvp.m[4] = right.y;
	mvp.m[5] = up.y;
	mvp.m[6] = forward.y;
	mvp.m[7] = 0.0f;

	mvp.m[8] = right.z;
	mvp.m[9] = up.z;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = -right * position;
	mvp.m[13] = -up * position;
	mvp.m[14] = -forward * position;
	mvp.m[15] = 1.0f;
}

void matrix4::mat_right(matrix4 &mvp, vec3 &position)
{
	vec3 right(0.0f, 0.0f, 1.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 forward(-1.0f, 0.0f, 0.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = up.x;
	mvp.m[2] = forward.x;
	mvp.m[3] = 0.0f;

	mvp.m[4] = right.y;
	mvp.m[5] = up.y;
	mvp.m[6] = forward.y;
	mvp.m[7] = 0.0f;

	mvp.m[8] = right.z;
	mvp.m[9] = up.z;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = -right * position;
	mvp.m[13] = -up * position;
	mvp.m[14] = -forward * position;
	mvp.m[15] = 1.0f;
}

void matrix4::mat_top(matrix4 &mvp, vec3 &position)
{
	vec3 right(1.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 0.0f, 1.0f);
	vec3 forward(0.0f, -1.0f, 0.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = up.x;
	mvp.m[2] = forward.x;
	mvp.m[3] = 0.0f;

	mvp.m[4] = right.y;
	mvp.m[5] = up.y;
	mvp.m[6] = forward.y;
	mvp.m[7] = 0.0f;

	mvp.m[8] = right.z;
	mvp.m[9] = up.z;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = -right * position;
	mvp.m[13] = -up * position;
	mvp.m[14] = -forward * position;
	mvp.m[15] = 1.0f;
}

void matrix4::mat_bottom(matrix4 &mvp, vec3 &position)
{
	vec3 right(1.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 0.0f, -1.0f);
	vec3 forward(0.0f, 1.0f, 0.0f);


	mvp.m[0] = right.x;
	mvp.m[1] = up.x;
	mvp.m[2] = forward.x;
	mvp.m[3] = 0.0f;

	mvp.m[4] = right.y;
	mvp.m[5] = up.y;
	mvp.m[6] = forward.y;
	mvp.m[7] = 0.0f;

	mvp.m[8] = right.z;
	mvp.m[9] = up.z;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = -right * position;
	mvp.m[13] = -up * position;
	mvp.m[14] = -forward * position;
	mvp.m[15] = 1.0f;
}

float matrix4::det() const
{
	return		 m[0] * 
				(
					 m[5] * ((m[10] * m[15]) - (m[11]*m[14])) +
					-m[6] * ((m[ 9] * m[15]) - (m[11]*m[13])) +
					 m[7] * ((m[ 9] * m[14]) - (m[10]*m[13]))
				) +
			-m[1] * (
					 m[4] * ((m[10] * m[15]) - (m[11]*m[14])) +
					-m[6] * ((m[ 8] * m[15]) - (m[11]*m[12])) +
					 m[7] * ((m[ 8] * m[14]) - (m[10]*m[12]))		
				) +
			 m[2] * (
					 m[4] * ((m[ 9] * m[15]) - (m[11]*m[13])) +
					-m[5] * ((m[ 8] * m[15]) - (m[11]*m[12])) +
					 m[7] * ((m[ 8] * m[13]) - (m[ 9]*m[12]))
				) +
			-m[3] * (
					 m[4] * ((m[ 9] * m[14]) - (m[10]*m[13])) +
					-m[5] * ((m[ 8] * m[14]) - (m[10]*m[12])) +
					 m[6] * ((m[ 8] * m[13]) - (m[ 9]*m[12]))
				);
}


void matrix4::lookat(const vec3 &position, vec3 &center, vec3 &up)
{
	vec3 forward = center - position;
	forward = forward.normalize();
	vec3 right = vec3::crossproduct(up, forward);
	vec3 newup = vec3::crossproduct(forward, right);

	m[0] = right.x;
	m[1] = newup.x;
	m[2] = forward.x;
	m[3] = 0.0f;

	m[4] = right.y;
	m[5] = newup.y;
	m[6] = forward.y;
	m[7] = 0.0f;

	m[8] = right.z;
	m[9] = newup.z;
	m[10] = forward.z;
	m[11] = 0.0f;

	m[12] = -(right * position);
	m[12] = -(newup * position);
	m[12] = -(forward * position);
}

// for scalar on left side, must be outside class
matrix3 operator*(const float scalar, const matrix3 &mat)
{
	matrix3 result;

	result.m[0] = scalar * mat.m[0];
	result.m[1] = scalar * mat.m[3];
	result.m[2] = scalar * mat.m[6];

	result.m[3] = scalar * mat.m[1];
	result.m[4] = scalar * mat.m[4];
	result.m[5] = scalar * mat.m[7];

	result.m[6] = scalar * mat.m[2];
	result.m[6] = scalar * mat.m[5];
	result.m[7] = scalar * mat.m[8];

	return result;
}

matrix4 operator*(const float scalar, const matrix4 &mat)
{
	matrix4 result;

	result.m[0] = scalar * mat.m[0];
	result.m[1] = scalar * mat.m[4];
	result.m[2] = scalar * mat.m[8];
	result.m[4] = scalar * mat.m[12];

	result.m[4] = scalar * mat.m[1];
	result.m[5] = scalar * mat.m[5];
	result.m[6] = scalar * mat.m[9];
	result.m[7] = scalar * mat.m[13];

	result.m[8]  = scalar * mat.m[2];
	result.m[9]  = scalar * mat.m[6];
	result.m[10] = scalar * mat.m[10];
	result.m[11] = scalar * mat.m[14];

	result.m[10] = scalar * mat.m[3];
	result.m[13] = scalar * mat.m[7];
	result.m[14] = scalar * mat.m[11];
	result.m[15] = scalar * mat.m[15];

	return result;
}

vec3 operator*(const vec3 v, const matrix3 &mat)
{
	vec4 result;

	result.x = v.x * mat.m[0] + v.x * mat.m[1] + v.x * mat.m[2];
	result.y = v.y * mat.m[3] + v.y * mat.m[4] + v.y * mat.m[5];
	result.z = v.z * mat.m[6] + v.z * mat.m[7] + v.z * mat.m[8];

	return result;
}


vec4 operator*(const vec4 v, const matrix4 &mat)
{
	vec4 result;

	result.x = v.x * mat.m[0] + v.x * mat.m[1] + v.x * mat.m[2] + v.x * mat.m[3];
	result.y = v.y * mat.m[4] + v.y * mat.m[5] + v.y * mat.m[6] + v.y * mat.m[7];
	result.z = v.z * mat.m[8] + v.z * mat.m[9] + v.z * mat.m[10] + v.z * mat.m[11];
	result.w = v.w * mat.m[12] + v.w * mat.m[13] + v.w * mat.m[14] + v.w * mat.m[15];

	return result;
}