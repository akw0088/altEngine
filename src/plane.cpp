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

// Should probably delete this class and just use a strut

#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Plane::Plane()
{
	normal.x = 0.0f;
	normal.y = 1.0f;
	normal.z = 0.0f;
	d = 0.0f;
}

Plane::Plane(vec4 vector)
{
	normal = vector;
	d = vector.w;
}

Plane::Plane(vec3 normal, float d)
{
	Plane::normal = normal;
	Plane::d = d;
}



/*
	Extract frustum planes from projection matrix
	Issue I've always had was rotating these planes to worldspace to test points
	But now that I think about it why dont I just convert world AABB's to clipspace
	and check [-1,1] and not even bother with these
*/
void Plane::get_frustum(matrix4 &projection, Plane *frustum)
{
	float A, B, C, D;

	//left = col1 + col4
	A = projection.m[0]  + projection.m[3];
	B = projection.m[4]  + projection.m[7];
	C = projection.m[8]  + projection.m[11];
	D = projection.m[12] + projection.m[15];

	frustum[0].normal.x = A;
	frustum[0].normal.y = B;
	frustum[0].normal.z = C;
	frustum[0].d = D;



	//right = -col1 + col4
	A = -projection.m[0] + projection.m[3];
	B = -projection.m[4] + projection.m[7];
	C = -projection.m[8] + projection.m[11];
	D = -projection.m[12] + projection.m[15];

	frustum[1].normal.x = A;
	frustum[1].normal.y = B;
	frustum[1].normal.z = C;
	frustum[1].d = D;



	//bottom = col2 + col4
	A = projection.m[1] + projection.m[3];
	B = projection.m[5] + projection.m[7];
	C = projection.m[9] + projection.m[11];
	D = projection.m[13] + projection.m[15];

	frustum[2].normal.x = A;
	frustum[2].normal.y = B;
	frustum[2].normal.z = C;
	frustum[2].d = D;


	//top = -col2 + col4
	A = -projection.m[1] + projection.m[3];
	B = -projection.m[5] + projection.m[7];
	C = -projection.m[9] + projection.m[11];
	D = -projection.m[13] + projection.m[15];

	frustum[3].normal.x = A;
	frustum[3].normal.y = B;
	frustum[3].normal.z = C;
	frustum[3].d = D;
		

	//near = col3 + col4
	A = projection.m[2] + projection.m[3];
	B = projection.m[6] + projection.m[7];
	C = projection.m[10] + projection.m[11];
	D = projection.m[14] + projection.m[15];

	frustum[4].normal.x = A;
	frustum[4].normal.y = B;
	frustum[4].normal.z = C;
	frustum[4].d = D;
		

	//far = -col3 + col4
	A = -projection.m[2] + projection.m[3];
	B = -projection.m[6] + projection.m[7];
	C = -projection.m[10] + projection.m[11];
	D = -projection.m[14] + projection.m[15];

	frustum[5].normal.x = A;
	frustum[5].normal.y = B;
	frustum[5].normal.z = C;
	frustum[5].d = D;
}

