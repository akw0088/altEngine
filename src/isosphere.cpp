#include "isosphere.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

float X = 0.525731112119133606f;
float Z = 0.850650808352039932f;
float N = 0.0f;

const vec3 vdata[12] = {
	{ -X,N,Z },{ X,N,Z },{ -X,N,-Z },{ X,N,-Z },
	{ N,Z,X },{ N,Z,-X },{ N,-Z,X },{ N,-Z,-X },
	{ Z,X,N },{ -Z,X, N },{ Z,-X,N },{ -Z,-X, N }
};

const int index[60] = {
	0,4,1,	0,9,4,		9,5,4,		4,5,8,		4,8,1,
	8,10,1,	8,3,10,		5,3,8,		5,2,3,		2,7,3,
	7,10,3,	7,6,10,		7,11,6,		11,0,6,		0,1,6,
	6,1,10,	9,0,11,		9,11,2,		9,2,5,		7,2,11
};

IsoSphere::IsoSphere()
{
	vertex_array = NULL;
	index_array = NULL;
	num_vert = 0;
	num_index = 0;
	loaded = false;
}

int IsoSphere::load(Graphics &gfx, char *texture_str, int depth, float scale)
{
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


	generate(depth, scale);
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
void IsoSphere::drawTriangle(const vec3 &v1, const vec3 &v2, const vec3 &v3, float scale)
{
	vertex_array[num_vert].position = v1 * scale;
	vertex_array[num_vert].color = ~0;
	vertex_array[num_vert].normal = v1;
	vertex_array[num_vert].texCoord0 = vec2(v1.x / 2.0f + 0.5f, v1.y / 2.0f + 0.5f);
	num_vert++;

	vertex_array[num_vert].position = v2 * scale;
	vertex_array[num_vert].color = ~0;
	vertex_array[num_vert].normal = v1;
	vertex_array[num_vert].texCoord0 = vec2(v2.x / 2.0f + 0.5f, v2.y / 2.0f + 0.5f);
	num_vert++;

	vertex_array[num_vert].position = v3 * scale;
	vertex_array[num_vert].color = ~0;
	vertex_array[num_vert].normal = v1;
	vertex_array[num_vert].texCoord0 = vec2(v3.x / 2.0f + 0.5f, v3.y / 2.0f + 0.5f);
	num_vert++;

	num_index += 3;
}

void IsoSphere::subdivide(const vec3 &v1, const vec3 &v2, const vec3 &v3, int depth, float scale)
{
	vec3 v12, v23, v31;
	if (depth == 0)
	{
		drawTriangle(v1, v2, v3, scale);
		return;
	}

	v12 = (v1 + v2) / 2.0f;
	v23 = (v2 + v3) / 2.0f;
	v31 = (v3 + v1) / 2.0f;

	v12 = v12.normalize();
	v23 = v23.normalize();
	v31 = v31.normalize();

	subdivide(v1, v12, v31, depth - 1, scale);
	subdivide(v2, v23, v12, depth - 1, scale);
	subdivide(v3, v31, v23, depth - 1, scale);
	subdivide(v12, v23, v31, depth - 1, scale);
}

void IsoSphere::generate(int depth, float scale)
{
	for (int i = 0; i < 60;)
	{
		subdivide(vdata[index[i]],
			vdata[index[i + 1]],
			vdata[index[i + 2]],
			depth, scale);

		i += 3;
	}
}

void IsoSphere::render(Graphics &gfx)
{
	gfx.SelectIndexBuffer(ibo);
	gfx.SelectVertexBuffer(vbo);
	gfx.SelectTexture(0, iso_tex);
	gfx.DrawArrayTri(0, 0, num_index, num_vert);
}


void IsoSphere::destroy(Graphics &gfx)
{
	gfx.DeleteTexture(iso_tex);
	gfx.DeleteIndexBuffer(ibo);
	gfx.DeleteVertexBuffer(vbo);

	delete [] index_array;
	delete [] vertex_array;
	num_vert = 0;
	num_index = 0;
}