#include "include.h"

#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#define MAX_VELOCITY 800.0f

class RigidBody : public Model
{
public:
	RigidBody(Entity *entity);
	void recalc();
	virtual void integrate(float time);
	bool collision_detect(vec3 &v);
	bool collision_detect(Plane &p);
	bool collision_detect_simple(RigidBody &body);
	bool collision_detect(RigidBody &body);
	void impulse(Plane &plane, vec3 &vertex);
	void impulse(RigidBody &rigid, vec3 &point);
	void impulse(RigidBody &rigid, vec3 &point, Plane &plane);
	void frame2ent(Frame *camera_frame, button_t &keyboard);
	void save_config(cfg_t &config);
	void load_config(cfg_t &config);

	//AI
	void seek(vec3 position);
	void flee(vec3 position);
	void arrive(vec3 position);
	void pursue();
	void evade();
	void set_target(Entity &target);


	bool move(Frame &camera_frame, button_t &keyboard);
	float get_volume();
	float get_height();

	bool			map_collision;
	bool			pursue_flag;
	bool			step_flag;
	Entity			*target;
	float			restitution;
	bool			sleep;
	bool			gravity;
	bool			noclip;
	bool			water;
	bool			last_water;
	bool			rotational_friction_flag;
	bool			translational_friction_flag;
	float			rotational_friction;
	float			translational_friction;
	float			water_depth;
	//Physical
	float			mass;
	matrix3			inverse_tensor;
	matrix3			world_tensor;
	vec3			velocity;
	vec3			angular_velocity;
	vec3			net_force;
	vec3			net_torque;
	vec3			old_position;
	matrix3			old_orientation;
};

#endif
