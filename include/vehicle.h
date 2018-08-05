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