//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#include "ent_light.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

EntLight::EntLight(Entity *entity, Graphics &gfx, int num, float scale)
{
	EntLight::entity = entity;
	color = vec3(1.0f, 1.0f, 1.0f);
	intensity = 300.0f;
	attenuation = 1.0f; // default
	active = false;
	light_num = num;
	timer_flag = false;
	timer = 0;
	lightmap_scale = scale;

	memset(quad_tex, 0, sizeof(unsigned int) * NUM_CUBE_FACE);
	memset(depth_tex, 0, sizeof(unsigned int) * NUM_CUBE_FACE);
#ifdef SHADOWVOL
	num_shadowvol = 0;
#endif
}

void EntLight::generate_cubemaps(Graphics &gfx)
{
	shadow_projection.perspective(90.0, 1.0, 1.0, 2001.0, false);
#ifdef OPENGL
	unsigned int normal_depth;

	if (light_num != 999)
	{
		for (int i = 0; i < NUM_CUBE_FACE; i++)
		{
			gfx.CreateFramebuffer((int)(SHADOWMAP_DEFAULT_RES * lightmap_scale), (int)(SHADOWMAP_DEFAULT_RES * lightmap_scale), fbo_shadowmaps[i], quad_tex[i], depth_tex[i], normal_depth, 0, false);
		}
	}
#endif
}

void EntLight::destroy(Graphics &gfx)
{
	for (int i = 0; i < NUM_CUBE_FACE; i++)
	{
		gfx.DeleteFrameBuffer(fbo_shadowmaps[i], quad_tex[i], depth_tex[i]);
	}

#ifdef SHADOWVOL
	for (int i = 0; i < MAX_SHADOWVOL; i++)
	{
		shadow[i].destroy(gfx);
	}
#endif
}

void EntLight::render_shadow_volume(Graphics &gfx, int index)
{
#ifdef SHADOWVOL
	shadow[index].render(gfx);
#endif
}

void EntLight::render_map_shadowvol(Graphics &gfx)
{
	map_shadow.render(gfx);
}

void EntLight::generate_map_volumes(Graphics &gfx, Bsp &map, int current_light)
{
	static vertex_t shadow_vertex[50 * 4096];
	static unsigned int		 shadow_index[50 * 4096];
	int		num_index = 0;

	memset(&shadow_vertex[0], 0, sizeof(shadow_vertex));
	memset(&shadow_index[0], 0, sizeof(shadow_index));
	map.CreateShadowVolumes(gfx, entity->position, current_light, &shadow_vertex[0], &shadow_index[0], num_index);

//	map_shadow.ibo = gfx.CreateIndexBuffer(shadow_index, num_index);
//	map_shadow.vbo = gfx.CreateVertexBuffer(shadow_vertex, num_index);
	if (num_index > 0)
	{
		map_shadow.CreateVolume(gfx, &shadow_vertex[0], &shadow_index[0], num_index / 3, entity->position);
	}
}


void EntLight::generate_ent_volumes(Graphics &gfx, vector<Entity *> &entity_list)
{
#ifdef SHADOWVOL
	//int j = 0;
	num_shadowvol = 0;
	for (unsigned int i = 100; i < entity_list.size(); i++)
	{
		if (entity_list[i]->model == NULL)
			continue;

		if (entity_list[i]->flags.visible == false)
			continue;

		vec3 dist = entity_list[i]->position - entity->position;

		if (dist.magnitude() > SHADOWVOL_MAX_DIST)
		{
			continue;
		}

		if (entity_list[i]->ent_type <= ENT_VISIBLE_START || entity_list[i]->ent_type >= ENT_VISIBLE_END)
			continue;

//		if (entity_list[i]->ent_type != ENT_ITEM_ARMOR_COMBAT)
			//continue;



		matrix4 matrix;
		EntModel *model = entity_list[i]->model;
		model->get_matrix(matrix.m);
		vec3 position = entity_list[i]->model->morientation.transpose() * entity->position;

		// Probably need a test to see if either light or model moved, as static volumes dont need to be regenerated
		shadow[num_shadowvol].CreateVolume(gfx, model->model_vertex_array,
												model->model_index_array,
												model->num_index,
												position);

		// so shadow rotates with object
		entity->light->shadow[num_shadowvol].morientation = model->morientation;
		entity->light->shadow[num_shadowvol].position = entity_list[i]->position - model->center;
		num_shadowvol++;

		if (num_shadowvol >= MAX_SHADOWVOL)
			break;
	}
#endif
}



