#include "model.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <float.h> // for FLT_MAX

int Model::quad_index = 0;
int Model::quad_vertex = 0;

int Model::cube_index = 0;
int Model::cube_vertex = 0;

int Model::skybox_index = 0;
int Model::skybox_vertex = 0;



void Model::load(Graphics &gfx, char *file)
{
	char vbo_file[LINE_SIZE];
	char ibo_file[LINE_SIZE];
	char tga_file[LINE_SIZE];
	char normal_file[LINE_SIZE];

	snprintf(vbo_file, LINE_SIZE, "%s.vbo", file);
	snprintf(ibo_file, LINE_SIZE, "%s.ibo", file);
	snprintf(tga_file, LINE_SIZE, "%s.tga", file);
	snprintf(normal_file, LINE_SIZE, "%s_normal.tga", file);

	model_file = get_file(vbo_file, NULL);
	if (model_file == NULL)
	{
		debugf("Unable to load vertex array %s\n", vbo_file);
		return;
	}

	num_vertex = *((int *)model_file);
	model_array = (vertex_t *)(model_file + 4);

	make_aabb();
	make_skybox(gfx);
	create_box(gfx, aabb);

	char *index_file = get_file(ibo_file, NULL);
	if (index_file == NULL)
	{
		debugf("Unable to load index array %s\n", index_file);
		delete [] model_file;
		return;
	}

	num_index = *((int *)index_file);

	model_vertex = gfx.CreateVertexBuffer(model_array, num_vertex);
	model_index = gfx.CreateIndexBuffer(index_file + 4, num_index);
	delete [] index_file;
	index_file = NULL;
	delete [] model_file;
	model_file = NULL;

	model_tex = load_texture(gfx, tga_file, false);
	normal_tex = load_texture(gfx, normal_file, false);

	if (entity->rigid)
	{
		entity->rigid->recalc();
	}
}

Model::Model(Entity *entity)
{
	Model::entity = entity;


	num_vertex = 0;
	num_index = 0;
	normal_tex = 0;
	model_vertex = 0;
	model_index = 0;
	model_tex = 0;
	box_vertex = 0;
	box_index = 0;
	model_file = NULL;
	model_array = NULL;
	center = vec3();

	morientation.m[0] = 1.0f;
	morientation.m[1] = 0.0f;
	morientation.m[2] = 0.0f;

	morientation.m[3] = 0.0f;
	morientation.m[4] = 1.0f;
	morientation.m[5] = 0.0f;

	morientation.m[6] = 0.0f;
	morientation.m[7] = 0.0f;
	morientation.m[8] = 1.0f;
}

// copies visual data from model
void Model::clone(Model &model)
{
	for(int i = 0; i < 8; i++)
	{
		aabb[i] = model.aabb[i];
	}
	center = model.center;
	box_index = model.box_index;
	box_vertex = model.box_vertex;
	model_array = model.model_array;
	model_index = model.model_index;
	model_tex = model.model_tex;
	model_vertex = model.model_vertex;
	num_index = model.num_index;
	num_vertex = model.num_vertex;

	//hacks
	if (entity->rigid)
	{
		entity->rigid->gravity = model.entity->rigid->gravity;
		entity->rigid->angular_velocity = model.entity->rigid->angular_velocity;
	}
}

void Model::make_aabb()
{
	aabb[0] = vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	aabb[7] = vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	vec3 sum = vec3();
	for (int i = 0; i < num_vertex; i++)
	{
		sum += model_array[i].position;

		if (model_array[i].position.x < aabb[0].x)
			aabb[0].x = model_array[i].position.x;
		else if (model_array[i].position.y < aabb[0].y)
			aabb[0].y = model_array[i].position.y;
		else if (model_array[i].position.z < aabb[0].z)
			aabb[0].z = model_array[i].position.z;

		if (model_array[i].position.x > aabb[7].x)
			aabb[7].x = model_array[i].position.x;
		else if (model_array[i].position.y > aabb[7].y)
			aabb[7].y = model_array[i].position.y;
		else if (model_array[i].position.z > aabb[7].z)
			aabb[7].z = model_array[i].position.z;
	}
	center = sum / (float)num_vertex;
	// binary order
	aabb[1] = vec3(aabb[0].x, aabb[0].y, aabb[7].z);
	aabb[2] = vec3(aabb[0].x, aabb[7].y, aabb[0].z);
	aabb[3] = vec3(aabb[0].x, aabb[7].y, aabb[7].z);
	aabb[4] = vec3(aabb[7].x, aabb[0].y, aabb[0].z);
	aabb[5] = vec3(aabb[7].x, aabb[0].y, aabb[7].z);
	aabb[6] = vec3(aabb[7].x, aabb[7].y, aabb[0].z);
}


