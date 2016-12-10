#include "light.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Light::Light(Entity *entity, Graphics &gfx, int num)
{
	Light::entity = entity;
	color = vec3(1.0f, 1.0f, 1.0f);
	intensity = 300.0f;
	active = false;
	light_num = num;

	memset(quad_tex, 0, sizeof(unsigned int) * 6);
	memset(depth_tex, 0, sizeof(unsigned int) * 6);
	generate_cubemaps(1024, 1024);
}



void Light::generate_cubemaps(int width, int height)
{

	for (int i = 0; i < 6; i++)
	{
		glGenTextures(1, &quad_tex[i]);
		glBindTexture(GL_TEXTURE_2D, quad_tex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glGenTextures(1, &depth_tex[i]);
		glBindTexture(GL_TEXTURE_2D, depth_tex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
//		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	}

}

void Light::select_shadowmap(Graphics &gfx, int face)
{
	gfx.fbAttachTexture(quad_tex[face]);
	gfx.fbAttachDepth(depth_tex[face]);
	gfx.checkFramebuffer();
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
#ifndef DIRECTX
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





