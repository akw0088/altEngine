#include "include.h"

#define DAMPING 0.001f
#define CONSTRAINT_ITERATIONS 5


using namespace cloth;
using namespace std;

Particle::Particle(vec3 &position)
{
	Particle::position = position;
	old_pos = position;
	acceleration = vec3(0, 0, 0);
	mass = 1.0f;
	movable = true;
	accumulated_normal = vec3(0, 0, 0);
}

Particle::Particle()
{
}

void Particle::add_force(const vec3 &force)
{
	acceleration += force / mass;
}

void Particle::step(float time)
{
	if (movable)
	{
		vec3 temp = position;
		position = position + (position - old_pos) * (1.0f - DAMPING) + acceleration * time;
		old_pos = temp;
		acceleration = vec3(0, 0, 0);
	}
}

Constraint::Constraint(Particle *p1, Particle *p2) : p1(p1), p2(p2)
{
	vec3 vec = p1->position - p2->position;
	rest_distance = vec.magnitude();
}

void Constraint::satisfy_constraint()
{
	vec3 p1_to_p2 = p2->position - p1->position;
	float current_distance = p1_to_p2.magnitude();
	vec3 correctionVector = p1_to_p2 * (1.0f - rest_distance / current_distance);
	vec3 correctionVectorHalf = correctionVector * 0.5f;

	if (p1->movable)
		p1->position += correctionVectorHalf;
	if (p2->movable)
		p2->position += -correctionVectorHalf;
}

Cloth::Cloth()
{
	ibo = -1;
	vbo = -1;
}

void Cloth::init(int width, int height, int num_particles_width, int num_particles_height)
{
	Cloth::num_particles_width = num_particles_width;
	Cloth::num_particles_height = num_particles_height;
	Cloth::width = width;
	Cloth::height = height;
	particles.resize(num_particles_width * num_particles_height);

	for (int x = 0; x < num_particles_width; x++)
	{
		for (int y = 0; y < num_particles_height; y++)
		{
			vec3 pos = vec3(width * (x / (float)num_particles_width),
				-height * (y / (float)num_particles_height),
				0);
			particles[y*num_particles_width + x] = Particle(pos);
		}
	}

	for (int x = 0; x < num_particles_width; x++)
	{
		for (int y = 0; y < num_particles_height; y++)
		{
			if (x < num_particles_width - 1)
			{
				make_constraint(get_particle(x, y), get_particle(x + 1, y));
			}
			if (y < num_particles_height - 1)
			{
				make_constraint(get_particle(x, y), get_particle(x, y + 1));
			}
			if (x < num_particles_width - 1 && y < num_particles_height - 1)
			{
				make_constraint(get_particle(x, y), get_particle(x + 1, y + 1));
			}
			if (x < num_particles_width - 1 && y < num_particles_height - 1)
			{
				make_constraint(get_particle(x + 1, y), get_particle(x, y + 1));
			}
		}
	}


	// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
	for (int x = 0; x < num_particles_width; x++)
	{
		for (int y = 0; y < num_particles_height; y++)
		{
			if (x < num_particles_width - 2)
			{
				make_constraint(get_particle(x, y), get_particle(x + 2, y));
			}
			if (y < num_particles_height - 2)
			{
				make_constraint(get_particle(x, y), get_particle(x, y + 2));
			}
			if (x < num_particles_width - 2 && y < num_particles_height - 2)
			{
				make_constraint(get_particle(x, y), get_particle(x + 2, y + 2));
			}
			if (x < num_particles_width - 2 && y < num_particles_height - 2)
			{
				make_constraint(get_particle(x + 2, y), get_particle(x, y + 2));
			}
		}
	}


	for (int i = 0; i < 3; i++)
	{
		Particle *p1 = get_particle(0 + i, 0);
		Particle *p2 = get_particle(num_particles_width - 1 - i, 0);
		if (p1->movable)
		{
			p1->position += vec3(0.5f, 0.0f, 0.0f);
		}
		p1->movable = false;


		if (p2->movable)
			p2->position += vec3(-0.5f, 0.0f, 0.0f);
		p2->movable = false;
	}
}



void Cloth::step(float time)
{
	for (int i = 0; i < CONSTRAINT_ITERATIONS; i++)
	{
		for (unsigned int j = 0; j < constraints.size(); j++)
		{
			constraints[j].satisfy_constraint();
		}
	}

	for (unsigned int i = 0; i < particles.size(); i++)
	{
		particles[i].step(time);
	}
}

void Cloth::add_force(const vec3 &direction)
{
	for (unsigned int i = 0; i < particles.size(); i++)
	{
		particles[i].add_force(direction);
	}
}

void Cloth::wind_force(const vec3 &direction)
{
	for (int x = 0; x < num_particles_width - 1; x++)
	{
		for (int y = 0; y < num_particles_height - 1; y++)
		{
			add_wind_forces(get_particle(x + 1, y), get_particle(x, y), get_particle(x, y + 1), direction);
			add_wind_forces(get_particle(x + 1, y + 1), get_particle(x + 1, y), get_particle(x, y + 1), direction);
		}
	}
}

void Cloth::add_wind_forces(Particle *p1, Particle *p2, Particle *p3, const vec3 &direction)
{
	vec3 normal = calc_normal(p1, p2, p3);
	vec3 d = normal;
	d.normalize();
	vec3 force = normal * (d * direction);
	p1->add_force(force);
	p2->add_force(force);
	p3->add_force(force);
}

