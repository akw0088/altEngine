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
	Vehicle(Entity *entity);
	void integrate2(float time);
	bool move(input_t &input, float speed_scale);

public:
	float	throttle_position;
private:
	float	kAir;
	float	kRoll;
	float	kBrake;
	float	brake_position;
	float	gear_ratio[7];
	int		gear;
	float	diff_ratio;
	float	efficiency;
	float	min_rpm;
	float	max_rpm;
	float	engine_wear;	// idea is to have this affect efficiency / blown engine
	float	wheel_radius;
	float	torque_curve[41];

	//Need to model wheel positions
	//using hardcoded constants for now
};

#endif