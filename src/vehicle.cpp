#include "vehicle.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Vehicle::Vehicle(Entity *entity)
: RigidBody(entity)
{
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
}

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


bool Vehicle::move(input_t &input, float speed_scale)
{
	if (input.moveup)
	{
		vec3 vec;
		getForwardVector(vec);
		vec.y = 0;
		entity->position += vec;
	}

	if (input.moveleft)
	{
		vec3 vec;
		getForwardVector(vec);
		vec.y = 0;
		entity->position -= vec3::crossproduct(vec, vec3(0.0, 1.0f, 0.0f));
	}

	if (input.moveright)
	{
		vec3 vec;
		getForwardVector(vec);
		vec.y = 0;
		entity->position += vec3::crossproduct(vec, vec3(0.0, 1.0f, 0.0f));
	}


	if (input.movedown)
	{
		vec3 vec;
		getForwardVector(vec);
		vec.y = 0;
		entity->position -= vec;
	}



	return true;
}