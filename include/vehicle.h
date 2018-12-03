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

#ifndef VEHICLE_H
#define VEHICLE_H

class Vehicle : public RigidBody
{
public:
	Vehicle(Entity *entity, carinfo_t *info);
	void init(carinfo_t *info);
	void step(float delta_t);
	bool move(input_t &input, float speed_scale, int tick_num);

public:
	vec3	force;
	vec3	resistance;
	vec3	ftraction;
	vec3	flatf, flatr;

	float	sideslip;
	float	slipanglefront;
	float	slipanglerear;
	float	torque;
	float	angular_acceleration;

	int		rear_slip;
	int		front_slip;


	//vec3	velocity;
	float	rot_angle;
	float	angle_rad;				// angle of car body orientation (in rads)
	float	angularvelocity;

	float	steerangle;			// angle of steering (input)
	float	throttle;			// amount of throttle (input)
	float	brake;				// amount of braking (input)

	int		gear;
	float	engine_wear;	// idea is to have this affect efficiency / blown engine
	float	rpm;


	carinfo_t info;
};


#endif