void Cloth::ball_collision(const vec3 &center, const float radius)
{
	for (unsigned int i = 0; i < particles.size(); i++)
	{
		vec3 v = particles[i].position - center;
		float l = v.magnitude();
		if (v.magnitude() < radius)
		{
			particles[i].position += v.normalize() * (radius - l);
		}
	}
}

inline Particle* Cloth::get_particle(int x, int y)
{
	return &particles[y * num_particles_width + x];
}

inline void Cloth::make_constraint(Particle *p1, Particle *p2)
{
	constraints.push_back(Constraint(p1, p2));
}

vec3 Cloth::calc_normal(Particle *p1, Particle *p2, Particle *p3)
{
	vec3 pos1 = p1->position;
	vec3 pos2 = p2->position;
	vec3 pos3 = p3->position;

	vec3 v1 = pos2 - pos1;
	vec3 v2 = pos3 - pos1;

	return vec3::crossproduct(v1, v2);
}


void Cloth::add_triangle(vertex_t *vertex_array, Particle *p1, Particle *p2, Particle *p3, const vec3 &color)
{
	vertex_array[0].normal = p1->accumulated_normal.normalize();
	vertex_array[0].position = p1->position;
	vertex_array[0].texCoord0.x = vertex_array[0].position.x / width;
	vertex_array[0].texCoord0.y = vertex_array[0].position.y / height;
//	vertex_array[0].color = RGB(color.x * 255, color.y * 255, color.z * 255);

	vertex_array[1].normal = p2->accumulated_normal.normalize();
	vertex_array[1].position = p2->position;
	vertex_array[1].texCoord0.x = vertex_array[1].position.x / width;
	vertex_array[1].texCoord0.y = vertex_array[1].position.y / height;
//	vertex_array[1].color = RGB(color.x * 255, color.y * 255, color.z * 255);

	vertex_array[2].normal = p3->accumulated_normal.normalize();
	vertex_array[2].position = p3->position;
	vertex_array[2].texCoord0.x = vertex_array[2].position.x / width;
	vertex_array[2].texCoord0.y = vertex_array[2].position.y / height;
//	vertex_array[2].color = RGB(color.x * 255, color.y * 255, color.z * 255);
}

void Cloth::create_buffers(Graphics &gfx)
{

	for (unsigned int i = 0; i < particles.size(); i++)
	{
		particles[i].accumulated_normal = vec3(0.0f, 0.0f, 0.0f);
	}

	for (int x = 0; x < num_particles_width - 1; x++)
	{
		for (int y = 0; y < num_particles_height - 1; y++)
		{
			vec3 normal = calc_normal(get_particle(x + 1, y), get_particle(x, y), get_particle(x, y + 1));
			get_particle(x + 1, y)->accumulated_normal += normal;
			get_particle(x, y)->accumulated_normal += normal;
			get_particle(x, y + 1)->accumulated_normal += normal;

			normal = calc_normal(get_particle(x + 1, y + 1), get_particle(x + 1, y), get_particle(x, y + 1));
			get_particle(x + 1, y + 1)->accumulated_normal += normal;
			get_particle(x + 1, y)->accumulated_normal += normal;
			get_particle(x, y + 1)->accumulated_normal += normal;
		}
	}

	static vertex_t *vertex_array = NULL;
	if (vertex_array == NULL)
	{
		vertex_array = new vertex_t[2 * 2052];
	}
	num_vert = 0;
	num_index = 0;

	static int *index_array = NULL;
	if (index_array == NULL)
	{
		index_array = new int[2 * 2052];
	}

	for (int x = 0; x < num_particles_width - 1; x++)
	{
		for (int y = 0; y < num_particles_height - 1; y++)
		{
			vec3 color(0, 0, 0);
			if (x % 2) // red and white color is interleaved according to which column number
				color = vec3(0.6f, 0.2f, 0.2f);
			else
				color = vec3(1.0f, 1.0f, 1.0f);

			// add frontwards
			add_triangle(&vertex_array[num_vert], get_particle(x + 1, y), get_particle(x, y), get_particle(x, y + 1), color);
			index_array[num_index] = num_index;
			index_array[num_index + 1] = num_index + 1;
			index_array[num_index + 2] = num_index + 2;
			num_index += 3;
			num_vert += 3;

			add_triangle(&vertex_array[num_vert], get_particle(x + 1, y + 1), get_particle(x + 1, y), get_particle(x, y + 1), color);
			index_array[num_index] = num_index;
			index_array[num_index + 1] = num_index + 1;
			index_array[num_index + 2] = num_index + 2;
			num_vert += 3;
			num_index += 3;


			//make double sided
			add_triangle(&vertex_array[num_vert], get_particle(x + 1, y), get_particle(x, y + 1), get_particle(x, y), color);
			index_array[num_index] = num_index;
			index_array[num_index + 1] = num_index + 1;
			index_array[num_index + 2] = num_index + 2;
			num_index += 3;
			num_vert += 3;

			add_triangle(&vertex_array[num_vert], get_particle(x + 1, y + 1), get_particle(x, y + 1), get_particle(x + 1, y), color);
			index_array[num_index] = num_index;
			index_array[num_index + 1] = num_index + 1;
			index_array[num_index + 2] = num_index + 2;
			num_vert += 3;
			num_index += 3;
		}
	}

	if (ibo != -1)
	{
		gfx.DeleteIndexBuffer(ibo);
	}
	if (vbo != -1)
	{
		gfx.DeleteVertexBuffer(vbo);
	}
	vbo = gfx.CreateVertexBuffer(vertex_array, num_vert, true);
	ibo = gfx.CreateIndexBuffer(index_array, num_index);
}