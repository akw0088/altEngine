#include "sph.h"

Sph::Sph()
{
	num_particle = 500;
	last_calculated = 0;
	last_rendered = 0;



	// small scale works better, scale up when displaying results
	max_bound = vec3(6.0f, 18.0f, 6.0f);
	min_bound = vec3(0.0f, 0.0f, 0.0f);
	part = new particle_t[num_particle];

	float smoothing_length = 6.0;
	grid_width = (max_bound.x - min_bound.x) / smoothing_length;
	grid_height = (max_bound.y - min_bound.y) / smoothing_length;
	grid_depth = (max_bound.z - min_bound.z) / smoothing_length;

	init();
	update_grid();
	update_neighbors();
}

void Sph::init()
{
	for (int i = 0; i < num_particle; i++)
	{
		memset(&part[i], 0, sizeof(particle_t));
		part[i].pos.x = rand_float(min_bound.x, max_bound.x);
		part[i].pos.y = rand_float(min_bound.y, max_bound.y);
		part[i].pos.z = rand_float(min_bound.z, max_bound.z);
	}
}

// place all particles in bounds into grid
void Sph::update_grid()
{
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
		float xf = part[i].pos.x / (max_bound.x - min_bound.x);
		xf = clamp(xf, 0.0f, 1.0f);
		int x = xf * grid_width;

		float yf = part[i].pos.y / (max_bound.y - min_bound.y);
		yf = clamp(yf, 0.0f, 1.0f);
		int y = yf * grid_height;

		float zf = part[i].pos.z / (max_bound.z - min_bound.z);
		zf = clamp(zf, 0.0f, 1.0f);
		int z = zf * grid_width;
		
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


	// Initialize to zero
	for (int i = 0; i < num_particle; i++)
	{
		part[i].nbCount = 0;
		for (int j = 0; j < MAX_NEIGHBOR; j++)
		{
			part[i].nbList[j] = 0;
		}
	}

	// Update neighbors on a grid by grid basis (ignoring particles outside of same grid)
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
						dr2 += dist * dist * SCALE * SCALE;

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
		calc_density_pressure();
		calc_force();
		calc_pos();
		last_calculated = frame;
	}
}





// DENSITY 
//
// Calculate the density by basically making a weighted sum
// of the distances of neighboring particles within the radius of support (r)
void Sph::calc_density_pressure()
{
	float r2, sum, dist;
	int num = 0;
	float max_dens = 0.0f;
	float max_pressure = 0.0f;

	for (int i = 0; i < num_particle; i++)
	{
		sum = 0;
		for (int j = 0; j < part[i].nbCount; j++)
		{
			if (i >= j)
				continue;

			num = part[i].nbList[j];
			r2 = norm2(part[i].pos, part[j].pos) * SCALE * SCALE;

			if (kH2 < r2)
			{
				dist = kH2 - r2;
				// density
				sum += dist * dist * dist;
			}
		}

		sum += kH2 * kH2 * kH2;

		part[i].dens = PMASS * POLY6_KERN * sum;
		if (part[i].dens > max_dens)
		{
			max_dens = part[i].dens;
//			printf("Max density: %f\n", max_dens);
		}

		// PRESSURE
		//
		// Make the simple pressure calculation from the equation of state.
		part[i].pres = I_STIFF * (part[i].dens - REST_DENS);
		if (part[i].pres > max_pressure)
		{
			max_pressure = part[i].pres;
//			printf("Max pressure: %f\n", max_pressure);
		}
	}

}

//calculate acceleration and color field
void Sph::calc_force()
{
	float pterm, vterm, r, color, temp, temp2;
	vec3 pacc;
	vec3 vacc;
	int num = 0;

	//debug variable
	float min = 0;
	float max = 0;

	// PRESSURE FORCE
	//
	// We will force particles in or out from their neighbors
	// based on their difference from the rest density.
	for (int i = 0; i < num_particle; i++)
	{
		pacc = vec3(0.0f, 0.0f, 0.0f);
		vacc = vec3(0.0f, 0.0f, 0.0f);
		color = 1.0f;

		for (int j = 0; j < part[i].nbCount; j++)
		{
			num = part[i].nbList[j];

			if (i != num)
			{
				r = newtonSqrt(norm2(part[i].pos, part[num].pos)) * SCALE;

				if (r < kH)
				{
					// Compute force due to pressure and viscosity
					temp = kH - r;
					pterm = temp * temp * (part[i].pres + part[num].pres) / (r * part[num].dens);
					vterm = temp / part[num].dens;
					temp2 = kH2 - r * r;
					color += 1.0f / part[num].dens * temp2 * temp2 * temp2;

					pacc += (part[i].pos - part[num].pos) * pterm;
					vacc += (part[num].vel - part[i].vel) * vterm;
				}
			}
		}

		part[i].color = PMASS * POLY6_KERN * color;
		part[i].acc = (pacc  * -0.5 * SPIKY_KERN
			+ vacc * VISCOSITY_KERN * VISC) * PMASS / part[i].dens;

		//debug
		if (i == 0)
		{
			min = max = part[i].color;
		}
		else
		{
			temp = part[i].color;
			temp = clamp(temp, min, max);
		}
	}

}

//calculate position from forces and accelerations
void Sph::calc_pos()
{
	float temp;

	for (int i = 0; i < num_particle; i++)
	{
		vec3 zero(0.0f, 0.0f, 0.0f);
		//acceleration limit
		temp = norm2(part[i].acc, zero);
		if (ACCEL_LIMIT * ACCEL_LIMIT < temp)
		{
			part[i].acc *= ACCEL_LIMIT / sqrt(temp);
		}

		//lower boundary condition
		temp = 2 * RAD - (part[i].pos.y - min_bound.y) * SCALE;
		if (temp > EPSILON)
		{
			part[i].acc.y += E_STIFF * temp - E_DAMP * part[i].vel.y;
		}

		//upper boundary condition
		temp = 2 * RAD - (max_bound.y - part[i].pos.y) * SCALE;
		if (temp > EPSILON)
		{
			part[i].acc.y += -E_STIFF * temp - E_DAMP * part[i].vel.y;
		}

		part[i].acc.y += -9.8f;

		part[i].vel += part[i].acc * DT;
		part[i].pos += part[i].vel * DT / SCALE;

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
}


inline float Sph::norm2(vec3 &a, vec3 &b)
{
	//	return (a - b) * (a - b);
	return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z);
}

void Sph::render()
{
	last_rendered = last_calculated;
	// Draw Fluid Particles
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	for (int i = 0; i < num_particle; ++i)
	{
		float c = 0.1f * part[i].pres;
		float x = 0.1f * fabs(part[i].nbCount);
		float y = 0.5f * fabs(part[i].dens);

		glColor3f(0.0f, x, 0.3f + x);
		glVertex3f(part[i].pos.x, part[i].pos.y, part[i].pos.z);
	}
	glEnd();
}