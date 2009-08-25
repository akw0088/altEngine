#include "include.h"

void Plane::draw_plane()
{
	GLfloat fExtent = 200.0f;
	GLfloat fStep = 10.0f;
	GLint iLine;
	float matrix[16];

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

	glPushMatrix();
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

	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = 0;
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

