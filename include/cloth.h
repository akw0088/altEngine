#ifndef CLOTH_H
#define CLOTH_H
#include "vector.h"
#include <vector>

namespace cloth
{
	class Particle
	{
	public:
		Particle(vec3 pos);
		Particle();
		void addForce(vec3 f);
		void step();
		vec3& getPos();
		void resetAcceleration();
		void offsetPos(const vec3 v);
		void makeUnmovable();
		void addToNormal(vec3 normal);
		vec3& getNormal();
		void resetNormal();

	private:
		bool movable;

		float mass;
		vec3 pos;
		vec3 old_pos;
		vec3 acceleration;
		vec3 accumulated_normal;
	};

	class Constraint
	{
	public:
		Constraint(Particle *p1, Particle *p2);
		void satisfyConstraint();

		Particle *p1, *p2;
	private:
		float rest_distance;
	};

	class Cloth
	{
	public:
		Cloth();
		void init(int width, int height, int num_particles_width, int num_particles_height);
		void create_buffers(Graphics &gfx);
		void step();
		void addForce(const vec3 direction);
		void windForce(const vec3 direction);
		void ballCollision(const vec3 center, const float radius);


		unsigned int ibo;
		unsigned int vbo;
		unsigned int tex;
		unsigned int num_index;
		unsigned int num_vert;

	private:
		int num_particles_width;
		int num_particles_height;
		int width;
		int height;

		std::vector<Particle> particles;
		std::vector<Constraint> constraints;

		Particle* getParticle(int x, int y);
		void makeConstraint(Particle *p1, Particle *p2);
		vec3 calcTriangleNormal(Particle *p1, Particle *p2, Particle *p3);
		void addWindForcesForTriangle(Particle *p1, Particle *p2, Particle *p3, const vec3 direction);
		void addTriangle(vertex_t *vertex_array, Particle *p1, Particle *p2, Particle *p3, const vec3 color);
	};

};
#endif