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


		extend(entity->position);
}

void Light::generate_volumes(Bsp &map)
{
//	map.find_edges(entity->position, edge_list);
}

void Light::extend(vec3 position)
{
#ifdef OPENGL_OLD
	float t = 500.0f;

	if (false)
	{
		glEnable(GL_BLEND);
		glColor3f(0.0f, 1.0f, 0.0f);
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

		if (vec3::crossproduct(a - c, a - b) * (entity->position - a) > 0)
		{
			glBegin(GL_TRIANGLES);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(a.x, a.y, a.z);
			glVertex3f(b.x, b.y, b.z);
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex3f(c.x, c.y, c.z);

			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(a.x, a.y, a.z);
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex3f(c.x, c.y, c.z);
			glVertex3f(d.x, d.y, d.z);
			glEnd();
		}
		else
		{
			glBegin(GL_TRIANGLES);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(a.x, a.y, a.z);
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex3f(c.x, c.y, c.z);
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(b.x, b.y, b.z);

			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(a.x, a.y, a.z);
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex3f(d.x, d.y, d.z);
			glVertex3f(c.x, c.y, c.z);
			glEnd();
		}
	}
	if (false)
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glDisable(GL_BLEND);
	}
#endif
}

void Light::extrude(vec3 position)
{
#ifdef OPENGL_OLD

	if(false)
	{
//		gfx.Blend(true);
//		gfx.BlendFunc("one", "one_minus_src_color")
		glEnable(GL_BLEND);
		glColor3f(0.0f, 1.0f, 0.0f);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	}

	for(int i = 0; i < shadow_list.size(); i++)
	{
		vec3 a = shadow_list[i].a;
		vec3 b = shadow_list[i].b;
		vec3 c = shadow_list[i].c;
//		vec3 v1 = shadow_list[i].lightdir1;
//		vec3 v2 = shadow_list[i].lightdir2;
//		vec3 v3 = shadow_list[i].lightdir3;

		vec3 v1 = shadow_list[i].a;
		vec3 v2 = shadow_list[i].b;
		vec3 v3 = shadow_list[i].c;


			glBegin(GL_TRIANGLES);
				//render backface
				glVertex4f(a.x, a.y, a.z, 1.0f);
				glVertex4f(b.x, b.y, b.z, 1.0f);
				glVertex4f(c.x, c.y, c.z, 1.0f);

				//render backcap
				glColor3f(0.0f, 1.0f, 0.1f);
				glVertex4f(v1.x, v1.y, v1.z, 0.0f);
				glVertex4f(v2.x, v2.y, v2.z, 0.0f);
				glVertex4f(v3.x, v3.y, v3.z, 0.0f);

				glColor3f(0.0f, 0.0f, 1.0f);
				//render volume sides
				glVertex4f(a.x, a.y, a.z, 1.0f);
				glVertex4f(v1.x, v1.y, v1.z, 0.0f);
				glVertex4f(v3.x, v3.y, v3.z, 0.0f);

				//render volume sides
				glVertex4f(c.x, c.y, c.z, 1.0f);
				glVertex4f(a.x, a.y, a.z, 1.0f);
				glVertex4f(v3.x, v3.y, v3.z, 0.0f);

				//render volume sides
				glVertex4f(a.x, a.y, a.z, 1.0f);
				glVertex4f(v2.x, v2.y, v2.z, 0.0f);
				glVertex4f(v1.x, v1.y, v1.z, 0.0f);

				//render volume sides
				glVertex4f(b.x, b.y, b.z, 1.0f);
				glVertex4f(v2.x, v2.y, v2.z, 0.0f);
				glVertex4f(a.x, a.y, a.z, 1.0f);

				//render volume sides
				glVertex4f(c.x, c.y, c.z, 1.0f);
				glVertex4f(v2.x, v2.y, v2.z, 0.0f);
				glVertex4f(b.x, b.y, b.z, 1.0f);

				//render volume sides
				glVertex4f(c.x, c.y, c.z, 1.0f);
				glVertex4f(v2.x, v2.y, v2.z, 0.0f);
				glVertex4f(v3.x, v3.y, v3.z, 0.0f);
				
			glEnd();
	}
	if (false)
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glDisable(GL_BLEND);
	}
#endif
}




