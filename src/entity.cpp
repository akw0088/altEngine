#include "entity.h"

void Entity_list::addEntity(Entity *entity)
{
	num_entities++;
	list_size++;

	if (list_size == num_entities)
	{
		Entity	**old = entity_list;

		entity_list = new Entity *[list_size];

		for (int i = 0; i < num_entities - 1; i++)
			entity_list[i] = old[i];

		delete [] old;
	}

	entity_list[list_size - 1] = entity;
}

void Entity_list::removeEntity()
{
	num_entities--;
	delete entity_list[num_entities];
}

Entity &Entity_list::operator[](int index)
{
	return *entity_list[index];
}

Entity_list::~Entity_list()
{
	while (num_entities != 0)
	{
		removeEntity();
	}
	delete [] entity_list;
	list_size = 0;
}

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

#define SCALAR 20.0f
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

Entity::~Entity()
{
	delete [] vertex_array;
	delete [] index_array;
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
	gfx.DrawArray("triangle", index_array, num_index);
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

bool Entity::collision_detect(Plane &p)
{
	for( int i = 0;	i < num_vertex; i++)
	{
		float d = vertex_array[i].vPosition * p.normal + p.d;

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

bool Entity::in_frustum(Entity &entity)
{
	Plane	left_plane, right_plane, bottom_plane, top_plane, near_plane, far_plane;
	float	m[16], p[16], r[16];
	int		i,j,k;

	/*
	get_matrix(m);
	glGetFloatv(GL_PROJECTION_MATRIX, p);

	for( i = 0; i < 4; i++)
	{
		for( j = 0; j < 4; j++)
		{
			r[i*4+j] = 0.0f;
			for(k = 0; k < 4; k++)
			{
				r[i*4+j] += m[i*4+k] * p[k*4+j];
			}
		}
	}

	left_plane.normal.x = r[12] + r[0];
	left_plane.normal.y = r[13] + r[1];
	left_plane.normal.z = r[14] + r[2];
	left_plane.normal.normalize();
	left_plane.d = r[15] + r[3];

	right_plane.normal.x = r[12] - r[0];
	right_plane.normal.y = r[13] - r[1];
	right_plane.normal.z = r[14] - r[2];
	right_plane.normal.normalize();
	right_plane.d = r[15] - r[3];

	bottom_plane.normal.x = r[12] + r[4];
	bottom_plane.normal.y = r[13] + r[5];
	bottom_plane.normal.z = r[14] + r[6];
	bottom_plane.normal.normalize();
	bottom_plane.d = r[15] + r[7];

	top_plane.normal.x = r[12] - r[4];
	top_plane.normal.y = r[13] - r[5];
	top_plane.normal.z = r[14] - r[6];
	top_plane.normal.normalize();
	top_plane.d = r[15] - r[7];

	near_plane.normal.x = r[12] + r[8];
	near_plane.normal.y = r[13] + r[9];
	near_plane.normal.z = r[14] + r[10];
	near_plane.normal.normalize();
	near_plane.d = r[15] + r[11];

	far_plane.normal.x = r[12] - r[8];
	far_plane.normal.y = r[13] - r[9];
	far_plane.normal.z = r[14] - r[10];
	far_plane.normal.normalize();
	far_plane.d = r[15] - r[11];
	*/

	left_plane.normal = vec3(-1.0f, 0.0f, 0.0f);
	left_plane.d = 50.0f;
	right_plane.normal = vec3(1.0f, 0.0f, 0.0f);
	right_plane.d = 50.0f;
	top_plane.normal = vec3(0.0f, -1.0f, 0.0f);
	top_plane.d = 50.0f;
	bottom_plane.normal = vec3(0.0f, 1.0f, 0.0f);
	bottom_plane.d = 50.0f;

	far_plane.normal = vec3(0.0f, 0.0f, -1.0f);
	far_plane.d = 100.0f;
	near_plane.normal = vec3(0.0f, 0.0f, 1.0f);
	near_plane.d = 0.0f;


	glColor3f(0.0f, 1.0f, 0.0f);
	left_plane.draw_plane();
	glColor3f(1.0f, 0.0f, 0.0f);
	right_plane.draw_plane();

	glColor3f(0.0f, 1.0f, 1.0f);
	top_plane.draw_plane();
	glColor3f(0.0f, 0.0f, 1.0f);
	bottom_plane.draw_plane();

	glColor3f(1.0f, 1.0f, 1.0f);
	near_plane.draw_plane();
	glColor3f(1.0f, 1.0f, 0.0f);
	far_plane.draw_plane();

	float result = left_plane.normal * entity.position - left_plane.d;

	if (result > 0)
	{
		result = right_plane.normal * entity.position - right_plane.d;
		if (result > 0)
		{
			result = top_plane.normal * entity.position - top_plane.d;
			if (result > 0)
			{
				result = bottom_plane.normal * entity.position - bottom_plane.d;
				if (result > 0)
				{
					result = near_plane.normal * entity.position - near_plane.d;
					if (result > 0)
					{
						result = far_plane.normal * entity.position - far_plane.d;
						if (result > 0)
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

