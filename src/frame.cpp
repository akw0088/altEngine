#include "include.h"

void rotateVector(float rad, vec3 &vec, float x, float y, float z)
{
	vec3 old;
	float matrix[9], sinVal, cosVal, minusVal;

	sinVal = sin(rad);
	cosVal = cos(rad);
	minusVal = 1.0f - cosVal;

	old = vec;

	matrix[0] = cosVal + minusVal * x * x;
	matrix[1] = minusVal * x * y - sinVal * z;
	matrix[2] = minusVal * x * z + sinVal * y;

	matrix[3] = minusVal * y * x + sinVal * z;
	matrix[4] = cosVal + minusVal * y * y;
	matrix[5] = minusVal * y * z - sinVal * x;

	matrix[6] = minusVal * z * x - sinVal * y;
	matrix[7] = minusVal * z * y + sinVal * x;
	matrix[8] = cosVal + minusVal * z * z;

	vec.x = old.x * matrix[0] + old.y * matrix[1] + old.z * matrix[2];
	vec.y = old.x * matrix[3] + old.y * matrix[4] + old.z * matrix[5]; 
	vec.z = old.x * matrix[6] + old.y * matrix[7] + old.z * matrix[8];
}

void Frame::set()
{
	float	matrix[16];
	vec3	vRight;

	vRight = vec3::crossproduct(this->up, this->forward);
	vRight.normalize();

	matrix[0] = vRight.x;
	matrix[1] = up.x;
	matrix[2] = forward.x;
	matrix[3] = 0.0f;

	matrix[4] = vRight.y;
	matrix[5] = up.y;
	matrix[6] = forward.y;
	matrix[7] = 0.0f;

	matrix[8] = vRight.z;
	matrix[9] = up.z;
	matrix[10] = forward.z;
	matrix[11] = 0.0f;

	matrix[12] = -(vRight * pos);
	matrix[13] = -(up * pos);
	matrix[14] = -(forward * pos);
	matrix[15] = 1.0f;
	glLoadMatrixf(matrix);
}

void Frame::update(Keyboard &keyboard)
{
	vec3	right;

#define SPEED 8.0f
	right = vec3::crossproduct(up, forward);
	if (keyboard.up)
		pos -= forward * SPEED;
	if (keyboard.down)
		pos += forward * SPEED;
	if (keyboard.left)
		pos -= right * SPEED;
	if (keyboard.right)
		pos += right * SPEED;
	if (keyboard.enter)
		pos.y += SPEED;
	if (keyboard.shift)
		pos.y -= SPEED;
}

void Frame::update(const vec2 &delta)
{
	vec3		right;



//"camera"
	// Left / Right
	rotateVector(-delta.x / 50.0f, forward, 0.0f, 1.0f, 0.0f);
	rotateVector(-delta.x / 50.0f, up, 0.0f, 1.0f, 0.0f);
	forward.normalize();
	up.normalize();

	// Up / Down
	right = vec3::crossproduct(up, forward);
	right.normalize();

	rotateVector(-delta.y / 50.0f, forward, right.x, right.y, right.z);
	rotateVector(-delta.y / 50.0f, up, right.x, right.y, right.z);
	forward.normalize();
	up.normalize();
}

