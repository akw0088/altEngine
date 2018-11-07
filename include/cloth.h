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
		void add_force(vec3 f);
		void step();

		bool movable;
		vec3 position;
		vec3 accumulated_normal;
		float mass;
		vec3 old_pos;
		vec3 acceleration;
	};

	class Constraint
	{
	public:
		Constraint(Particle *p1, Particle *p2);
		void satisfy_constraint();

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
		void add_force(const vec3 direction);
		void wind_force(const vec3 direction);
		void ball_collision(const vec3 center, const float radius);


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

		inline Particle* get_particle(int x, int y);
		void make_constraint(Particle *p1, Particle *p2);
		vec3 calc_normal(Particle *p1, Particle *p2, Particle *p3);
		void add_wind_forces(Particle *p1, Particle *p2, Particle *p3, const vec3 direction);
		void add_triangle(vertex_t *vertex_array, Particle *p1, Particle *p2, Particle *p3, const vec3 color);
	};

};
#endif