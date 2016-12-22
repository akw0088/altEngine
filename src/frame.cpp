#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void rotate_vector(float rad, vec3 &vec, const vec3 &axis)
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

void Frame::set(matrix4 &trans)
{
	vec3	right;

	right = vec3::crossproduct(up, forward);
	right.normalize();

	// opengl - 12,13,14 are position
	trans.m[0] = right.x;
	trans.m[1] = up.x;
	trans.m[2] = forward.x;
	trans.m[3] = 0.0f;

	trans.m[4] = right.y;
	trans.m[5] = up.y;
	trans.m[6] = forward.y;
	trans.m[7] = 0.0f;

	trans.m[8] = right.z;
	trans.m[9] = up.z;
	trans.m[10] = forward.z;
	trans.m[11] = 0.0f;

	trans.m[12] = -(right * pos);
	trans.m[13] = -(up * pos);
	trans.m[14] = -(forward * pos);
	trans.m[15] = 1.0f;
}

void Frame::set(matrix3 &trans)
{
	vec3	right;

	right = vec3::crossproduct(up, forward);
	right.normalize();

	trans.m[0] = -forward.x;
	trans.m[1] = -forward.y;
	trans.m[2] = -forward.z;
	trans.m[3] = up.x;
	trans.m[4] = up.y;
	trans.m[5] = up.z;
	trans.m[6] = right.x;
	trans.m[7] = right.y;
	trans.m[8] = right.z;
}


// Note: this is only used when you hold control currently
void Frame::update(button_t &keyboard)
{
	vec3	right = vec3::crossproduct(up, forward);

#define SPEED (8.0f) // meters per second
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
	const vec3		vup(0.0f, 1.0f, 0.0f);
	vec3		right;

//"camera"
	// Left / Right
	rotate_vector(delta.x / 50.0f, forward, vup );
	rotate_vector(delta.x / 50.0f, up, vup);
	forward.normalize();
	up.normalize();

	// Up / Down
	right = vec3::crossproduct(up, forward);
	right.normalize();

	vec3 old_forward = forward;
	vec3 old_up = up;
	float deltad = 50.0f;

	rotate_vector(delta.y / deltad, forward, right);
	rotate_vector(delta.y / deltad, up, right);
	forward.normalize();
	up.normalize();

	if (up * vup < 0)
	{
		forward = old_forward;
		up = old_up;
	}

}

void Frame::reset()
{
	pos = vec3(0.0f, 0.0f, 0.0f);
	up = vec3(0.0f, 1.0f, 0.0f);
	forward = vec3(0.0f, 0.0f, 1.0f);
}