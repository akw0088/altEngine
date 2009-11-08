#include "include.h"

void rotateVector(float rad, vec3 &vec, vec3 &axis)
{
	matrix3	m;
	float	sinVal, cosVal, minusVal;

	sinVal = (float)fsin(rad);
	cosVal = (float)fcos(rad);
	minusVal = 1.0f - cosVal;

	m.m[0] = cosVal + minusVal * axis.x * axis.x;
	m.m[1] = minusVal * axis.x * axis.y - sinVal * axis.z;
	m.m[2] = minusVal * axis.x * axis.z + sinVal * axis.y;

	m.m[3] = minusVal * axis.y * axis.x + sinVal * axis.z;
	m.m[4] = cosVal + minusVal * axis.y * axis.y;
	m.m[5] = minusVal * axis.y * axis.z - sinVal * axis.x;

	m.m[6] = minusVal * axis.z * axis.x - sinVal * axis.y;
	m.m[7] = minusVal * axis.z * axis.y + sinVal * axis.x;
	m.m[8] = cosVal + minusVal * axis.z * axis.z;

	vec = m * vec;
}

void Frame::set(Graphics &gfx)
{
	float	matrix[16];
	vec3	right;

	right = vec3::crossproduct(this->up, this->forward);
	right.normalize();

	// opengl - 12,13,14 are position
	matrix[0] = right.x;
	matrix[1] = up.x;
	matrix[2] = forward.x;
	matrix[3] = 0.0f;

	matrix[4] = right.y;
	matrix[5] = up.y;
	matrix[6] = forward.y;
	matrix[7] = 0.0f;

	matrix[8] = right.z;
	matrix[9] = up.z;
	matrix[10] = forward.z;
	matrix[11] = 0.0f;

	matrix[12] = -(right * pos);
	matrix[13] = -(up * pos);
	matrix[14] = -(forward * pos);
	matrix[15] = 1.0f;
	gfx.LoadMatrix(matrix);
}

void frame2ent(Frame *camera, Entity &entity, Keyboard &keyboard)
{
	if (keyboard.control == false)
	{
		vec3		right;

		right = vec3::crossproduct(camera->up, camera->forward);
		right.normalize();

		entity.morientation.m[0] = camera->forward.x;
		entity.morientation.m[1] = camera->forward.y;
		entity.morientation.m[2] = camera->forward.z;

		entity.morientation.m[3] = camera->up.x;
		entity.morientation.m[4] = camera->up.y;
		entity.morientation.m[5] = camera->up.z;

		entity.morientation.m[6] = right.x;
		entity.morientation.m[7] = right.y;
		entity.morientation.m[8] = right.z;

		entity.position = camera->pos - camera->forward * 100.0f - camera->up * 20.0f;
		entity.angular_velocity = vec3();
		entity.velocity = vec3();
		entity.sleep = false;
	}

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
	rotateVector(delta.x / 50.0f, forward, vec3(0.0f, 1.0f, 0.0f));
	rotateVector(delta.x / 50.0f, up, vec3(0.0f, 1.0f, 0.0f));
	forward.normalize();
	up.normalize();

	// Up / Down
	right = vec3::crossproduct(up, forward);
	right.normalize();

	rotateVector(delta.y / 50.0f, forward, right);
	rotateVector(delta.y / 50.0f, up, right);
	forward.normalize();
	up.normalize();
}
