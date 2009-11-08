#include "include.h"

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

#ifndef DIRECTX
void Plane::draw_plane()
{
	float	fExtent = 500.0f;
	float	fStep = 20.0f;
	float	iLine;
	float	matrix[16];

	vec3 forward = vec3::crossproduct(normal, vec3(0.0, 1.0, 0.0));
	forward.normalize();
	float magnitude = forward.magnitude();

	if ( magnitude < 0.001f && magnitude > -0.001f)
	{
		forward = vec3::crossproduct(normal, vec3(0.0, 0.0, 1.0));
		forward.normalize();
	}

	vec3 right = vec3::crossproduct(normal, forward);
	right.normalize();

	matrix[0] = right.x;
	matrix[1] = right.y;
	matrix[2] = right.z;
	matrix[3] = 0.0f;

	matrix[4] = normal.x;
	matrix[5] = normal.y;
	matrix[6] = normal.z;
	matrix[7] = 0.0f;

	matrix[8] = forward.x;
	matrix[9] = forward.y;
	matrix[10] = forward.z;
	matrix[11] = 0.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;

	glPushMatrix();
	glMultMatrixf(matrix);

	glBegin(GL_LINES);
	for(iLine = -fExtent; iLine <= fExtent; iLine += fStep)
	{
		glVertex3f(iLine, -d, fExtent);    // Draw Z lines
		glVertex3f(iLine, -d, -fExtent);
		glVertex3f(fExtent, -d, iLine);
		glVertex3f(-fExtent, -d, iLine);
	}
	glEnd();
	glPopMatrix();
}
#endif
