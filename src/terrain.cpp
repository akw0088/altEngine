
#include "terrain.h"
#include "stb_image.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


Terrain::Terrain()
{
	vbo = -1;
	ibo = -1;
	vertex_array = NULL;
	index_array = NULL;
	num_index = 0;
	num_vertex = 0;
	loaded = false;
}

int Terrain::load(Graphics &gfx, float scale, vec3 &offset, char *heightmap, char *texture_str, bool sphere, int anisotropic)
{
	if (sphere)
	{
		CreateSphere(heightmap, scale, offset, vertex_array, index_array, num_vertex, num_index);
	}
	else
	{
		CreateMesh(heightmap, scale, scale, offset, vertex_array, index_array, num_vertex, num_index);
		CreateWater(scale, scale, water_array, water_index, water_nvertex, water_nindex);
	}

	water_vbo = gfx.CreateVertexBuffer(water_array, water_nvertex, false);
	water_ibo = gfx.CreateIndexBuffer(water_index, water_nindex);
	water_tex = load_texture(gfx, "media/terrain/blue.png", false, false, anisotropic);



	vbo = gfx.CreateVertexBuffer(vertex_array, num_vertex, false);
	ibo = gfx.CreateIndexBuffer(index_array, num_index);
	terrain_tex = load_texture(gfx, texture_str, false, false, anisotropic);
	loaded = true;
//	WriteObj("terrain.obj", vertex_array, num_vertex, index_array, num_index);
	return 0;
}


void Terrain::CreateSphere(char *heightmap, float radius, vec3 &offset, vertex_t *&vertex, unsigned int *&index, unsigned int &num_vertex, unsigned int &num_index)
{
	float theta1 = 0, theta2 = 0, theta3 = 0;
	float xcoord = 0;
	float ycoord = 0;
	float ex = 0, px = 0, cx = xcoord;
	float ey = 0, py = 0, cy = ycoord;
	float ez = 0, pz = 0, cz = 0, r = radius;
	int k = 0;

	int width;
	int height;
	int components;

	int size = 0;
	unsigned char *data = (unsigned char *)get_file(heightmap, &size);
	unsigned char *image = stbi_load_from_memory(data, size, &width, &height, &components, 0);

	int sides = width;

	vertex = new vertex_t[sides * (sides + 1)];
	num_vertex = sides * (sides + 1);


	
	for (int j = 0; j < sides / 2; j++)
	{
		theta1 = j * (2 * MY_PI) / sides - MY_PI / 2;
		theta2 = (j + 1) * (2 * MY_PI) / sides - MY_PI / 2;

		for (int i = 0; i <= sides; i++)
		{
			int index = j * sides + i;

			theta3 = i * (2 * MY_PI) / sides;

			ex = (float)(fcos(theta1) * fcos(theta3));
			ey = (float)fsin(theta1);
			ez = (float)(fcos(theta1) * fsin(theta3));
			px = cx + r * ex;
			py = cy + r * ey;
			pz = cz + r * ez;

			vertex[k].normal = vec3(ex, ey, ez);
			vertex[k].texCoord0.x = i / (float)sides;
			vertex[k].texCoord0.y = 2 * j / (float)sides;
			vertex[k].position = vec3(px, py, pz) * image[index * sizeof(int)];
			k++;

			ex = (float)(fcos(theta2) * fcos(theta3));
			ey = (float)fsin(theta2);
			ez = (float)(fcos(theta2) * fsin(theta3));
			px = cx + r * ex;
			py = cy + r * ey;
			pz = cz + r * ez;

			vertex[k].normal = vec3(ex, ey, ez);
			vertex[k].texCoord0.x = i / (float)sides;
			vertex[k].texCoord0.y = 2 * (j + 1) / (float)sides;
			vertex[k].position = vec3(px, py, pz)  * image[index * sizeof(int)];


			if (i == 0 && j > 0)
			{
				vertex[k - (sides + 1)].position = vertex[k].position;
				vertex[k - (sides + 1)].position = vertex[k - 1].position;
			}

			k++;
		}
	}
	index = new unsigned int[k * 3];
	num_vertex = k;

	unsigned int j = 0;
	for (unsigned int i = 0; i < num_vertex; i += 2)
	{
		// read quad strip, generate two triangles
		if (i == 0)
		{
			index[j + 0] = i + 2;
			index[j + 1] = i + 1;
			index[j + 2] = i + 0;

			index[j + 3] = i + 1;
			index[j + 4] = i + 2;
			index[j + 5] = i + 3;
			j += 6;
			i += 2;
		}
		else
		{
			index[j + 0] = i + 0;
			index[j + 1] = i - 1;
			index[j + 2] = i - 2;

			index[j + 3] = i - 1;
			index[j + 4] = i + 0;
			index[j + 5] = i + 1;
			j += 6;
		}
	}
	num_index = j;
}


