#include "isocube.h"
#include "stb_image.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


vertex_t cube[36];
vec3 vdata[36];
int vindex[36];
vec2 tex[4];
int cube_idx[36];

void IsoCube::init_cube()
{
	tex[0] = vec2(0.0f, 0.0f);
	tex[1] = vec2(0.0f, 1.0f);
	tex[2] = vec2(1.0f, 0.0f);
	tex[3] = vec2(1.0f, 1.0f);

	memset(&cube, 0, 36 * sizeof(vertex_t));
	cube[0].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[0].texCoord0 = tex[0];
	cube[1].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[1].texCoord0 = tex[1];
	cube[2].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[2].texCoord0 = tex[2];

	cube[3].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[3].texCoord0 = tex[1];
	cube[4].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[4].texCoord0 = tex[3];
	cube[5].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[5].texCoord0 = tex[2];

	cube[6].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[6].texCoord0 = tex[3];
	cube[7].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[7].texCoord0 = tex[1];
	cube[8].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[8].texCoord0 = tex[1];

	cube[9].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[9].texCoord0 = tex[3];
	cube[10].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[10].texCoord0 = tex[3];
	cube[11].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[11].texCoord0 = tex[1];

	cube[12].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[12].texCoord0 = tex[1];
	cube[13].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[13].texCoord0 = tex[2];
	cube[14].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[14].texCoord0 = tex[3];


	cube[15].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[15].texCoord0 = tex[1];
	cube[16].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[16].texCoord0 = tex[0];
	cube[17].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[17].texCoord0 = tex[2];

	cube[18].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[18].texCoord0 = tex[0];
	cube[19].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[19].texCoord0 = tex[2];
	cube[20].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[20].texCoord0 = tex[2];


	cube[21].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[21].texCoord0 = tex[0];
	cube[22].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[22].texCoord0 = tex[2];
	cube[23].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[23].texCoord0 = tex[0];

	cube[24].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[24].texCoord0 = tex[0];
	cube[25].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[25].texCoord0 = tex[0];
	cube[26].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[26].texCoord0 = tex[1];

	cube[27].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[27].texCoord0 = tex[1];
	cube[28].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[28].texCoord0 = tex[0];
	cube[29].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[29].texCoord0 = tex[1];

	cube[30].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[30].texCoord0 = tex[2];
	cube[31].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[31].texCoord0 = tex[3];
	cube[32].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[32].texCoord0 = tex[2];

	cube[33].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[33].texCoord0 = tex[3];
	cube[34].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[34].texCoord0 = tex[3];
	cube[35].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[35].texCoord0 = tex[2];


	for (int i = 0; i < 36; i++)
	{
		cube_idx[i] = i;
		vindex[i] = i;
		vdata[i] = cube[i].position;		
	}
}

IsoCube::IsoCube()
{
	vertex_array = NULL;
	index_array = NULL;
	num_vert = 0;
	num_index = 0;
	loaded = false;

	init_cube();
}

int IsoCube::load(Graphics &gfx, char *texture_str, char *heightmap, int depth, float scale)
{
	int width;
	int height;
	int components;

	int size = 0;
	unsigned char *data = (unsigned char *)get_file(heightmap, &size);
	unsigned char *image = stbi_load_from_memory(data, size, &width, &height, &components, 0);


	// Have repeated vertices it seems
	switch (depth)
	{
	case 0:
		vertex_array = new vertex_t[20 * 3];
		break;
	case 1:
		vertex_array = new vertex_t[80 * 3];
		break;
	case 2:
		vertex_array = new vertex_t[320 * 3];
		break;
	case 3:
		vertex_array = new vertex_t[1280 * 3];
		break;
	case 4:
		vertex_array = new vertex_t[5120 * 3];
		break;
	case 5:
		vertex_array = new vertex_t[20480 * 3];
		break;
	case 6:
		vertex_array = new vertex_t[81920 * 3];
		break;
	case 7:
		vertex_array = new vertex_t[327680 * 3];
		break;
	case 8:
		vertex_array = new vertex_t[1310720 * 3];
		break;
	case 9:
		vertex_array = new vertex_t[5242880 * 3];
		break;
//	case 10:
//		vertex_array = new vertex_t[20971519 * 3];
//		break;
	}

	generate(depth, scale, (unsigned char *)image, width, height);
	vbo = gfx.CreateVertexBuffer(vertex_array, num_vert, false);

	index_array = new unsigned int[num_index];
	for (unsigned int i = 0; i < num_index; i++)
	{
		index_array[i] = i;
	}

	ibo = gfx.CreateIndexBuffer(index_array, num_index);
	iso_tex = load_texture(gfx, texture_str, false, false, 0);
	loaded = true;
	return 0;
}

