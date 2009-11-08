#include "include.h"

#ifndef ENTITY_H
#define ENTITY_H

class Entity
{
public:
	Entity(const Entity &entity);

	Entity(float mass, const vec3 &position);
	~Entity();

	void render(Graphics &gfx);
	float *get_matrix(float *matrix);

	void integrate(float time);
	bool collision_detect(vec3 &v);
	bool collision_detect(Plane &p);
	bool collision_detect(Entity &entity);
	void impulse(Plane &plane, vec3 &vertex);
	bool in_frustum(Entity &entity);


	float			restitution;
	float			kfriction;
	//Physical
	float			mass;
	matrix3			inverse_tensor;
	matrix3			world_tensor;
	vec3			position;
	vec3			velocity;
	vec3			angular_velocity;
//	quaternion		orientation;
	matrix3			morientation;

	vec3			net_force;
	vec3			net_torque;

	//Visual
	vertex_t	*vertex_array;
	int			num_vertex;
	int			*index_array;
	int			num_index;

	//Virtual
	vec3			aabb[2];
	bool			sleep;
};
#endif