void Terrain::Smooth(vertex_t *image, int width, int height, vec3 &avg)
{
	float kernel[9] = {
		1.0f / 9.0f, 1.0f / 9.0f,  1.0f / 9.0f,
		1.0f / 9.0f, 1.0f / 9.0f,  1.0f / 9.0f,
		1.0f / 9.0f, 1.0f / 9.0f,  1.0f / 9.0f
	};

	avg.x = 0.0f;
	avg.y = 0.0f;
	avg.z = 0.0f;
	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{

			image[y * width + x].position.y =
				image[(y - 1) * width + (x - 1)].position.y * kernel[0] +
				image[(y - 1) * width + (x + 0)].position.y * kernel[1] +
				image[(y - 1) * width + (x + 1)].position.y * kernel[2] +

				image[(y)* width + (x - 1)].position.y * kernel[3] +
				image[(y)* width + (x + 0)].position.y * kernel[4] +
				image[(y)* width + (x + 1)].position.y * kernel[5] +

				image[(y + 1) * width + (x - 1)].position.y * kernel[6] +
				image[(y + 1) * width + (x + 0)].position.y * kernel[7] +
				image[(y + 1) * width + (x + 1)].position.y * kernel[8];
			avg += image[y* width + x + 0].position;
		}
	}

	avg *= 1.0f / ((width - 2) * (height - 2));
}

int Terrain::CreateWater(float scale_width, float scale_height, vertex_t *&vertex, unsigned int *&index, unsigned int &num_vertex, unsigned int &num_index)
{
	float half_length = scale_width * (num_row >> 1);

	vertex = new vertex_t[4];

	vertex[0].position.x = offset.x - half_length;
	vertex[0].position.y = avg.y;
	vertex[0].position.z = offset.z - half_length;
	vertex[0].texCoord0 = vec2(0.0f, 0.0f);
	vertex[0].color = ~0;
	vertex[0].normal = vec3(0.0f, 1.0f, 0.0f);

	vertex[1].position.x = offset.x + half_length;
	vertex[1].position.y = avg.y;
	vertex[1].position.z = offset.z - half_length;
	vertex[1].texCoord0 = vec2(1.0f, 0.0f);
	vertex[1].color = ~0;
	vertex[1].normal = vec3(0.0f, 1.0f, 0.0f);

	vertex[2].position.x = offset.x - half_length;
	vertex[2].position.y = avg.y;
	vertex[2].position.z = offset.z + half_length;
	vertex[2].texCoord0 = vec2(0.0f, 1.0f);
	vertex[2].color = ~0;
	vertex[2].normal = vec3(0.0f, 1.0f, 0.0f);

	vertex[3].position.x = offset.x + half_length;
	vertex[3].position.y = avg.y;
	vertex[3].position.z = offset.z + half_length;
	vertex[3].texCoord0 = vec2(1.0f, 1.0f);
	vertex[3].color = ~0;
	vertex[3].normal = vec3(0.0f, 1.0f, 0.0f);

	num_vertex = 4;

	index = new unsigned int[6];

	index[0] = 0;
	index[1] = 1;
	index[2] = 2;

	index[3] = 3;
	index[4] = 2;
	index[5] = 1;
	num_index = 6;
	return 0;
}

