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

#include "include.h"

#ifndef RAYTRACE_H

#define invsqrtf(x) (1.0f / sqrtf(x))
typedef vec3 color_t;


namespace raytrace
{

	typedef struct
	{
		vec3 start;
		vec3 dir;
	} ray_t;


	typedef struct
	{
		color_t diffuse;
		float reflection;
		color_t specular;
		float power;
	} material_t;

	typedef struct
	{
		vec3 pos;
		float size;
		int materialId;
	} sphere_t;

	typedef struct
	{
		vec3 n;
		vec3 p;
		int materialId;
	} rplane_t;


	typedef struct
	{
		vec3 a, b, c;
	} triangle_t;

	typedef struct
	{
		vec3 pos;
		color_t intensity;
	} light_t;
};
#endif
