#include "include.h"

#ifndef ENTITY_H
#define ENTITY_H

class Entity
{
public:
	Entity(const Entity &entity);

	Entity(float mass, const vec3 &position);

	vec3 velocity();
	void render(Graphics &gfx);
	float *get_matrix(float *matrix);

	void integrate(float time);
	bool collision_detect(vec3 &v);
	bool collision_detect(plane &p);
	bool collision_detect(Entity &entitiy);
	bool collision_resolve(Entity &entity);


	//Physical
	float			mass;
	matrix3			inertial_tensor;
	vec3			position;
	vec3			linear_momentum;
	vec3			angular_momentum;
	quaternion		orientation;
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
};

#endif

