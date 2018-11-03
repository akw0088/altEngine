#ifndef CLOTH_H
#define CLOTH_H
#include "vector.h"
#include <vector>



#define DAMPING 0.01f
#define TIME_STEPSIZE2 0.125f
#define CONSTRAINT_ITERATIONS 15

namespace cloth
{
	class Particle
	{
	public:
		Particle(vec3 pos);
		Particle();
		void addForce(vec3 f);
		void timeStep();
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
		void init(float width, float height, int num_particles_width, int num_particles_height);
		void create_buffers(Graphics &gfx, unsigned int &vbo, unsigned int &num_vert, unsigned int &ibo, unsigned int &num_index, vec3 &scale, vec3 &offset);
		void timeStep();
		void addForce(const vec3 direction);
		void windForce(const vec3 direction);
		void ballCollision(const vec3 center, const float radius);

	private:
		int num_particles_width;
		int num_particles_height;

		std::vector<Particle> particles;
		std::vector<Constraint> constraints;

		Particle* getParticle(int x, int y);
		void makeConstraint(Particle *p1, Particle *p2);
		vec3 calcTriangleNormal(Particle *p1, Particle *p2, Particle *p3);
		void addWindForcesForTriangle(Particle *p1, Particle *p2, Particle *p3, const vec3 direction);
		void addTriangle(vertex_t *vertex_array, Particle *p1, Particle *p2, Particle *p3, const vec3 color, vec3 &scale, vec3 &offset);
	};

};
#endif