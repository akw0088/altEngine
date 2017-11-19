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


#define MAX_GRID_PARTICLE 1024
typedef struct
{
	particle_t *part[MAX_GRID_PARTICLE];
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
	voxel_t grid[4][4][4];
};

#endif