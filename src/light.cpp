#include "light.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Light::Light(Entity *entity, Graphics &gfx, int num)
{
	Light::entity = entity;
	color = vec3(1.0f, 1.0f, 1.0f);
	intensity = 0;
	active = false;
	light_num = num;

	generate_cubemaps(1280, 1280);
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glGenTextures(1, &depth_tex[i]);
		glBindTexture(GL_TEXTURE_2D, depth_tex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	}

}

void Light::select_shadowmap(Graphics &gfx, int face)
{
	gfx.fbAttachTexture(quad_tex[face]);
	gfx.fbAttachDepth(depth_tex[face]);
	gfx.checkFramebuffer();
}


void Light::render_shadow_volumes()
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


//		extend(entity->position);
}

void Light::generate_volumes(Bsp &map)
{
//	map.find_edges(entity->position, edge_list);
}







