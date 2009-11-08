#include "entity.h"

Entity::Entity(const Entity &entity)
{
	memcpy(this, &entity, sizeof(Entity));
}

Entity::~Entity()
{
	if (vertex_array != NULL)
		delete [] vertex_array;
	if (index_array != NULL)
		delete [] index_array;
}

/*
	Integrate physical quantaties over time by a fixed time step
*/
void Entity::integrate(float time)
{
	matrix3 rotation;
	vec3 acceleration, angular_acceleration;

	if (sleep)
		return;

	acceleration = net_force * (1.0f / mass);
	angular_acceleration = world_tensor * net_torque;
	angular_velocity = angular_velocity + angular_acceleration * time;

	rotation.star(angular_velocity);

	//v = v0 + at
	velocity = velocity + acceleration * time;
	//x = x0 + vt + (1/2)at^2
	position = position + (velocity * time) * UNITS_TO_METERS;

	morientation = morientation + morientation * rotation * time;
	morientation.normalize();
	world_tensor = morientation * inverse_tensor * morientation.transpose();
}

/*
	Detects a collision with a plane and applys physical impulse response
	Friction forces shouldnt be in here, but rely on the same plane calculations.
*/
bool Entity::collision_detect(Plane &p)
{
	for( int i = 0;	i < num_vertex; i++)
	{
		vec3 point = morientation * vertex_array[i].vPosition;
		float d = (point + position) * p.normal + p.d;
		net_force = vec3(0.0f, -9.8f, 0.0f);

		if (d < 0.25f && d > 0.0f)
		{
			// Really close, add friction to CM
			vec3	vertex_velocity = velocity + vec3::crossproduct(angular_velocity, point * (1.0f / UNITS_TO_METERS));
			float	pdotv = abs(p.normal * vertex_velocity);
			float	friction_force = pdotv * mass * 1;
			vec3	friction_vector = -((p.normal * pdotv) + velocity);

			friction_vector.normalize();
			net_force += friction_vector * friction_force;
		}

		if ( d < -0.25f )
		{
			// Simulated too far
			return true;
		}
		else if ( d < 0.0f )
		{
			// Colliding
			// To handle multiple collision we need to store temp velocities
			impulse(p, point * (1.0f / UNITS_TO_METERS) );
			break;
		}
	}
	return false;
}

/*
	Applys collision impulse to a vertex
	radius must be in units of meters from CM
*/
void Entity::impulse(Plane &plane, vec3 &radius)
{
	float	impulse_numerator;
	float	impulse_denominator;
	vec3	impulse_force;

	vec3	vertex_vel_ang = vec3::crossproduct(angular_velocity, radius);
	vec3	vertex_velocity = velocity + vertex_vel_ang;

	// coefficient of resistution * -relative velocity
	impulse_numerator = -(1.0f + restitution) * (vertex_velocity * plane.normal);

	// 1/mass + N dot [((1/I)(radius cross normal)) cross radius] -- units of momentum p = mv
	impulse_denominator = (1.0f / mass) + plane.normal *
		vec3::crossproduct(world_tensor * vec3::crossproduct(radius, plane.normal), radius);
    
    impulse_force = plane.normal * (impulse_numerator/impulse_denominator);

	// apply impulse to primary quantities
    velocity += impulse_force * (1.0f / mass);
    angular_velocity = world_tensor * vec3::crossproduct(radius, -impulse_force);
}

/*
	Detects a collision between current entity and a point
*/
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

/*
	Detects a collision between current entity and another
	(Should use aabb, but is using vertices)
*/
bool Entity::collision_detect(Entity &entity)
{
	for( int i = 0;	i < entity.num_vertex; i++)
	{
		if ( collision_detect(entity.vertex_array[i].vPosition) )
			return true;
	}
	return false;
}

