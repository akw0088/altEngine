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
#define MAX_GRIP	2.0f			// maximum (normalised) friction force, =diameter of friction circle

Vehicle::Vehicle(Entity *entity)
{
	Vehicle::entity = entity;
	init();
#if 0
	kAir = 0.5f * 0.3f * 2.2f * 1.29f;
	kRoll = 30.0f * kAir;
	kBrake = 150.0f;

	mass = 1472.0f;
	throttle_position = 1.0f;
	brake_position = 0.0f;
	//wheel_base = 2.654300;

	//zeroth gear is reverse, corvette c5
	gear_ratio[0] = 2.9f;
	gear_ratio[1] = 2.66f;
	gear_ratio[2] = 1.78f;
	gear_ratio[3] = 1.3f;
	gear_ratio[4] = 1.0f;
	gear_ratio[5] = 0.74f;
	gear_ratio[6] = 0.5f;
	gear = 1;

	diff_ratio = 3.42f;
	efficiency = 1.0f;
	wheel_radius = 0.34f;
	min_rpm = 900.0f;
	max_rpm = 6000.0f;

	//torque in 256rpm increments
	//will be loaded from file in future.
	torque_curve[0] = 190.00000f;
	torque_curve[1] = 190.452438f;
	torque_curve[2] = 195.723282f;
	torque_curve[3] = 292.907043f;
	torque_curve[4] = 381.565521f;
	torque_curve[5] = 387.813019f;
	torque_curve[6] = 394.548645f;
	torque_curve[7] = 404.267029f;
	torque_curve[8] = 412.944153f;
	torque_curve[9] = 417.109161f;
	torque_curve[10] = 422.738434f;
	torque_curve[11] = 433.150970f;
	torque_curve[12] = 442.587341f;
	torque_curve[13] = 449.529053f;
	torque_curve[14] = 457.609619f;
	torque_curve[15] = 468.022156f;
	torque_curve[16] = 474.529999f;
	torque_curve[17] = 474.529999f;
	torque_curve[18] = 467.208679f;
	torque_curve[19] = 449.854431f;
	torque_curve[20] = 432.500214f;
	torque_curve[21] = 415.145966f;
	torque_curve[22] = 397.791718f;
	torque_curve[23] = 380.437469f;
	torque_curve[24] = 363.083252f;
	torque_curve[25] = 345.729004f;
	torque_curve[26] = 328.374756f;
	torque_curve[27] = 311.020508f;
	torque_curve[28] = 293.666290f;
	torque_curve[29] = 276.312042f;
	torque_curve[30] = 258.957794f;
	torque_curve[31] = 241.603561f;
	torque_curve[32] = 224.249313f;
	torque_curve[33] = 206.895081f;
	torque_curve[34] = 189.540833f;
	torque_curve[35] = 172.186600f;
	torque_curve[36] = 154.832367f;
	torque_curve[37] = 137.478119f;
	torque_curve[38] = 120.123878f;
	torque_curve[39] = 102.769638f;
	torque_curve[40] = 85.415398f;
#endif
}
#if 0
void Vehicle::integrate2(float time)
{
	RigidBody::integrate(time);

	vec3 drag_air = velocity * -kAir * velocity.magnitude();
	vec3 drag_roll = velocity * -kRoll;
	vec3 force_brake = velocity * -kBrake * brake_position;
	vec3 force_drive = vec3();
	vec3 force_engine = vec3();
	float wheel_rotation = velocity.magnitude() / wheel_radius;
	float rpm = wheel_rotation * gear_ratio[gear] * diff_ratio * 30.0f / MY_PI;

	if (rpm < min_rpm)
		rpm = min_rpm;
	else if (rpm > max_rpm)
		engine_wear += 0.001f;

	int torque_index = (int)(rpm / 256.0f);

	if (torque_index > 40)
		torque_index = 40;

	force_engine = velocity.normalize() * torque_curve[torque_index] * throttle_position;
	force_drive = force_engine * gear_ratio[gear] * diff_ratio * efficiency / wheel_radius;

	// Apply net engine force
	net_force += force_drive + drag_air + drag_roll + force_brake;
}
#endif

