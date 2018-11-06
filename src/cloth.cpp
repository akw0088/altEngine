#include "include.h"

#define DAMPING 0.001f
#define TIME_STEPSIZE2 0.125f
#define CONSTRAINT_ITERATIONS 15


using namespace cloth;

Particle::Particle(vec3 pos)
{
	Particle::pos = pos;
	old_pos = pos;
	acceleration = vec3(0, 0, 0);
	mass = 1.0f;
	movable = true;
	accumulated_normal = vec3(0, 0, 0);
}

Particle::Particle()
{
}

void Particle::addForce(vec3 force)
{
	acceleration += force / mass;
}

void Particle::step()
{
	if (movable)
	{
		vec3 temp = pos;
		pos = pos + (pos - old_pos) * (1.0f - DAMPING) + acceleration * TIME_STEPSIZE2;
		old_pos = temp;
		acceleration = vec3(0, 0, 0);
	}
}

vec3& Particle::getPos()
{
	return pos;
}

void Particle::resetAcceleration()
{
	acceleration = vec3(0, 0, 0);
}

void Particle::offsetPos(const vec3 v)
{
	if (movable) pos += v;
}

void Particle::makeUnmovable()
{
	movable = false;
}

void Particle::addToNormal(vec3 normal)
{
	accumulated_normal += normal.normalize();
}

vec3& Particle::getNormal()
{
	return accumulated_normal;
}

void Particle::resetNormal()
{
	accumulated_normal = vec3(0, 0, 0);
}


Constraint::Constraint(Particle *p1, Particle *p2) : p1(p1), p2(p2)
{
	vec3 vec = p1->getPos() - p2->getPos();
	rest_distance = vec.magnitude();
}

void Constraint::satisfyConstraint()
{
	vec3 p1_to_p2 = p2->getPos() - p1->getPos();
	float current_distance = p1_to_p2.magnitude();
	vec3 correctionVector = p1_to_p2*(1.0f - rest_distance / current_distance);
	vec3 correctionVectorHalf = correctionVector * 0.5f;

	p1->offsetPos(correctionVectorHalf);
	p2->offsetPos(-correctionVectorHalf);
}

Cloth::Cloth()
{
	ibo = -1;
	vbo = -1;
}

void Cloth::init(float width, float height, int num_particles_width, int num_particles_height)
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
				makeConstraint(getParticle(x, y), getParticle(x + 1, y));
			}
			if (y < num_particles_height - 1)
			{
				makeConstraint(getParticle(x, y), getParticle(x, y + 1));
			}
			if (x < num_particles_width - 1 && y < num_particles_height - 1)
			{
				makeConstraint(getParticle(x, y), getParticle(x + 1, y + 1));
			}
			if (x < num_particles_width - 1 && y < num_particles_height - 1)
			{
				makeConstraint(getParticle(x + 1, y), getParticle(x, y + 1));
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
				makeConstraint(getParticle(x, y), getParticle(x + 2, y));
			}
			if (y < num_particles_height - 2)
			{
				makeConstraint(getParticle(x, y), getParticle(x, y + 2));
			}
			if (x < num_particles_width - 2 && y < num_particles_height - 2)
			{
				makeConstraint(getParticle(x, y), getParticle(x + 2, y + 2));
			}
			if (x < num_particles_width - 2 && y < num_particles_height - 2)
			{
				makeConstraint(getParticle(x + 2, y), getParticle(x, y + 2));
			}
		}
	}


	for (int i = 0; i < 3; i++)
	{
		getParticle(0 + i, 0)->offsetPos(vec3(0.5f, 0.0f, 0.0f));
		getParticle(0 + i, 0)->makeUnmovable();

		getParticle(0 + i, 0)->offsetPos(vec3(-0.5f, 0.0f, 0.0f));
		getParticle(num_particles_width - 1 - i, 0)->makeUnmovable();
	}
}

