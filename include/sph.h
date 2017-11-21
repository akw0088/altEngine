#include "include.h"

#ifndef SPH_H
#define SPH_H

#define kMass	0.010543f //kg uniform particle mass
#define kRadius		0.005f //m -- physical particle radius (for collision detection against mainly)
#define kH (0.03f) //m 6 * kRadius -- radius particles will interact with neighbors
#define kH2		(0.0009f) //m^2 (kH * kH) -- radius of interaction squared
#define kStiffness 0.08f
#define kRestDensity	82.0f //kg.m^3
#define kLinearViscocity 0.02f



#define kGravity 9.8f
#define kWallStiff 10000
#define kWallDamp	256
#define kDeltaTime	0.004f
#define kMaxAccel	200 //m.s^2
#define kScale	0.04f
#define kEpsilon 0.000001f

/*
W(r-rb,h) = 315 / 64pi h^9(h^2 - |r-rb|^2)^3
gradW(r-rb,h) = -45 / pi h^6 (h - |r-rb|)^2 [r-rb]/[r-rb]
lapW(r-rb,h)= 45 / pi h^6 [ h - |r-rb|)
*/

#define POLY6_KERN		(315.0f / (64.0f * MY_PI * (float)pow(kH, 9)))
#define GRAD_POLY6_KERN 945.0f / (32.0f * MY_PI * (float)pow(kH, 9));
#define LAP_POLY6_KERN  945.0f / (32.0f * MY_PI * (float)pow(kH, 9));
#define SPIKY_KERN		(-45.0f / (MY_PI * (float)pow(kH, 6)))
#define VISCOSITY_KERN	(45.0f / (MY_PI * (float)pow(kH, 6)))

#define MAX_NEIGHBOR 64

typedef struct
{
	vec3 pos;
	vec3 vel;
	vec3 acc;
	float pres;
	float near_pres;
	float dens;
	float near_dens;
	int nbCount;
	int nbList[MAX_NEIGHBOR];
} particle_t;

#define GRID_SIZE 6

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
	void init(int num_particle);
	void step(int frame);
	void render();

private:
	void calc_density_pressure(int i);
	void calc_force(int i);
	void calc_pos(int i);
	void update_grid();
	void update_neighbors();
	float norm2(vec3 &x, vec3 &y);


	particle_t *part;
	bool initialized;
	unsigned int last_rendered;
	unsigned int last_calculated;
	int num_particle;


	// define fluid bounds
	vec3 max_bound;
	vec3 min_bound;
	int grid_width;
	int grid_height;
	int grid_depth;
	voxel_t grid[GRID_SIZE][GRID_SIZE][GRID_SIZE];



	// constant kernel factors
	float poly6_kern;
	float grad_poly6_kern;
	float lap_poly6_kern;
	float spiky_kern;
	float viscosity_kern;
};

#endif
