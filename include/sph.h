#include "include.h"

#ifndef SPH_H
#define SPH_H

typedef struct
{
	vec3 pos;
	vec3 vel;
	vec3 acc;
	//	float mass;
	float pres;
	float dens;
	float color; //smoothed color field
	int nbCount;
	int nbList[32];
	unsigned int hash;
} particle_t;

typedef struct
{
	int r[100][100][100];
	int member[100];
	int num;
} grid_t;

class Sph
{
public:
	Sph();
	void init();
	void step(int frame);
	void render();

private:
	void calc_density_pressure();
	void calc_force();
	void calc_pos();
	double update_sum(int *x, vec3 &xp);
	void update_neighbors();
	float norm2(vec3 &x, vec3 &y);


	particle_t *part;
	int num_particle;

	unsigned int last_rendered;
	unsigned int last_calculated;

	// define fluid bounds
	vec3 bound_max;
	vec3 bound_min;
	int xd;
	int yd;
	int zd;
	grid_t grid;
};

#endif