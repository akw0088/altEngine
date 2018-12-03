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

#include "vehicle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	DRAG		5.0f		 	// factor for air resistance (drag)
#define	RESISTANCE	30.0f			// factor for rolling resistance
#define CA_R		-5.20f			// cornering stiffness
#define CA_F		-5.0f			// cornering stiffness
#define MAX_GRIP	4.0f			// maximum (normalised) friction force, =diameter of friction circle
#define SCALE		40.0f


Vehicle::Vehicle(Entity *entity, carinfo_t *info) : RigidBody(entity)
{
	RigidBody::entity = entity;
	init(info);
}

void Vehicle::init(carinfo_t *info)
{
	Vehicle::info = *info;

	velocity.x = 0;
	velocity.y = 0;
	velocity.z = 0;

	angle_rad = 0;
	angularvelocity = 0;
	steerangle = 0;
	throttle = 0;
	brake = 0;
	gear = 2;
}

void Vehicle::step(float delta_t)
{
	vec3 accel;
	vec3 accel_rotated;
	vec3 vel;
	float yawspeed;
	float sn = sin(angle_rad);
	float cs = cos(angle_rad);
	float weight;

	vel.x = cs * velocity.z + sn * velocity.x;
	vel.z = -sn * velocity.z + cs * velocity.x;

	yawspeed = info.wheel_base * 0.5f * angularvelocity;

	if (vel.x == 0)
		rot_angle = 0;
	else
		rot_angle = atan2f(yawspeed, vel.x);

	if (vel.x == 0)
		sideslip = 0;
	else
		sideslip = atan2f(vel.z, vel.x);

	slipanglefront = sideslip + rot_angle - steerangle;
	slipanglerear = sideslip - rot_angle;

	weight = mass * 9.8f * 0.5f;

	flatf.x = 0;
	flatf.z = CA_F * slipanglefront;
	flatf.z = MIN(MAX_GRIP, flatf.z);
	flatf.z = MAX(-MAX_GRIP, flatf.z);
	flatf.z *= weight;
	if (front_slip)
		flatf.z *= 0.5;

	flatr.x = 0;
	flatr.z = CA_R * slipanglerear;
	flatr.z = MIN(MAX_GRIP, flatr.z);
	flatr.z = MAX(-MAX_GRIP, flatr.z);
	flatr.z *= weight;
	if (rear_slip)
		flatr.z *= 0.5f;

	float dir = -1;
	if (vel.x > 0)
	{
		dir = 1;
	}


	float wheel_rotation = 2.2f * (velocity.magnitude() / info.wheel_radius);
	rpm = wheel_rotation * info.gear_ratio[gear] * info.diff_ratio * 30.0f / (float)M_PI;

	if (rpm < info.min_rpm)
		rpm = info.min_rpm;
//	if (rpm > info.max_rpm)
//		engine_wear += 0.001f;
	if (rpm > info.redline_rpm)
	{
		rpm = info.redline_rpm;
		velocity *= 0.99f;
	}

	if (info.automatic && gear >= 1)
	{
		if (rpm > 5800)
			gear++;
		if (gear >= info.num_gear)
			gear = 6;

		if (rpm < 4000)
			gear--;
		if (gear <= 2)
			gear = 2;
	}

	int torque_index = (int)(rpm / 256.0f);

	if (torque_index > 40)
		torque_index = 40;

	ftraction.x = 0.01f * (info.torque_curve[torque_index] * info.gear_ratio[gear] * info.diff_ratio * info.efficiency / info.wheel_radius) * (throttle - brake * dir);
	ftraction.z = 0;

	if (rear_slip)
		ftraction.x *= 0.5f;

	resistance.x = -(RESISTANCE * vel.x + DRAG * vel.x * fabsf(vel.x));
	resistance.z = -(RESISTANCE * vel.z + DRAG * vel.z * fabsf(vel.z));

	force.x = ftraction.x + sin(steerangle) * flatf.x + flatr.x + resistance.x;
	force.z = ftraction.z + cos(steerangle) * flatf.z + flatr.z + resistance.z;

	torque = info.cg_to_front * flatf.z - info.cg_to_rear * flatr.z;

	accel.x = force.x / mass;
	accel.z = force.z / mass;

	angular_acceleration = torque / info.inertia;

	accel_rotated.x = cs * accel.z + sn * accel.x;
	accel_rotated.z = -sn * accel.z + cs * accel.x;

	velocity.x += delta_t * accel_rotated.x * SCALE;
	velocity.y = 0.0f;
	velocity.z += delta_t * accel_rotated.z * SCALE;

	entity->position.x += delta_t * velocity.x * SCALE;
	entity->position.y = 0.0f;
	entity->position.z += delta_t * velocity.z * SCALE;

	angularvelocity += delta_t * angular_acceleration;
	angle_rad += delta_t * angularvelocity;

}



bool Vehicle::move(input_t &input, float speed_scale, int tick_num)
{
	if (input.moveup)
	{
		if (throttle < 100)
			throttle += 10;
	}
	if (input.movedown)
	{
		if (throttle >= 10)
			throttle -= 10;
	}
	if (input.duck)
	{
		brake = 100;
		throttle = 0;
	}
	else
	{
		brake = 0;
	}


	if (input.weapon_down)
	{
		gear--;
		if (gear < 0)
			gear = 0;
	}
	if (input.weapon_up)
	{
		static int last_tick = 0;
		int current = tick_num;

		if (last_tick + 1000 < current)
		{
			last_tick = tick_num;
			gear++;
			if (gear >= 6)
				gear = 6;
		}
	}



	if (input.moveright)
	{
		if (steerangle > -M_PI / 4.0f)
			steerangle -= M_PI / 32.0f;
	}
	else if (input.moveleft)
	{
		if (steerangle <  M_PI / 4.0f)
			steerangle += M_PI / 32.0f;
	}
	else
	{
		steerangle *= 0.9f;
	}

	rear_slip = 0;
	front_slip = 0;

	if (input.jump)
		rear_slip = 1;

	if (input.control)
		front_slip = 1;

	if (input.control && input.jump)
	{
		front_slip = 1;
		rear_slip = 1;
	}



	return 0;
}