int Terrain::CreateMesh(char *heightmap, float scale_width, float scale_height, vec3 &offset, vertex_t *&vertex, unsigned int *&index, unsigned int &num_vertex, unsigned int &num_index)
{
	int width;
	int height;
	int components;
	unsigned int i;

	int size = 0;
	unsigned char *data = (unsigned char *)get_file(heightmap, &size);
	unsigned char *image = stbi_load_from_memory(data, size, &width, &height, &components, 0);

	num_vertex = width * height;
	vertex = new vertex_t[width * height + 4];

	num_index = width * height * 3 * 2;
	index = new unsigned int[num_index];


	Terrain::size = (float)(width * scale_width);
	num_col = height;
	num_row = width;
	trilength = (float)size / num_row;


	float half_length = scale_width * (width >> 1);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int index = y * width + x;
			vertex[index].position.x = scale_width * x - half_length + offset.x;
			vertex[index].position.y = scale_height * image[index * sizeof(int)] + offset.y;
			vertex[index].position.z = scale_width * y - half_length + offset.z;

			vertex[index].texCoord0.x = (float)x / (width - 1);
			vertex[index].texCoord0.y = (float)y / (height - 1);
			vertex[index].texCoord1.x = (float)x / (width - 1);
			vertex[index].texCoord1.y = (float)y / (height - 1);
			vertex[index].color = ~0;
		}
	}

	Smooth(vertex, width, height, avg);

	// pulled from tessellate bezier curve -- glad I dont have to write this twice ;)
	unsigned int num_row = width;
	// generate index array
	unsigned int j = 0;
	for (i = 0; j < num_row * (num_row - 1);)
	{
		// Dont connect top of row to bottom of next row
		if ((j + 1) % (num_row) == 0)
		{
			j++;
			continue;
		}

		if (i >= num_index)
			break;

		index[i + 2] = j;
		index[i + 1] = num_row + j;
		index[i + 0] = j + 1;

		index[i + 5] = num_row + j;
		index[i + 4] = num_row + j + 1;
		index[i + 3] = j + 1;

		// generate normals
		vec3 a = vertex[index[i + 1]].position - vertex[index[i + 0]].position;
		vec3 b = vertex[index[i + 2]].position - vertex[index[i + 0]].position;
		vec3 normal = vec3::crossproduct(a, b);

		vertex[index[i + 0]].normal = normal;
		vertex[index[i + 1]].normal = normal;
		vertex[index[i + 2]].normal = normal;

		vertex[index[i + 3]].normal = normal;
		vertex[index[i + 4]].normal = normal;
		vertex[index[i + 5]].normal = normal;

		j++;
		i += 6;
	}
	num_index = i;
	stbi_image_free(image);
	free((void *)data);
	return 0;
}

void Terrain::render(Graphics &gfx)
{
	gfx.SelectIndexBuffer(ibo);
	gfx.SelectVertexBuffer(vbo);
	gfx.SelectTexture(0, terrain_tex);
	gfx.DrawArrayTri(0, 0, num_index, num_vertex);

	gfx.SelectIndexBuffer(water_ibo);
	gfx.SelectVertexBuffer(water_vbo);
	gfx.SelectTexture(0, water_tex);
	gfx.DrawArrayTri(0, 0, water_nindex, water_nvertex);

}

bool Terrain::collision_detect(RigidBody &body)
{
	vec3 normal;

	if (loaded == false)
		return false;

	for (int i = 0; i < 8; i++)
	{
		vec3 point = body.aabb[i];
		point += -body.center + body.entity->position;


		float tpos = GetHeight(point, normal);
		height = tpos;
		if (point.y < tpos)
		{
			body.entity->position = body.old_position;
			if (body.old_position.y - body.center.y < tpos)
			{
				body.old_position.y = tpos + body.center.y;
				body.entity->position.y = tpos + body.center.y;
			}

			body.bounce++;
			body.on_ground = true;
			ClipVelocity(body.entity->rigid->velocity, normal);
			body.velocity.y = 0.1f;
			return true;
		}
	}
	return false;
}

float Terrain::GetHeight(const vec3 &position, vec3 &normal)
{
	float height = -FLT_MAX;

	if (loaded == false)
		return height;

	//float terrainWidth = size;
	//float terrainHeight = size;
	//float halfWidth = size * 0.5f;
	//float halfHeight = size * 0.5f;

	int x = 0;
	int y = 0;
	//int width = num_row;
	float xf;
	float yf;

	// normalized 0-1 range for terrain
	xf = (position.x / size + 0.5f);
	yf = (position.z / size + 0.5f);
	x = num_row * xf;
	y = num_row * yf;

	clamp(x, 0, num_row);
	clamp(y, 0, num_row);




	// quad we are over
	int u0 = x;
	int u1 = u0 + 1;
	int v0 = y;
	int v1 = v0 + 1;

	x_index = x;
	y_index = y;

	if (u0 >= 0 && u1 < num_row && v0 >= 0 && v1 < num_col)
	{
		vec3 p00 = vertex_array[(v0 * num_row) + u0].position;    // Top-left
		vec3 p10 = vertex_array[(v0 * num_row) + u1].position;    // Top-right
		vec3 p01 = vertex_array[(v1 * num_row) + u0].position;    // Bottom-left
		vec3 p11 = vertex_array[(v1 * num_row) + u1].position;    // Bottom-right

		float percentU = xf * num_row - u0;
		float percentV = yf * num_row - v0;
		vec3 dU, dV;


		if (percentU > percentV)
		{   // Top triangle
			dU = p10 - p00;
			dV = p11 - p10;
			top = 1;
		}
		else
		{   // Bottom triangle
			dU = p11 - p01;
			dV = p01 - p00;
			top = 0;
		}

		normal = vec3::crossproduct(dV, dU);
		normal = normal.normalize();
		vec3 heightPos = p00 + (dU * percentU) + (dV * percentV);
		height = heightPos.y;
	}

	return height;
}

