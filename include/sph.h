#include "include.h"
#include <math.h>

#ifndef SPH_H
#define SPH_H

#define SCALE	0.04f
#define EPSILON 0.000001f
#define PMASS	0.010543f //kg 0.020543
#define I_STIFF 1
#define E_STIFF 10000
#define E_DAMP	256
#define REST_DENS	0.001f //kg.m-3

#define ACCEL_LIMIT	200 //m.s-2
#define VISC	0.05f
#define RAD		0.004f //m

#define DT		0.004f

#define kH		0.03f
#define kH2		(kH * kH)

#define POLY6_KERN		(315.0f / (64.0f * MY_PI * (float)pow(kH, 9)))
#define GRAD_POLY6_KERN 945.0f / (32.0f * MY_PI * (float)pow(h, 9));
#define LAP_POLY6_KERN  945.0f / (32.0f * MY_PI * (float)pow(h, 9));
#define SPIKY_KERN		(-45.0f / (MY_PI * (float)pow(kH, 6)))
#define VISCOSITY_KERN	(45.0f / (MY_PI * (float)pow(kH, 6)))

#define MAX_NEIGHBOR 32

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
	int nbList[MAX_NEIGHBOR];
	unsigned int hash;
} particle_t;

#define GRID_SIZE 1

#define MAX_GRID_PARTICLE 4000
typedef struct
{
	particle_t part[MAX_GRID_PARTICLE];
	int num_part;
} voxel_t;



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
	void update_grid();
	void update_neighbors();
	float norm2(vec3 &x, vec3 &y);


	particle_t *part;
	int num_particle;

	unsigned int last_rendered;
	unsigned int last_calculated;

	// define fluid bounds
	vec3 max_bound;
	vec3 min_bound;
	int grid_width;
	int grid_height;
	int grid_depth;
	voxel_t grid[GRID_SIZE][5][GRID_SIZE];
};

#endif