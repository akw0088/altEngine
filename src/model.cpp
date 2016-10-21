#include "model.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int Model::quad_index = 0;
int Model::quad_vertex = 0;

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

	model_file = get_file(vbo_file);
	if (model_file == NULL)
	{
		debugf("Unable to load vertex array %s\n", vbo_file);
		return;
	}

	num_vertex = *((int *)model_file);
	model_array = (vertex_t *)(model_file + 4);

	aabb[0] = vec3(2048.0f, 2048.0f, 2048.0f);
	aabb[7] = vec3(-2048.0f, -2048.0f, -2048.0f);
	vec3 sum = vec3();
	for(int i = 0; i < num_vertex; i++)
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

	create_box(gfx, aabb);

	char *index_file = get_file(ibo_file);
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

	model_tex = load_texture(gfx, tga_file);
	normal_tex = load_texture(gfx, normal_file);

	if (entity->rigid)
	{
		entity->rigid->recalc();
	}
}

Model::Model(Entity *entity)
{
	Model::entity = entity;

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

void Model::render(Graphics &gfx)
{
	gfx.SelectIndexBuffer(model_index);
	gfx.SelectVertexBuffer(model_vertex);
	gfx.SelectTexture(0, model_tex);
	gfx.SelectTexture(2, normal_tex);
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

	box_vertex = gfx.CreateVertexBuffer(vert, 8);
	box_index = gfx.CreateIndexBuffer(index, 24);
}


void Model::getForwardVector(vec3 &forward)
{
	forward.x = entity->model->morientation.m[6];
	forward.y = entity->model->morientation.m[7];
	forward.z = entity->model->morientation.m[8];
}