void Model::make_skybox(Graphics &gfx)
{
	vec3 verts[36] =
	{
		// Front face
		vec3(-200.0f, 200.0f, 200.0f), //3
		vec3(200.0f, -200.0f, 200.0f), //2
		vec3(200.0f, 200.0f, 200.0f),  //1

		vec3(-200.0f, -200.0f, 200.0f), //4
		vec3(200.0f, -200.0f, 200.0f),  //2
		vec3(-200.0f, 200.0f, 200.0f), //3


		// Back face
		vec3(200.0f, 200.0f, -200.0f),		//3
		vec3(-200.0f, -200.0f, -200.0f),	//2
		vec3(-200.0f, 200.0f, -200.0f),		//1

		vec3(200.0f, -200.0f, -200.0f),		//4
		vec3(-200.0f, -200.0f, -200.0f),	//2
		vec3(200.0f, 200.0f, -200.0f),		//3

		// Left face
		vec3(-200.0f, 200.0f, -200.0f),		//3
		vec3(-200.0f, -200.0f, 200.0f),		//2
		vec3(-200.0f, 200.0f, 200.0f),		//1

		vec3(-200.0f, -200.0f, -200.0f),	//4
		vec3(-200.0f, -200.0f, 200.0f),		//2
		vec3(-200.0f, 200.0f, -200.0f),		//3

		// Right face
		vec3(200.0f, 200.0f, 200.0f),		//3
		vec3(200.0f, -200.0f, -200.0f),		//2
		vec3(200.0f, 200.0f, -200.0f),		//1

		vec3(200.0f, -200.0f, 200.0f),		//4
		vec3(200.0f, -200.0f, -200.0f),		//2
		vec3(200.0f, 200.0f, 200.0f),		//3

		// Top face
		vec3(-200.0f, 200.0f, 200.0f),		//3
		vec3(200.0f, 200.0f, -200.0f),		//2
		vec3(-200.0f, 200.0f, -200.0f),		//1

		vec3(200.0f, 200.0f, 200.0f),		//4
		vec3(200.0f, 200.0f, -200.0f),		//2
		vec3(-200.0f, 200.0f, 200.0f),		//3

		// Bottom face
		vec3(200.0f, -200.0f, 200.0f),		//3
		vec3(-200.0f, -200.0f, -200.0f),	//2
		vec3(200.0f, -200.0f, -200.0f),		//1

		vec3(-200.0f, -200.0f, 200.0f),		//4
		vec3(-200.0f, -200.0f, -200.0f),	//2
		vec3(200.0f, -200.0f, 200.0f)		//3
	};

	vec2 texcoords[6] =
	{
		vec2(0.0f, 1.0f),			//1
		vec2(0.0f, 0.0f),			//2
		vec2(1.0f, 1.0f),			//3

		vec2(0.0f, 0.0f),			//2
		vec2(1.0f, 0.0f),			//4
		vec2(1.0f, 1.0f),			//3
	};

	vec3 normals[6] =
	{
		vec3(0.0f, 0.0f, -1.0f),
		vec3(0.0f, 0.0f, 1.0f),
		vec3(1.0f, 0.0f, 0.0f),
		vec3(-1.0f, 0.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f)
	};


	vertex_t skybox[36];
	int index[36];
	for (int i = 0; i < 36; i++)
	{
		skybox[i].position = verts[i] * 1000000.0f;
		skybox[i].texCoord0 = texcoords[i % 6];
		skybox[i].normal = normals[i / 4];
		index[i] = i;
	}

	skybox_vertex = gfx.CreateVertexBuffer(skybox, 36);
	skybox_index = gfx.CreateIndexBuffer(index, 36);
}

