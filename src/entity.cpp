#include "entity.h"

Entity::Entity(const Entity &entity)
{
	memcpy(this, &entity, sizeof(Entity));
}

Entity::Entity(float mass, const vec3 &position)
{
	Entity::mass = mass;
	Entity::position = position;

	linear_momentum.x = 0.0f;
	linear_momentum.y = 0.0f;
	linear_momentum.z = 0.0f;

	angular_momentum.x = 0.0f;
	angular_momentum.y = 0.0f;
	angular_momentum.z = 0.0f;

	morientation.m[0] = 1.0f;
	morientation.m[1] = 0.0f;
	morientation.m[2] = 0.0f;

	morientation.m[3] = 0.0f;
	morientation.m[4] = 1.0f;
	morientation.m[5] = 0.0f;

	morientation.m[6] = 0.0f;
	morientation.m[7] = 0.0f;
	morientation.m[8] = 1.0f;

	vertex_array = new vertex_t[8];

#define SCALAR 40.0f
	// Its a box! ?:)
	vertex_array[0].vPosition = vec3(-0.5f, -0.5f, -0.5f) * SCALAR;
	vertex_array[1].vPosition = vec3(-0.5f, -0.5f, 0.5f)  * SCALAR;
	vertex_array[2].vPosition = vec3(-0.5f, 0.5f, -0.5f)  * SCALAR;
	vertex_array[3].vPosition = vec3(-0.5f, 0.5f, 0.5f)   * SCALAR;
	vertex_array[4].vPosition = vec3(0.5f, -0.5f, -0.5f)  * SCALAR;
	vertex_array[5].vPosition = vec3(0.5f, -0.5f, 0.5f)   * SCALAR;
	vertex_array[6].vPosition = vec3(0.5f, 0.5f, -0.5f)   * SCALAR;
	vertex_array[7].vPosition = vec3(0.5f, 0.5f, 0.5f)    * SCALAR;
	num_vertex = 8;
	aabb[0] = vertex_array[0].vPosition;
	aabb[1] = vertex_array[7].vPosition;

	index_array = new int[36];
	index_array[0] = 0;
	index_array[1] = 2;
	index_array[2] = 1;

	index_array[3] = 1;
	index_array[4] = 2;
	index_array[5] = 3;

	index_array[6] = 7;
	index_array[7] = 5;
	index_array[8] = 1;

	index_array[9] = 7;
	index_array[10] = 1;
	index_array[11] = 3;

	index_array[12] = 5;
	index_array[13] = 7;
	index_array[14] = 6;

	index_array[15] = 5;
	index_array[16] = 6;
	index_array[17] = 4;

	index_array[18] = 0;
	index_array[19] = 6;
	index_array[20] = 2;

	index_array[21] = 0;
	index_array[22] = 4;
	index_array[23] = 6;

	index_array[24] = 0;
	index_array[25] = 1;
	index_array[26] = 4;

	index_array[27] = 1;
	index_array[28] = 5;
	index_array[29] = 4;

	index_array[30] = 2;
	index_array[31] = 6;
	index_array[32] = 3;

	index_array[33] = 3;
	index_array[34] = 6;
	index_array[35] = 7;

	num_index = 36;
}


vec3 Entity::velocity()
{
	return linear_momentum * (1.0f / mass);
}

float *Entity::get_matrix(float *matrix)
{
	matrix[0] = morientation.m[0];
	matrix[1] = morientation.m[1];
	matrix[2] = morientation.m[2];
	matrix[3] = 0.0f;

	matrix[4] = morientation.m[3];
	matrix[5] = morientation.m[4];
	matrix[6] = morientation.m[5];
	matrix[7] = 0.0f;

	matrix[8]  = morientation.m[6];
	matrix[9]  = morientation.m[7];
	matrix[10] = morientation.m[8];
	matrix[11] = 0.0f;

	matrix[12] = position.x;
	matrix[13] = position.y;
	matrix[14] = position.z;
	matrix[15] = 1.0f;
	return matrix;
}

void Entity::render(Graphics &gfx)
{
	float matrix[16];

	glMultMatrixf(get_matrix(matrix));
	gfx.VertexArray(vertex_array, num_vertex);
	gfx.DrawArray(index_array, num_index);
}

void Entity::integrate(float time)
{
	matrix3 rotation;

	rotation.m[0] = 0.0f;
	rotation.m[1] = -angular_momentum.z;
	rotation.m[2] = angular_momentum.y;

	rotation.m[3] = angular_momentum.z;
	rotation.m[4] = 0.0f;
	rotation.m[5] = -angular_momentum.x;

	rotation.m[6] = -angular_momentum.y;
	rotation.m[7] = angular_momentum.x;
	rotation.m[8] = 0.0f;

	position = position + velocity() * time;
	linear_momentum = linear_momentum + (net_force * time) * mass;
	morientation = morientation + morientation * rotation * time;
	morientation.normalize();
	orientation = orientation + (orientation * (angular_momentum * 0.5f)) * time;
}

bool Entity::collision_detect(vec3 &v)
{
	vec3 a, b;

	a = aabb[0] + position;
	b = aabb[1] + position;

	if ( (v.x > a.x) && (v.x < b.x) )
	{
		if ( (v.y > a.y) && (v.y < b.y) )
		{
			if ( (v.z > a.z) && (v.z < b.z) )
			{
				return true;
			}
		}
	}
	return false;
}

bool Entity::collision_detect(Entity &entity)
{
	for( int i = 0;	i < entity.num_vertex; i++)
	{
		if ( collision_detect(entity.vertex_array[i].vPosition) )
			return true;
	}
	return false;
}

bool Entity::collision_detect(plane &p)
{
	for( int i = 0;	i < num_vertex; i++)
	{
		float d = vertex_array[i].vPosition * vec3(p.a, p.b, p.c) + p.d;

		if ( d < -0.001f )
		{
			// Penetrating
			return true;
		}
		else if ( d < 0.001f)
		{
			// colliding
			return true;
		}
	}
	return false;	
}