void Cloth::create_buffers(Graphics &gfx)
{
	std::vector<Particle>::iterator particle;
	for (particle = particles.begin(); particle != particles.end(); particle++)
	{
		(*particle).resetNormal();
	}

	for (int x = 0; x < num_particles_width - 1; x++)
	{
		for (int y = 0; y < num_particles_height - 1; y++)
		{
			vec3 normal = calcTriangleNormal(getParticle(x + 1, y), getParticle(x, y), getParticle(x, y + 1));
			getParticle(x + 1, y)->addToNormal(normal);
			getParticle(x, y)->addToNormal(normal);
			getParticle(x, y + 1)->addToNormal(normal);

			normal = calcTriangleNormal(getParticle(x + 1, y + 1), getParticle(x + 1, y), getParticle(x, y + 1));
			getParticle(x + 1, y + 1)->addToNormal(normal);
			getParticle(x + 1, y)->addToNormal(normal);
			getParticle(x, y + 1)->addToNormal(normal);
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
			addTriangle(&vertex_array[num_vert], getParticle(x + 1, y), getParticle(x, y), getParticle(x, y + 1), color);
			index_array[num_index] = num_index;
			index_array[num_index+1] = num_index+1;
			index_array[num_index+2] = num_index+2;
			num_index += 3;
			num_vert += 3;

			addTriangle(&vertex_array[num_vert], getParticle(x + 1, y + 1), getParticle(x + 1, y), getParticle(x, y + 1), color);
			index_array[num_index] = num_index;
			index_array[num_index + 1] = num_index + 1;
			index_array[num_index + 2] = num_index + 2;
			num_vert += 3;
			num_index += 3;


			//make double sided
			addTriangle(&vertex_array[num_vert], getParticle(x + 1, y), getParticle(x, y + 1), getParticle(x, y), color);
			index_array[num_index] = num_index;
			index_array[num_index + 1] = num_index + 1;
			index_array[num_index + 2] = num_index + 2;
			num_index += 3;
			num_vert += 3;

			addTriangle(&vertex_array[num_vert], getParticle(x + 1, y + 1), getParticle(x, y + 1), getParticle(x + 1, y), color);
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

void Cloth::step()
{
	std::vector<Constraint>::iterator constraint;
	for (int i = 0; i < CONSTRAINT_ITERATIONS; i++)
	{
		for (constraint = constraints.begin(); constraint != constraints.end(); constraint++)
		{
			(*constraint).satisfyConstraint();
		}
	}

	std::vector<Particle>::iterator particle;
	for (particle = particles.begin(); particle != particles.end(); particle++)
	{
		(*particle).step();
	}
}

void Cloth::addForce(const vec3 direction)
{
	std::vector<Particle>::iterator particle;
	for (particle = particles.begin(); particle != particles.end(); particle++)
	{
		(*particle).addForce(direction);
	}

}

void Cloth::windForce(const vec3 direction)
{
	for (int x = 0; x < num_particles_width - 1; x++)
	{
		for (int y = 0; y < num_particles_height - 1; y++)
		{
			addWindForcesForTriangle(getParticle(x + 1, y), getParticle(x, y), getParticle(x, y + 1), direction);
			addWindForcesForTriangle(getParticle(x + 1, y + 1), getParticle(x + 1, y), getParticle(x, y + 1), direction);
		}
	}
}

void Cloth::ballCollision(const vec3 center, const float radius)
{
	std::vector<Particle>::iterator particle;
	for (particle = particles.begin(); particle != particles.end(); particle++)
	{
		vec3 v = (*particle).getPos() - center;
		float l = v.magnitude();
		if (v.magnitude() < radius)
		{
			(*particle).offsetPos(v.normalize() * (radius - l));
		}
	}
}

Particle* Cloth::getParticle(int x, int y)
{
	return &particles[y * num_particles_width + x];
}
void Cloth::makeConstraint(Particle *p1, Particle *p2)
{
	constraints.push_back(Constraint(p1, p2));
}

vec3 Cloth::calcTriangleNormal(Particle *p1, Particle *p2, Particle *p3)
{
	vec3 pos1 = p1->getPos();
	vec3 pos2 = p2->getPos();
	vec3 pos3 = p3->getPos();

	vec3 v1 = pos2 - pos1;
	vec3 v2 = pos3 - pos1;

	return vec3::crossproduct(v1, v2);
}

void Cloth::addWindForcesForTriangle(Particle *p1, Particle *p2, Particle *p3, const vec3 direction)
{
	vec3 normal = calcTriangleNormal(p1, p2, p3);
	vec3 d = normal;
	d.normalize();
	vec3 force = normal * (d * direction);
	p1->addForce(force);
	p2->addForce(force);
	p3->addForce(force);
}

void Cloth::addTriangle(vertex_t *vertex_array, Particle *p1, Particle *p2, Particle *p3, const vec3 color)
{
	vertex_array[0].normal = p1->getNormal().normalize();
	vertex_array[0].position = p1->getPos();
	vertex_array[0].texCoord0.x = vertex_array[0].position.x / width;
	vertex_array[0].texCoord0.y = vertex_array[0].position.y / height;
	vertex_array[0].color = RGB(color.x * 255, color.y * 255, color.z * 255);

	vertex_array[1].normal = p2->getNormal().normalize();
	vertex_array[1].position = p2->getPos();
	vertex_array[1].texCoord0.x = vertex_array[1].position.x / width;
	vertex_array[1].texCoord0.y = vertex_array[1].position.y / height;
	vertex_array[1].color = RGB(color.x * 255, color.y * 255, color.z * 255);

	vertex_array[2].normal = p3->getNormal().normalize();
	vertex_array[2].position = p3->getPos();
	vertex_array[2].texCoord0.x = vertex_array[2].position.x / width;
	vertex_array[2].texCoord0.y = vertex_array[2].position.y / height;
	vertex_array[2].color = RGB(color.x * 255, color.y * 255, color.z * 255);
}
