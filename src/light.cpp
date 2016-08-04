#include "light.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Light::Light(Entity *entity)
{
	Light::entity = entity;
	color = vec3(1.0f, 1.0f, 1.0f);
	intensity = 0;
	active = false;
}

void Light::render_shadows()
{
	extend(entity->position);
}

void Light::generate_volumes(Bsp &map)
{
	map.find_backfaces(entity->position, shadow_list);
}

void Light::extend(vec3 position)
{
#ifndef DIRECTX

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
		vec3 v1 = shadow_list[i].lightdir1;
		vec3 v2 = shadow_list[i].lightdir2;
		vec3 v3 = shadow_list[i].lightdir3;

			glBegin(GL_TRIANGLES);

				//render backface
				glVertex4f(a.x, a.y, a.z, 1.0f);
				glVertex4f(b.x, b.y, b.z, 1.0f);
				glVertex4f(c.x, c.y, c.z, 1.0f);

				//render backcap
				glVertex4f(v1.x, v1.y, v1.z, 0.0f);
				glVertex4f(v2.x, v2.y, v2.z, 0.0f);
				glVertex4f(v3.x, v3.y, v3.z, 0.0f);

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