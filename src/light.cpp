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

	memset(quad_tex, 0, sizeof(unsigned int) * 6);
	memset(depth_tex, 0, sizeof(unsigned int) * 6);
}

void Light::generate_cubemaps(Graphics &gfx)
{
	shadow_projection.perspective(90.0, 1.0, 1.0, 2001.0, false);
#ifdef OPENGL

	if (light_num != 999)
	{
		for (int i = 0; i < 6; i++)
		{
			gfx.setupFramebuffer((int)(1024 * lightmap_scale), (int)(1024 * lightmap_scale), fbo_shadowmaps[i], quad_tex[i], depth_tex[i], 0);
		}
	}
#endif
}

void Light::destroy(Graphics &gfx)
{
	for (int i = 0; i < 6; i++)
	{
		gfx.DeleteFrameBuffer(fbo_shadowmaps[i]);
		gfx.DeleteTexture(quad_tex[i]);
		gfx.DeleteTexture(depth_tex[i]);
	}
}

void Light::render_shadow_volumes(Graphics &gfx, int current_light)
{
	if (entity->visible)
	{
		shadow.render(gfx);
	}
//		extend(edge_list, entity->position, current_light);
}

void Light::generate_map_volumes(Bsp &map)
{
//	map.find_edges(entity->position, edge_list);
}


void Light::generate_ent_volumes(Graphics &gfx, vector<Entity *> &entity_list)
{
	for (unsigned int i = 0; i < entity_list.size(); i++)
	{
		if (entity_list[i]->ent_type == ENT_ITEM_ARMOR_COMBAT)
		{
			if (entity_list[i]->visible && entity->visible)
			{
				shadow.CreateVolume(gfx, entity_list[i]->model->model_vertex_array, entity_list[i]->model->model_index_array, entity_list[i]->model->num_index / 3, entity->position - entity_list[i]->position);

				// so shadow rotates with object
				entity->rigid->morientation = entity_list[i]->rigid->morientation;
				entity->light->shadow.position = entity_list[i]->position - entity_list[i]->model->center;
			}

		}
	}
}



