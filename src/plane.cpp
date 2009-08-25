#include "include.h"

void Plane::draw_plane()
{
	GLfloat fExtent = 200.0f;
	GLfloat fStep = 10.0f;
	GLint iLine;
	float matrix[16];
	vec3 pos;

	pos.x = normal.x * d;
	pos.y = normal.y * d;
	pos.z = normal.z * d;

	vec3 forward = vec3::crossproduct(normal, vec3(1.0, 2.0, 3.0));
	forward.normalize();
	vec3 right = vec3::crossproduct(normal, forward);
	right.normalize();

	glPushMatrix();
	matrix[0] = right.x;
	matrix[1] = normal.x;
	matrix[2] = forward.x;
	matrix[3] = 0.0f;

	matrix[4] = right.y;
	matrix[5] = normal.y;
	matrix[6] = forward.y;
	matrix[7] = 0.0f;

	matrix[8] = right.z;
	matrix[9] = normal.z;
	matrix[10] = forward.z;
	matrix[11] = 0.0f;

	matrix[12] = right * pos;
	matrix[13] = normal * pos;
	matrix[14] = forward * pos;
	matrix[15] = 1.0f;
	glMultMatrixf(matrix);

	glBegin(GL_LINES);
	for(iLine = -fExtent; iLine <= fExtent; iLine += fStep)
	{
		glVertex3f(iLine, d, fExtent);    // Draw Z lines
		glVertex3f(iLine, d, -fExtent);
		glVertex3f(fExtent, d, iLine);
		glVertex3f(-fExtent, d, iLine);
	}
	glEnd();
	glPopMatrix();
}

