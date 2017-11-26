#include "sph.h"

Sph::Sph()
{
	num_particle = 500;
	last_calculated = 0;
	last_rendered = 0;
	initialized = false;


	poly6_kern = POLY6_KERN;
	grad_poly6_kern = GRAD_POLY6_KERN;
	lap_poly6_kern = LAP_POLY6_KERN;
	spiky_kern = SPIKY_KERN;
	viscosity_kern = VISCOSITY_KERN;



	// small scale works better, scale up when displaying results
	max_bound = vec3(36.0f, 36.0f, 36.0f);
	min_bound = vec3(0.0f, 0.0f, 0.0f);

	float smoothing_length = 6.0f;
	grid_width = (int)((max_bound.x - min_bound.x) / smoothing_length);
	grid_height = (int)((max_bound.y - min_bound.y) / smoothing_length);
	grid_depth = (int)((max_bound.z - min_bound.z) / smoothing_length);

	part = NULL;
	init(500);
	initialized = true;
}

void Sph::init(int num_particle)
{
	Sph::num_particle = num_particle;
	if (part != NULL)
		delete[] part;

	// make points circles
#ifdef OPENGL
	glEnable(GL_POINT_SMOOTH);
#endif

	printf("Allocating %d particles\n", num_particle);
	part = new particle_t[num_particle];
	for (int i = 0; i < num_particle; i++)
	{
		memset(&part[i], 0, sizeof(particle_t));
		part[i].pos.x = rand_float(min_bound.x, max_bound.x);
		part[i].pos.y = rand_float(min_bound.y, max_bound.y);
		part[i].pos.z = rand_float(min_bound.z, max_bound.z);
		part[i].mass = kMass;
	}
	update_grid();
	update_neighbors();
}

// place all particles in bounds into grid
void Sph::update_grid()
{
	// reset grid to empty
	for (int i = 0; i < grid_width; i++)
	{
		for (int j = 0; j < grid_height; j++)
		{
			for (int k = 0; k < grid_depth; k++)
			{
				grid[i][j][k].num_part = 0;
			}
		}
	}


	for (int i = 0; i < num_particle; i++)
	{
		part[i].nbCount = 0;

		float xf = part[i].pos.x / (max_bound.x - min_bound.x);
		xf = clamp(xf, 0.0f, 1.0f);
		int x = (int)(xf * grid_width);

		float yf = part[i].pos.y / (max_bound.y - min_bound.y);
		yf = clamp(yf, 0.0f, 1.0f);
		int y = (int)(yf * grid_height);

		float zf = part[i].pos.z / (max_bound.z - min_bound.z);
		zf = clamp(zf, 0.0f, 1.0f);
		int z = (int)(zf * grid_width);

		if (x < 0 || y < 0 || z < 0)
			continue;

		x = y = z = 0;
		int num = grid[x][y][z].num_part;
		if (num < MAX_GRID_PARTICLE)
		{
			grid[x][y][z].part[num] = part[i];
			grid[x][y][z].num_part++;
		}
	}
}

// check for neighbor particles
void Sph::update_neighbors()
{
	float dr2 = 0;
	float lh2 = 4 * kH2;
	int max_neighbor = 0;

	// Update neighbors on a grid by grid basis (ignoring particles outside of same grid)
	// note: moved clearing of part[i].nbCount to update grid to cut out extra loop
	for (int x = 0; x < grid_width; x++)
	{
		for (int y = 0; y < grid_height; y++)
		{
			for (int z = 0; z < grid_depth; z++)
			{
				for (int i = 0; i < grid[x][y][z].num_part; i++)
				{
					for (int j = 0; j < grid[x][y][z].num_part; j++)
					{
						if (i >= j)
							continue;

						if (grid[x][y][z].part[i].nbCount >= MAX_NEIGHBOR)
						{
							break;
						}

						if (part[i].nbCount >= MAX_NEIGHBOR)
						{
							break;
						}

						if (part[i].nbCount > max_neighbor)
						{
							max_neighbor = part[i].nbCount;
							//							printf("max neighbor: %d\r\n", max_neighbor);
						}
						//calculate distance
						dr2 = 0;

						vec3 dist = (part[i].pos - part[j].pos);
						dr2 += dist * dist * kScale * kScale;

						//list neighborhood particle
						if (dr2 < lh2)
						{
							part[i].nbList[part[i].nbCount] = j;
							part[i].nbCount++;
							if (part[j].nbCount < MAX_NEIGHBOR)
							{
								part[j].nbList[part[j].nbCount] = i;
								part[j].nbCount++;
							}
						}
					}
				}
			}
		}
	}


}

void Sph::step(int frame)
{
	if (last_calculated == last_rendered)
	{
		update_grid();
		update_neighbors();

		// Lot's of particles, combine into single loop when possible
		for (int i = 0; i < num_particle; i++)
		{
			calc_density_pressure(i);
			calc_force(i);
			calc_pos(i);
		}
		last_calculated = frame;
	}
}