/*
	Frustum culling code that extracts frustum from the projection matrix
	doesnt work, dont know why, I need to write a specification for this
*/
bool Entity::in_frustum(Entity &entity)
{
	vec4	left_plane, right_plane, bottom_plane, top_plane, near_plane, far_plane;
	matrix4	p, m;

	get_matrix((float *)&m);
	glGetFloatv(GL_PROJECTION_MATRIX, (float *)&p);

	p = p * m;
	p.normalize();
	
	left_plane.x = p.m[12] + p.m[0];
	left_plane.y = p.m[13] + p.m[1];
	left_plane.z = p.m[14] + p.m[2];
	left_plane.w = p.m[15] + p.m[3];
	left_plane.normalize();

	right_plane.x = p.m[12] - p.m[0];
	right_plane.y = p.m[13] - p.m[1];
	right_plane.z = p.m[14] - p.m[2];
	right_plane.w = p.m[15] - p.m[3];
	right_plane.normalize();

	bottom_plane.x = p.m[12] + p.m[4];
	bottom_plane.y = p.m[13] + p.m[5];
	bottom_plane.z = p.m[14] + p.m[6];
	bottom_plane.w = p.m[15] + p.m[7];
	bottom_plane.normalize();

	top_plane.x = p.m[12] - p.m[4];
	top_plane.y = p.m[13] - p.m[5];
	top_plane.z = p.m[14] - p.m[6];
	top_plane.w = p.m[15] - p.m[7];
	top_plane.normalize();

	near_plane.x = p.m[12] + p.m[8];
	near_plane.y = p.m[13] + p.m[9];
	near_plane.z = p.m[14] + p.m[10];
	near_plane.w = p.m[15] + p.m[11];
	near_plane.normalize();

	far_plane.x = p.m[12] - p.m[8];
	far_plane.y = p.m[13] - p.m[9];
	far_plane.z = p.m[14] - p.m[10];
	far_plane.w = p.m[15] - p.m[11];
	far_plane.normalize();

	glColor3f(0.0f, 1.0f, 0.0f);
	((Plane)left_plane).draw_plane();
	glColor3f(1.0f, 0.0f, 0.0f);
	((Plane)right_plane).draw_plane();


	glColor3f(0.0f, 1.0f, 1.0f);
	((Plane)top_plane).draw_plane();
	glColor3f(0.0f, 0.0f, 1.0f);
	((Plane)bottom_plane).draw_plane();


	glColor3f(1.0f, 1.0f, 0.0f);
	((Plane)near_plane).draw_plane();
	glColor3f(1.0f, 1.0f, 1.0f);
	((Plane)far_plane).draw_plane();
	glColor3f(0.0f, 0.0f, 0.0f);

	float result = (vec3)left_plane * entity.position - left_plane.w;

	if (result > 0)
	{
//		glColor3f(0.0f, 1.0f, 0.0f);
		result = (vec3)right_plane * entity.position - right_plane.w;
		if (result > 0)
		{
//			glColor3f(1.0f, 0.0f, 0.0f);
			result = (vec3)top_plane * entity.position - top_plane.w;
			if (result > 0)
			{
//				glColor3f(0.0f, 1.0f, 1.0f);
				result = (vec3)bottom_plane * entity.position - bottom_plane.w;
				if (result > 0)
				{
//					glColor3f(0.0f, 0.0f, 1.0f);
					result = (vec3)near_plane * entity.position - near_plane.w;
					if (result > 0)
					{
//						glColor3f(1.0f, 1.0f, 0.0f);
						result = (vec3)far_plane * entity.position - far_plane.w;
						if (result > 0)
						{
//							glColor3f(1.0f, 1.0f, 1.0f);
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

Entity::Entity(float mass, const vec3 &position)
{
	sleep = false;
	Entity::mass = mass;
	Entity::position = position;
	restitution = 0.5f;
	kfriction = 0.5f;
	float height = 10.0f / UNITS_TO_METERS;
	float width = 10.0f / UNITS_TO_METERS;
	float depth = 10.0f / UNITS_TO_METERS;

	world_tensor.m[0] = 0.0f;
	world_tensor.m[1] = 0.0f;
	world_tensor.m[2] = 0.0f;
	world_tensor.m[3] = 0.0f;
	world_tensor.m[4] = 0.0f;
	world_tensor.m[5] = 0.0f;
	world_tensor.m[6] = 0.0f;
	world_tensor.m[7] = 0.0f;
	world_tensor.m[8] = 0.0f;

	inverse_tensor.m[0] = 3.0f / (mass *  (height * height + depth * depth));
	inverse_tensor.m[1] = 0;
	inverse_tensor.m[2] = 0;

	inverse_tensor.m[3] = 0;
	inverse_tensor.m[4] = 3.0f / (mass *  (width * width + depth * depth));
	inverse_tensor.m[5] = 0;
	
	inverse_tensor.m[6] = 0;
	inverse_tensor.m[7] = 0;
	inverse_tensor.m[8] = 3.0f / (mass *  (width * width + height * height));

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

	gfx.MultMatrix(get_matrix(matrix));
	gfx.VertexArray(vertex_array, num_vertex);
	gfx.DrawArray("triangle", index_array, num_index, num_vertex);
}