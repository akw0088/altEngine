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
}

int Terrain::load(Graphics &gfx, char *heightmap, char *texture_str, int anisotropic)
{
//	CreateMesh(heightmap, 100.0f, 100.0f, vertex_array, index_array, num_vertex, num_index);
	CreateSphere(32, 100.0f, vertex_array, index_array, num_vertex, num_index);
	vbo = gfx.CreateVertexBuffer(vertex_array, num_vertex, false);
	ibo = gfx.CreateIndexBuffer(index_array, num_index);
	terrain_tex = load_texture(gfx, texture_str, false, false, anisotropic);
	return 0;
}


typedef enum
{
	PLANE,
	SPHERE
} terrain_type_t;



int Terrain::CreateMesh(char *heightmap, float scale_width, float scale_height, vertex_t *&vertex, unsigned int *&index, unsigned int &num_vertex, unsigned int &num_index)
{
	int width;
	int height;
	int components;
	unsigned int i;
	terrain_type_t type = PLANE;

	int size = 0;
	unsigned char *data = (unsigned char *)get_file(heightmap, &size);
	unsigned char *image = stbi_load_from_memory(data, size, &width, &height, &components, 0);

	num_vertex = width * height;
	vertex = new vertex_t[width * height];

	num_index = width * height * 3 * 2;
	index = new unsigned int[num_index];


	float half_length = scale_width * (width >> 1);

	if (type == PLANE)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int index = y * width + x;
				vertex[index].position.x = scale_width * x - half_length;
				vertex[index].position.y = scale_height * image[index * sizeof(int)] - 50000.0f;
				vertex[index].position.z = scale_width * y - half_length;

				vertex[index].texCoord0.x = (float)x / (width - 1);
				vertex[index].texCoord0.y = (float)y / (height - 1);
				vertex[index].texCoord1.x = (float)x / (width - 1);
				vertex[index].texCoord1.y = (float)y / (height - 1);
				vertex[index].color = ~0;
			}
		}
	}
	else if (type == SPHERE)
	{
		// How to make spheres:
		// 1. Typical UV map with texture distortion at north / south poles (yuck)
		// 2. subdivide a box until you are happy, normalize all verticies to create sphere (interesting)
		// 3. geo sphere (triangular approximation of sphere)
		//  ???
	}


	// pulled from tessellate bezier curve -- glad I dont have to write this twice ;)
	int num_row = width;
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
}

bool Terrain::collision_detect(RigidBody &body)
{
	terrain_t terrain;
	static vec3 old_normal1;
	static vec3 old_normal2;
	static float old_d1;
	static float old_d2;
	static int old_x = 0;
	static int old_y = 0;
	vec3 a;
	vec3 b;
	vec3 c;
	vec3 d;
	vec3 e;
	vec3 f;


	vec3 normal1;
	vec3 normal2;
	float d1;
	float d2;

	if (body.entity->player == NULL)
		return false;

	terrain.num_col = 102;
	terrain.num_row = 102;
	terrain.offset = vec3(0.0f, -5000.0f, 0.0f);
	terrain.size = 50000.0f;
	terrain.trilength = terrain.size / terrain.num_row;

	if (body.entity->position.x > terrain.size || body.entity->position.x < -terrain.size)
		return false;
	if (body.entity->position.z > terrain.size || body.entity->position.z < -terrain.size)
		return false;



	int x = 0;//body.entity->position.x / terrain.num_row + terrain.num_row / 2;
	int y = 0;// body.entity->position.z / terrain.num_col + terrain.num_col / 2;
	int width = 102;
	x = 102 * (-body.entity->position.x / (2.0f * terrain.size) + 0.5f);
	y = 102 * (-body.entity->position.z / (2.0f * terrain.size) + 0.5f);
	if (x < 0)
	{
		x = 0;
		printf("Error: Clamped min x\n");
	}
	if (x > 102)
	{
		x = 102;
		printf("Error: Clamped max x\n");
	}
	if (y < 0)
	{
		y = 0;
		printf("Error: Clamped min y\n");
	}
	if (y > 102)
	{
		y = 102;
		printf("Error: Clamped max y\n");
	}

	if (x != old_x && y != old_y)
	{
//		a = ent->model->model_vertex_array[y * width + x].position;
//		b = ent->model->model_vertex_array[y * width + x + 1].position;
//		c = ent->model->model_vertex_array[(y + 1) * width + x + 1].position;

		vec3 ab = a - b;
		vec3 ac = a - c;
		normal1 = vec3::crossproduct(ab, ac);
		normal1 = normal1.normalize();
		d1 = -(a * normal1);

//		d = ent->model->model_vertex_array[(y + 1) * width + x].position;
//		e = ent->model->model_vertex_array[(y + 1) * width + x + 1].position;
//		f = ent->model->model_vertex_array[(y + 1) * width + x].position;

		vec3 de = d - e;
		vec3 df = d - f;
		normal2 = vec3::crossproduct(de, df);
		normal2 = normal2.normalize();
		d2 = -(d * normal2);
	}
	else
	{
		normal1 = old_normal1;
		normal2 = old_normal2;
		d1 = old_d1;
		d2 = old_d2;
	}

	// ax + by + cz + d = 0

	vec3 point = body.entity->position + terrain.offset;

	if (point * normal1 + d1 > 0)
	{
		/*
		if (tick_num % 125 == 0)
		{
			vec3 dist = a - point;
			printf("Distance from plane one is %f\n", point * normal1 + d1);
			printf("Distance to point A is %f %f %f\n", dist.x, dist.y, dist.z);
		}
		*/


		body.entity->position = body.old_position;
		body.morientation = body.old_orientation;
		body.on_ground = true;
//		ClipVelocity(body.entity->rigid->velocity, normal1);
		return true;
	}
	else if (point * normal2 + d2 > 0)
	{
		/*
		if (tick_num % 125 == 0)
		{
			vec3 dist = d - point;

			printf("Distance from plane two is %f\n", point * normal2 + d2);
			printf("Distance to point D is %f %f %f\n", dist.x, dist.y, dist.z);
		}
		*/

		body.entity->position = body.old_position;
		body.morientation = body.old_orientation;
		body.on_ground = true;
//		ClipVelocity(body.entity->rigid->velocity, normal2);
		return true;
	}

	body.on_ground = true;

	return false;
}