void Model::render(Graphics &gfx)
{
	gfx.SelectIndexBuffer(model_index);
	gfx.SelectVertexBuffer(model_vertex);
	for (int i = 0; i < MAX_TEXTURES; i++)
	{
		gfx.SelectTexture(i, 0);
	}
	gfx.SelectTexture(0, model_tex);
//	gfx.SelectTexture(2, normal_tex);
	gfx.DrawArrayTri(0, 0, num_index, num_vertex);
//	gfx.DeselectTexture(2);
//	gfx.DeselectTexture(0);
}

void Model::render_box(Graphics &gfx)
{
	gfx.SelectIndexBuffer(box_index);
	gfx.SelectVertexBuffer(box_vertex);
	gfx.SelectTexture(0, model_tex);
	gfx.DrawArrayTri(0, 0, 24, 24);
//	gfx.DeselectTexture(0);
}

float *Model::get_matrix(float *matrix)
{
	matrix[0] = morientation.m[0];
	matrix[1] = morientation.m[1];
	matrix[2] = morientation.m[2];
	matrix[3] = 0.0f;

	matrix[4] = morientation.m[3];
	matrix[5] = morientation.m[4];
	matrix[6] = morientation.m[5];
	matrix[7] = 0.0f;

	matrix[8]  = morientation.m[6];
	matrix[9]  = morientation.m[7];
	matrix[10] = morientation.m[8];
	matrix[11] = 0.0f;

	/* matrix rotates around center, but position is arbitrary point
	from which verts are defined */
	vec3 offset = morientation * center;
	matrix[12] = entity->position.x - offset.x;
	matrix[13] = entity->position.y - offset.y;
	matrix[14] = entity->position.z - offset.z;
	matrix[15] = 1.0f;
	return matrix;
}

void Model::CreateObjects(Graphics &gfx)
{
	int qindex[] = {0,1,3,0,3,2};
	vertex_t quad[4];

	memset(&quad, 0, 4 * sizeof(vertex_t));
	quad[0].position = vec3(-1.0f, -1.0f, 0.0f);
	quad[0].texCoord0 = vec2(0.0f, 0.0f);
	quad[0].color = ~0;
	quad[1].position = vec3(-1.0f, 1.0f, 0.0f);
	quad[1].texCoord0 = vec2(0.0f, 1.0f);
	quad[1].color = ~0;
	quad[2].position = vec3(1.0f, -1.0f, 0.0f);
	quad[2].texCoord0 = vec2(1.0f, 0.0f);
	quad[2].color = ~0;
	quad[3].position = vec3(1.0f, 1.0f, 0.0f);
	quad[3].texCoord0 = vec2(1.0f, 1.0f);
	quad[3].color = ~0;

	quad_index = gfx.CreateIndexBuffer(qindex, 6);
	quad_vertex = gfx.CreateVertexBuffer(quad, 4);

	/*
	int cube_idx[36];
	vertex_t cube[36];
	vec2 tex[4];

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
		cube[i].color = ~0;
		cube[i].position *= 500.0f;
	}


	Model::cube_index = gfx.CreateIndexBuffer(cube_idx, 36);
	Model::cube_vertex = gfx.CreateVertexBuffer(cube, 36);
	*/
}

Model::~Model()
{
	if (model_file != NULL)
		delete [] model_file;
	model_file = NULL;
}


