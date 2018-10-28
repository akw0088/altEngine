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

class Vehicle : public Model
{
public:
	Vehicle(Entity *entity);
	void init();
	void step(float delta_t);
	bool move(input_t &input, float speed_scale);

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


	// car data
	//vec3	position;
	Entity *entity;
	vec3	velocity;

	float	angle;				// angle of car body orientation (in rads)
	float	angularvelocity;

	float	steerangle;			// angle of steering (input)
	float	throttle;			// amount of throttle (input)
	float	brake;				// amount of braking (input)

								// car static data
	float	wheel_base;		// wheel base in m
	float	cg_to_front;	// in m, distance from CG to front axle
	float	cg_to_rear;		// in m, idem to rear axle
	float	cg_to_ground;	// in m, height of CM from ground
	float	mass;			// in kg
	float	inertia;		// in kg.m
	float	length;
	float	width;
	float	wheel_length;
	float	wheel_width;


	// gearing
	float	gear_ratio[7];
	int		gear;
	float	diff_ratio;
	float	efficiency;
	float	min_rpm;
	float	max_rpm;
	float	redline_rpm;
	float	engine_wear;	// idea is to have this affect efficiency / blown engine
	float	wheel_radius;
	float	torque_curve[41];
	float	rpm;
	int automatic;
};


#endif