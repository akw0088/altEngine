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

void Light::render_shadow_volumes(int current_light)
{
	float ident[9] = { 1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f};

		entity->rigid->angular_velocity = vec3();
		entity->rigid->morientation.m[0] = ident[0];
		entity->rigid->morientation.m[1] = ident[1];
		entity->rigid->morientation.m[2] = ident[2];
		entity->rigid->morientation.m[3] = ident[3];
		entity->rigid->morientation.m[4] = ident[4];
		entity->rigid->morientation.m[5] = ident[5];
		entity->rigid->morientation.m[6] = ident[6];
		entity->rigid->morientation.m[7] = ident[7];
		entity->rigid->morientation.m[8] = ident[8];


		extend(edge_list, entity->position, current_light);
}

void Light::generate_volumes(Bsp &map)
{
	map.find_edges(entity->position, edge_list);
}



void Light::extend(Edge &edge_list, vec3 position, int current_light)
{
#ifdef OPENGL
	float t = 10.0f;
	bool debug = true;


	if (debug)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	}

	for (int i = 0; i < edge_list.num_edges; i++)
	{
		vec3 delta_a = edge_list.edge_list[2 * i] - position;
		vec3 delta_b = edge_list.edge_list[2 * i + 1] - position;
		vec3 a = edge_list.edge_list[2 * i];
		vec3 b = edge_list.edge_list[2 * i + 1];
		vec3 c = b + delta_b.normalize() * t;
		vec3 d = a + delta_a.normalize() * t;

			glBegin(GL_TRIANGLES);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(a.x, a.y, a.z);
			glVertex3f(b.x, b.y, b.z);
			glVertex3f(c.x, c.y, c.z);

			glVertex3f(a.x, a.y, a.z);
			glVertex3f(c.x, c.y, c.z);
			glVertex3f(d.x, d.y, d.z);
			glEnd();
			glBegin(GL_TRIANGLES);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(a.x, a.y, a.z);
			glVertex3f(c.x, c.y, c.z);
			glVertex3f(b.x, b.y, b.z);

			glVertex3f(a.x, a.y, a.z);
			glVertex3f(d.x, d.y, d.z);
			glVertex3f(c.x, c.y, c.z);
			glEnd();
	}
	if (debug)
	{
		glDisable(GL_BLEND);
	}
#endif
}





