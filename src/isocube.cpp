#include "isocube.h"
#include "stb_image.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


vertex_t cube[36];
int vindex[36];
vec2 tex[8];
int cube_idx[36];

void IsoCube::init_cube()
{
	tex[0] = vec2(0.0f, 0.0f);
	tex[1] = vec2(1.0f, 0.0f);
	tex[2] = vec2(0.0f, 1.0f);
	tex[3] = vec2(1.0f, 1.0f);

	tex[4] = vec2(0.0f, 0.0f);
	tex[5] = vec2(-1.0f, 0.0f);
	tex[6] = vec2(0.0f, 1.0f);
	tex[7] = vec2(-1.0f, 1.0f);


	memset(&cube, 0, 36 * sizeof(vertex_t));
	// side face - good
	cube[2].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[2].texCoord0 = tex[0];
	cube[1].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[1].texCoord0 = tex[1];
	cube[0].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[0].texCoord0 = tex[2];

	cube[5].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[5].texCoord0 = tex[1];
	cube[4].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[4].texCoord0 = tex[3];
	cube[3].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[3].texCoord0 = tex[2];

	//side face - good
	cube[8].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[8].texCoord0 = tex[3];
	cube[7].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[7].texCoord0 = tex[0];
	cube[6].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[6].texCoord0 = tex[1];

	cube[11].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[11].texCoord0 = tex[3];
	cube[10].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[10].texCoord0 = tex[2];
	cube[9].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[9].texCoord0 = tex[0];

	//side face - was sideways (negated texcoords)
	cube[14].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[14].texCoord0 = tex[5];
	cube[13].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[13].texCoord0 = tex[6];
	cube[12].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[12].texCoord0 = tex[7];

	cube[17].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[17].texCoord0 = tex[5];
	cube[16].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[16].texCoord0 = tex[4];
	cube[15].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[15].texCoord0 = tex[6];

	//face - was sideways (negated texcoords)
	cube[20].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[20].texCoord0 = tex[4];
	cube[19].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[19].texCoord0 = tex[6];
	cube[18].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[18].texCoord0 = tex[7];

	cube[23].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[23].texCoord0 = tex[4];
	cube[22].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[22].texCoord0 = tex[7];
	cube[21].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[21].texCoord0 = tex[5];

	//bottom face
	cube[26].position = vec3(-0.5f, -0.5f, -0.5f); //1
	cube[26].texCoord0 = tex[0];
	cube[25].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[25].texCoord0 = tex[1];
	cube[24].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[24].texCoord0 = tex[2];

	cube[29].position = vec3(-0.5f, -0.5f, 0.5f);  //2
	cube[29].texCoord0 = tex[2];
	cube[28].position = vec3(0.5f, -0.5f, -0.5f);  //5
	cube[28].texCoord0 = tex[1];
	cube[27].position = vec3(0.5f, -0.5f, 0.5f);   //6
	cube[27].texCoord0 = tex[3];

	//top face
	cube[32].position = vec3(-0.5f, 0.5f, -0.5f);  //3
	cube[32].texCoord0 = tex[4];
	cube[31].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[31].texCoord0 = tex[6];
	cube[30].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[30].texCoord0 = tex[5];

	cube[35].position = vec3(-0.5f, 0.5f, 0.5f);   //4
	cube[35].texCoord0 = tex[6];
	cube[34].position = vec3(0.5f, 0.5f, 0.5f);    //8
	cube[34].texCoord0 = tex[7];
	cube[33].position = vec3(0.5f, 0.5f, -0.5f);   //7
	cube[33].texCoord0 = tex[5];


	for (int i = 0; i < 36; i++)
	{
		cube_idx[i] = i;
		vindex[i] = i;
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
	if (data == NULL)
	{
		printf("Unable to open %s\n", heightmap);
		return -1;
	}

	unsigned char *image = stbi_load_from_memory(data, size, &width, &height, &components, 0);
	if (image == NULL)
	{
		printf("Unable to process %s\n", heightmap);
		return -1;
	}

	switch (depth)
	{
	case 0:
		vertex_array = new vertex_t[12 * 3];
		break;
	case 1:
		vertex_array = new vertex_t[48 * 3];
		break;
	case 2:
		vertex_array = new vertex_t[192 * 3];
		break;
	case 3:
		vertex_array = new vertex_t[768 * 3];
		break;
	case 4:
		vertex_array = new vertex_t[3072 * 3];
		break;
	case 5:
		vertex_array = new vertex_t[12288 * 3];
		break;
	case 6:
		vertex_array = new vertex_t[49152 * 3];
		break;
	case 7:
		vertex_array = new vertex_t[196608 * 3];
		break;
	case 8:
		vertex_array = new vertex_t[786432 * 3];
		break;
	case 9:
		vertex_array = new vertex_t[3145728 * 3];
		break;
	default:
		return -1;
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
void IsoCube::drawTriangle(const vertex_t &v1, const vertex_t &v2, const vertex_t &v3, float scale, unsigned char *image, int width, int height)
{
	unsigned int x, y;
	float heightmap_scale = 0.1f;
	vec2 t1 = v1.texCoord0;
	vec2 t2 = v2.texCoord0;
	vec2 t3 = v3.texCoord0;

	if (t1.x < 0)
	{
		t1.x += 1.0f;
	}
	if (t2.x < 0)
	{
		t2.x += 1.0f;
	}
	if (t3.x < 0)
	{
		t3.x += 1.0f;
	}
	if (t1.y < 0)
	{
		t1.y += 1.0f;
	}
	if (t2.y < 0)
	{
		t2.y += 1.0f;
	}
	if (t3.y < 0)
	{
		t3.y += 1.0f;
	}




	vertex_array[num_vert].color = ~0;
	vertex_array[num_vert].normal = v1.position;
	vertex_array[num_vert].texCoord0 = v1.texCoord0;
	vertex_array[num_vert].texCoord1 = v1.texCoord1;
	x = (unsigned int)(t1.x * (width - 1));
	y = (unsigned int)(t1.y * (height - 1));
	vertex_array[num_vert].position = v1.position * scale *
		(((image[(y * (width - 1) + x) * sizeof(int)]) / 255.0f) * heightmap_scale + 1.0f);
	num_vert++;

	vertex_array[num_vert].color = ~0;
	vertex_array[num_vert].normal = v2.position;
	vertex_array[num_vert].texCoord0 = v2.texCoord0;
	vertex_array[num_vert].texCoord1 = v2.texCoord1;
	x = (unsigned int)(t2.x * (width - 1));
	y = (unsigned int)(t2.y * (height - 1));
	vertex_array[num_vert].position = v2.position * scale *
		(((image[(y * (width - 1) + x) * sizeof(int)]) / 255.0f) * heightmap_scale + 1.0f);
	num_vert++;

	vertex_array[num_vert].color = ~0;
	vertex_array[num_vert].normal = v3.position;
	vertex_array[num_vert].texCoord0 = v3.texCoord0;
	vertex_array[num_vert].texCoord1 = v3.texCoord1;
	x = (unsigned int)(t3.x * (width - 1));
	y = (unsigned int)(t3.y * (height - 1));
	vertex_array[num_vert].position = v3.position * scale *
		(((image[(y * (width - 1) + x) * sizeof(int)]) / 255.0f) * heightmap_scale + 1.0f);
	num_vert++;

	num_index += 3;
}

void IsoCube::subdivide(const vertex_t &v1, const vertex_t &v2, const vertex_t &v3, int depth, float scale, unsigned char *image, int width, int height)
{
	vertex_t v12, v23, v31;

	if (depth == 0)
	{
		drawTriangle(v1, v2, v3, scale, image, width, height);
		return;
	}

	v12.position = (v1.position + v2.position) / 2.0f;
	v23.position = (v2.position + v3.position) / 2.0f;
	v31.position = (v3.position + v1.position) / 2.0f;

	v12.texCoord0 = (v1.texCoord0 + v2.texCoord0) / 2.0f;
	v23.texCoord0 = (v2.texCoord0 + v3.texCoord0) / 2.0f;
	v31.texCoord0 = (v3.texCoord0 + v1.texCoord0) / 2.0f;

	v12.texCoord1 = (v1.texCoord1 + v2.texCoord1) / 2.0f;
	v23.texCoord1 = (v2.texCoord1 + v3.texCoord1) / 2.0f;
	v31.texCoord1 = (v3.texCoord1 + v1.texCoord1) / 2.0f;


	v12.position = v12.position.normalize();
	v23.position = v23.position.normalize();
	v31.position = v31.position.normalize();

	subdivide(v1, v12, v31, depth - 1, scale, image, width, height);
	subdivide(v2, v23, v12, depth - 1, scale, image, width, height);
	subdivide(v3, v31, v23, depth - 1, scale, image, width, height);
	subdivide(v12, v23, v31, depth - 1, scale, image, width, height);
}

void IsoCube::generate(int depth, float scale, unsigned char *image, int width, int height)
{
	for (int i = 0; i < 36;)
	{
		subdivide(cube[vindex[i]],
			cube[vindex[i + 1]],
			cube[vindex[i + 2]],
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