void Vehicle::init()
{
	// static data
	cg_to_front = 1.0f;					// meters
	cg_to_rear = 1.0f;					// meters
	cg_to_ground = 1.0f;				// meters
	mass = 1500;						// kg	
	inertia = 1500;						// kg.m			
	width = 1.5f;						// meters
	length = 3.0f;						// meters, must be > wheelbase
	wheel_length = 0.7f;
	wheel_width = 0.3f;
	wheel_base = cg_to_front + cg_to_rear;

	// dynamic data
	entity->position.x = 0;
	entity->position.y = 0;
	entity->position.z = 0;
	velocity.x = 0;
	velocity.y = 0;
	velocity.z = 0;

	angle = 0;
	angularvelocity = 0;
	steerangle = 0;
	throttle = 0;
	brake = 0;
}

void Vehicle::step(float delta_t)
{
	vec3 accel;
	vec3 accel_rotated;
	vec3 vel;
	float rot_angle;
	float yawspeed;
	float sn = sinf(angle);
	float cs = cosf(angle);
	float weight;

	vel.x = cs * velocity.y + sn * velocity.x;
	vel.y = -sn * velocity.y + cs * velocity.x;

	yawspeed = wheel_base * 0.5f * angularvelocity;

	if (vel.x == 0)
		rot_angle = 0;
	else
		rot_angle = atan2f(yawspeed, vel.x);

	if (vel.x == 0)
		sideslip = 0;
	else
		sideslip = atan2f(vel.y, vel.x);

	slipanglefront = sideslip + rot_angle - steerangle;
	slipanglerear = sideslip - rot_angle;

	weight = mass * 9.8f * 0.5f;

	flatf.x = 0;
	flatf.y = CA_F * slipanglefront;
	flatf.y = MIN(MAX_GRIP, flatf.y);
	flatf.y = MAX(-MAX_GRIP, flatf.y);
	flatf.y *= weight;
	if (front_slip)
		flatf.y *= 0.5;

	flatr.x = 0;
	flatr.y = CA_R * slipanglerear;
	flatr.y = MIN(MAX_GRIP, flatr.y);
	flatr.y = MAX(-MAX_GRIP, flatr.y);
	flatr.y *= weight;
	if (rear_slip)
		flatr.y *= 0.5f;

	float dir = -1;
	if (vel.x > 0)
	{
		dir = 1;
	}

	ftraction.x = 100 * (throttle - brake * dir);
	ftraction.y = 0;
	if (rear_slip)
		ftraction.x *= 0.5f;

	resistance.x = -(RESISTANCE * vel.x + DRAG * vel.x * fabsf(vel.x));
	resistance.y = -(RESISTANCE * vel.y + DRAG * vel.y * fabsf(vel.y));

	force.x = ftraction.x + sinf(steerangle) * flatf.x + flatr.x + resistance.x;
	force.y = ftraction.y + cosf(steerangle) * flatf.y + flatr.y + resistance.y;

	torque = cg_to_front * flatf.y - cg_to_rear * flatr.y;

	accel.x = force.x / mass;
	accel.y = force.y / mass;

	angular_acceleration = torque / inertia;

	accel_rotated.x = cs * accel.y + sn * accel.x;
	accel_rotated.y = -sn * accel.y + cs * accel.x;

	velocity.x += delta_t * accel_rotated.x;
	velocity.y += delta_t * accel_rotated.y;

	entity->position.x += delta_t * velocity.x;
	entity->position.y += delta_t * velocity.y;

	angularvelocity += delta_t * angular_acceleration;
	angle += delta_t * angularvelocity;

}



bool Vehicle::move(input_t &input, float speed_scale)
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
	if (input.moveleft)
	{
		if (steerangle > -M_PI / 4.0f)
			steerangle -= M_PI / 32.0f;
	}
	else if (input.moveright)
	{
		if (steerangle <  M_PI / 4.0f)
			steerangle += M_PI / 32.0f;
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