void Sph::calc_density_pressure(int i)
{
	float r2, dist;
	float density = 0.0f;

	// add density for all neighbors within smoothing radius kH (using sqaures of both to avoid sqrt)
	for (int j = 0; j < part[i].nbCount; j++)
	{
		int num;

		if (i >= j)
			continue;

		num = part[i].nbList[j];
		r2 = norm2(part[i].pos, part[num].pos) * kScale * kScale;

		if (kH2 < r2)
		{
			dist = kH2 - r2;
			// density
			density += dist * dist * dist;
		}
	}

	density += kH2 * kH2 * kH2;

	part[i].density = part[i].mass * poly6_kern * density;

	// PRESSURE
	//
	// Make the pressure is fudge factor times difference from resting density
	part[i].pressure = kStiffness * (part[i].density - kRestDensity);
}

//calculate acceleration and color field
void Sph::calc_force(int i)
{
	float pterm;
	float vterm;
	float r;
	float temp;
//	float temp2;
	vec3 pacc;
	vec3 vacc;
	int num = 0;


	pacc = vec3(0.0f, 0.0f, 0.0f);
	vacc = vec3(0.0f, 0.0f, 0.0f);

	for (int j = 0; j < part[i].nbCount; j++)
	{
		num = part[i].nbList[j];

		if (i != num)
		{
			r = newtonSqrt(norm2(part[i].pos, part[num].pos)) * kScale;

			if (r < kH)
			{
				// Compute force due to pressure and viscosity
				temp = kH - r;
				pterm = temp * temp * (part[i].pressure + part[num].pressure) / (r * part[num].density);
				vterm = temp / part[num].density;
				//temp2 = kH2 - r * r;

				pacc += (part[i].pos - part[num].pos) * pterm;
				vacc += (part[num].vel - part[i].vel) * vterm;
			}
		}
	}

	part[i].acc = (pacc  * -0.5f * spiky_kern
		+ vacc * viscosity_kern * kLinearViscocity) * part[i].mass / part[i].density;
}

//calculate position from forces and accelerations
void Sph::calc_pos(int i)
{
	float temp = 0.0f;

	vec3 zero(0.0f, 0.0f, 0.0f);
	//acceleration limit
	temp = norm2(part[i].acc, zero);
	if (kMaxAccel * kMaxAccel < temp)
	{
		part[i].acc *= kMaxAccel / newtonSqrt(temp);
	}

	//lower boundary condition
	temp = 2 * kRadius - (part[i].pos.y - min_bound.y) * kScale;
	if (temp > kEpsilon)
	{
		part[i].acc.y += kWallStiff * temp - kWallDamp * part[i].vel.y;
	}

	//upper boundary condition
	temp = 2 * kRadius - (max_bound.y - part[i].pos.y) * kScale;
	if (temp > kEpsilon)
	{
		part[i].acc.y += -kWallStiff * temp - kWallDamp * part[i].vel.y;
	}

	part[i].acc.y += -kGravity;

	part[i].vel += part[i].acc * kDeltaTime;
	part[i].pos += part[i].vel * kDeltaTime / kScale;

	// probably not so efficient, but noticed some particles escaping bounds
	if (part[i].pos.x > max_bound.x && part[i].vel.x > 0)
	{
		part[i].pos.x = clamp(part[i].pos.x, min_bound.x, max_bound.x);
		part[i].vel.x *= -1;
	}
	else if (part[i].pos.x < min_bound.x && part[i].vel.x < 0)
	{
		part[i].pos.x = clamp(part[i].pos.x, min_bound.x, max_bound.x);
		part[i].vel.x *= -1;
	}

	if (part[i].pos.z > max_bound.x && part[i].vel.z > 0)
	{
		part[i].pos.z = clamp(part[i].pos.z, min_bound.z, max_bound.z);
		part[i].vel.z *= -1;
	}
	else if (part[i].pos.z < min_bound.z && part[i].vel.z < 0)
	{
		part[i].pos.z = clamp(part[i].pos.z, min_bound.z, max_bound.z);
		part[i].vel.z *= -1;
	}

	if (part[i].pos.y > max_bound.y && part[i].vel.y > 0)
	{
		part[i].pos.y = clamp(part[i].pos.y, min_bound.y, max_bound.y);
		part[i].vel.y *= -1;
	}
	else if (part[i].pos.y < min_bound.y && part[i].vel.y < 0)
	{
		part[i].pos.y = clamp(part[i].pos.y, min_bound.y, max_bound.y);
		part[i].vel.y *= -1;
	}
}


inline float Sph::norm2(vec3 &a, vec3 &b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	float dz = a.z - b.z;

	return dx*dx + dy*dy + dz*dz;
}

void Sph::render()
{
	last_rendered = last_calculated;
#ifdef OPENGL
	// Draw Fluid Particles
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	for (int i = 0; i < num_particle; ++i)
	{
		//float c = 0.1f * part[i].pres;
		float x = 0.1f * part[i].nbCount;
		//float y = 0.5f * fabs(part[i].dens);

		glColor3f(0.0f, x, 0.3f + x);
		glVertex3f(part[i].pos.x, part[i].pos.y, part[i].pos.z);
	}
	glEnd();
#endif
}