// Not really drawing, more adding to vertex array, would be immediate mode draw here though
void IsoCube::drawTriangle(const vec3 &v1, const vec3 &v2, const vec3 &v3, float scale, unsigned char *image, int width, int height)
{
	vec2 texcoord1(v1.x / 2.0f + 0.5f, v1.y / 2.0f + 0.5f);
	vec2 texcoord2(v2.x / 2.0f + 0.5f, v2.y / 2.0f + 0.5f);
	vec2 texcoord3(v3.x / 2.0f + 0.5f, v3.y / 2.0f + 0.5f);
	int x, y;
	float heightmap_scale = 0.1f;


	vertex_array[num_vert].color = ~0;
	vertex_array[num_vert].normal = v1;
	vertex_array[num_vert].texCoord0 = texcoord1;
	x = texcoord1.x * width;
	y = texcoord1.y * height;
	vertex_array[num_vert].position = v1 * scale *
		(((image[(y * width + x) * sizeof(int)]) / 255.0f) * heightmap_scale + 1.0f);
	num_vert++;

	vertex_array[num_vert].color = ~0;
	vertex_array[num_vert].normal = v2;
	vertex_array[num_vert].texCoord0 = texcoord2;
	x = texcoord2.x * width;
	y = texcoord2.y * height;
	vertex_array[num_vert].position = v2 * scale *
		(((image[(y * width + x) * sizeof(int)]) / 255.0f) * heightmap_scale + 1.0f);
	num_vert++;

	vertex_array[num_vert].color = ~0;
	vertex_array[num_vert].normal = v3;
	vertex_array[num_vert].texCoord0 = texcoord3;
	x = texcoord3.x * width;
	y = texcoord3.y * height;
	vertex_array[num_vert].position = v3 * scale *
		(((image[(y * width + x) * sizeof(int)]) / 255.0f) * heightmap_scale + 1.0f);
	num_vert++;

	num_index += 3;
}

void IsoCube::subdivide(const vec3 &v1, const vec3 &v2, const vec3 &v3, int depth, float scale, unsigned char *image, int width, int height)
{
	vec3 v12, v23, v31;

	if (depth == 0)
	{
		drawTriangle(v1, v2, v3, scale, image, width, height);
		return;
	}

	v12 = (v1 + v2) / 2.0f;
	v23 = (v2 + v3) / 2.0f;
	v31 = (v3 + v1) / 2.0f;

	v12 = v12.normalize();
	v23 = v23.normalize();
	v31 = v31.normalize();

	subdivide(v1, v12, v31, depth - 1, scale, image, width, height);
	subdivide(v2, v23, v12, depth - 1, scale, image, width, height);
	subdivide(v3, v31, v23, depth - 1, scale, image, width, height);
	subdivide(v12, v23, v31, depth - 1, scale, image, width, height);
}

void IsoCube::generate(int depth, float scale, unsigned char *image, int width, int height)
{
	for (int i = 0; i < 36;)
	{
		subdivide(vdata[vindex[i]],
			vdata[vindex[i + 1]],
			vdata[vindex[i + 2]],
			depth, scale, image, width, height);

		i += 3;
	}
}

void IsoCube::render(Graphics &gfx)
{
	gfx.SelectIndexBuffer(ibo);
	gfx.SelectVertexBuffer(vbo);
	gfx.SelectTexture(0, iso_tex);
	gfx.DrawArrayTri(0, 0, num_index, num_vert);
}


void IsoCube::destroy(Graphics &gfx)
{
	gfx.DeleteTexture(iso_tex);
	gfx.DeleteIndexBuffer(ibo);
	gfx.DeleteVertexBuffer(vbo);

	delete [] index_array;
	delete [] vertex_array;
	num_vert = 0;
	num_index = 0;
}