void Model::calc_frustum(matrix4 &clip)
{
//	matrix4	clip;

//	clip = transformation * projection;

	// This will extract the RIGHT side of the frustum
	frustum[0].x = clip.m[ 3] - clip.m[ 0];
	frustum[0].y = clip.m[ 7] - clip.m[ 4];
	frustum[0].z = clip.m[11] - clip.m[ 8];
	frustum[0].w = clip.m[15] - clip.m[12];
	frustum[0].normalize();

	// This will extract the LEFT side of the frustum
	frustum[1].x = clip.m[ 3] + clip.m[ 0];
	frustum[1].y = clip.m[ 7] + clip.m[ 4];
	frustum[1].z = clip.m[11] + clip.m[ 8];
	frustum[1].w = clip.m[15] + clip.m[12];
	frustum[1].normalize();

	// This will extract the BOTTOM side of the frustum
	frustum[2].x = clip.m[ 3] + clip.m[ 1];
	frustum[2].y = clip.m[ 7] + clip.m[ 5];
	frustum[2].z = clip.m[11] + clip.m[ 9];
	frustum[2].w = clip.m[15] + clip.m[13];
	frustum[2].normalize();

	// This will extract the TOP side of the frustum
	frustum[3].x = clip.m[ 3] - clip.m[ 1];
	frustum[3].y = clip.m[ 7] - clip.m[ 5];
	frustum[3].z = clip.m[11] - clip.m[ 9];
	frustum[3].w = clip.m[15] - clip.m[13];
	frustum[3].normalize();

	// This will extract the BACK side of the frustum
	frustum[4].x = clip.m[ 3] - clip.m[ 2];
	frustum[4].y = clip.m[ 7] - clip.m[ 6];
	frustum[4].z = clip.m[11] - clip.m[10];
	frustum[4].w = clip.m[15] - clip.m[14];
	frustum[4].normalize();

	// This will extract the FRONT side of the frustum
	frustum[5].x = clip.m[ 3] + clip.m[ 2];
	frustum[5].y = clip.m[ 7] + clip.m[ 6];
	frustum[5].z = clip.m[11] + clip.m[10];
	frustum[5].w = clip.m[15] + clip.m[14];
	frustum[5].normalize();
}

/*
bool Model::in_frustum(Global &global, vec3 &position, matrix4 &transformation, matrix4 &projection)
{
	((Plane)left_plane).draw_plane(global, transformation, projection);
	((Plane)right_plane).draw_plane(global,  transformation, projection);

	((Plane)top_plane).draw_plane(global,  transformation, projection);
	((Plane)bottom_plane).draw_plane(global,  transformation, projection);

	((Plane)near_plane).draw_plane(global,  transformation, projection);
	((Plane)far_plane).draw_plane(global,  transformation, projection);

	float result = (vec3)left_plane * position - left_plane.w;

	if (result > 0)
	{
		result = (vec3)right_plane * position - right_plane.w;
		if (result > 0)
		{
			result = (vec3)top_plane * position - top_plane.w;
			if (result > 0)
			{
				result = (vec3)bottom_plane * position - bottom_plane.w;
				if (result > 0)
				{
					result = (vec3)near_plane * position - near_plane.w;
					if (result > 0)
					{
						result = (vec3)far_plane * position - far_plane.w;
						if (result > 0)
						{
							return true;
						}
					}
				}
			}
		}
	}

	// skipping farplane due to infinite projection
//	if ((vec3)left_plane * position - left_plane.w > 0 && 
//	(vec3)right_plane * position - right_plane.w > 0 &&
//	(vec3)top_plane * position - top_plane.w > 0 &&
//	(vec3)bottom_plane * position - bottom_plane.w > 0 &&
//	(vec3)near_plane * position - near_plane.w > 0)
//	{
//		return true;
//	}
//	else
//	{
//		return false;
//	}
	return false;
}
*/

void Model::create_box(Graphics &gfx, vec3 *box)
{
	vertex_t	vert[8];

	memset(&vert, 0, sizeof(vert));
	for(int i = 0; i < 8; i++)
		vert[i].position = box[i];
	int	index[24] = {	0, 1,
						0, 2,
						1, 3,
						2, 3,
						4, 5,
						4, 6,
						5, 7,
						6, 7,
						0, 4,
						1, 5,
						2, 6,
						3, 7};

//	box_vertex = gfx.CreateVertexBuffer(vert, 8);
//	box_index = gfx.CreateIndexBuffer(index, 24);
}


void Model::getForwardVector(vec3 &forward)
{
	forward.x = entity->model->morientation.m[6];
	forward.y = entity->model->morientation.m[7];
	forward.z = entity->model->morientation.m[8];
}

void Model::get_frame(Frame &frame)
{
	frame.up.x = morientation.m[3];
	frame.up.y = morientation.m[4];
	frame.up.z = morientation.m[5];
	frame.forward.x = -morientation.m[6];
	frame.forward.y = -morientation.m[7];
	frame.forward.z = -morientation.m[8];
	frame.pos = entity->position;
}
