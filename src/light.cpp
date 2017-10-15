#include "light.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Light::Light(Entity *entity, Graphics &gfx, int num, float scale)
{
	Light::entity = entity;
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

	num_shadowvol = 0;
}

void Light::generate_cubemaps(Graphics &gfx)
{
	shadow_projection.perspective(90.0, 1.0, 1.0, 2001.0, false);
#ifdef OPENGL

	if (light_num != 999)
	{
		for (int i = 0; i < NUM_CUBE_FACE; i++)
		{
			gfx.setupFramebuffer((int)(SHADOWMAP_DEFAULT_RES * lightmap_scale), (int)(SHADOWMAP_DEFAULT_RES * lightmap_scale), fbo_shadowmaps[i], quad_tex[i], depth_tex[i], 0);
		}
	}
#endif
}

void Light::destroy(Graphics &gfx)
{
	for (int i = 0; i < NUM_CUBE_FACE; i++)
	{
		gfx.DeleteFrameBuffer(fbo_shadowmaps[i]);
		gfx.DeleteTexture(quad_tex[i]);
		gfx.DeleteTexture(depth_tex[i]);
	}


	for (int i = 0; i < MAX_SHADOWVOL; i++)
	{
		shadow[i].destroy(gfx);
	}
}

void Light::render_shadow_volume(Graphics &gfx, int index)
{
	shadow[index].render(gfx);
}

void Light::generate_map_volumes(Bsp &map)
{
//	map.find_edges(entity->position, edge_list);
}


void Light::generate_ent_volumes(Graphics &gfx, vector<Entity *> &entity_list)
{
	int j = 0;
	num_shadowvol = 0;
	for (unsigned int i = 100; i < entity_list.size(); i++)
	{
		if (entity_list[i]->model == NULL)
			continue;

		if (entity_list[i]->visible == false)
			continue;

		vec3 dist = entity_list[i]->position - entity->position;

		if (dist.magnitude() > SHADOWVOL_MAX_DIST)
		{
			continue;
		}

		if (entity_list[i]->ent_type <= ENT_VISIBLE_START && entity_list[i]->ent_type >= ENT_VISIBLE_END)
			continue;

		if (entity_list[i]->ent_type != ENT_ITEM_ARMOR_COMBAT)
			continue;


		vec3 position = entity->position;// -entity_list[i]->position;
		matrix4 matrix;


		entity_list[i]->model->get_matrix(matrix.m);

		position = entity_list[i]->model->morientation.transpose() * position;

		// Probably need a test to see if either light or model moved, as static volumes dont need to be regenerated
		shadow[num_shadowvol].CreateVolume(gfx, entity_list[i]->model->model_vertex_array,
												entity_list[i]->model->model_index_array,
												entity_list[i]->model->num_index / 3,
												position);

		// so shadow rotates with object
		entity->model->morientation = entity_list[i]->model->morientation;
		entity->light->shadow[num_shadowvol].position = entity_list[i]->position - entity_list[i]->model->center;
		num_shadowvol++;

		if (num_shadowvol >= MAX_SHADOWVOL)
			break;
	}
}



