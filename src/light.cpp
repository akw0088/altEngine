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
	extend(edge_list, entity->position);
}

void Light::generate_volumes(Bsp &map)
{
	map.find_edges(entity->position, edge_list);
}

void Light::extend(Edge &edge_list, vec3 position)
{
#ifndef DIRECTX
	float t = 500.0f;

	if(true)
	{
		glEnable(GL_BLEND);
		glColor3f(0.0f, 1.0f, 0.0f);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	}

	for(int i = 0; i < edge_list.num_edges; i++)
	{
		vec3 delta_a = edge_list.edge_list[2*i] - position;
		vec3 delta_b = edge_list.edge_list[2*i+1] - position;
		vec3 a = edge_list.edge_list[2*i];
		vec3 b = edge_list.edge_list[2*i+1];
		vec3 c = b + delta_b.normalize() * t;
		vec3 d = a + delta_a.normalize() * t;

		if (vec3::crossproduct(a-c,a-b) * (entity->position - a) > 0)
		{
			glBegin(GL_TRIANGLES);
				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(a.x,a.y,a.z);
				glVertex3f(b.x,b.y,b.z);
				glColor3f(0.0f, 0.0f, 0.0f);
				glVertex3f(c.x,c.y,c.z);

				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(a.x,a.y,a.z);
				glColor3f(0.0f, 0.0f, 0.0f);
				glVertex3f(c.x,c.y,c.z);
				glVertex3f(d.x,d.y,d.z);
			glEnd();
		}
		else
		{
			glBegin(GL_TRIANGLES);
				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(a.x,a.y,a.z);
				glColor3f(0.0f, 0.0f, 0.0f);
				glVertex3f(c.x,c.y,c.z);
				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(b.x,b.y,b.z);

				glColor3f(1.0f, 1.0f, 1.0f);
				glVertex3f(a.x,a.y,a.z);
				glColor3f(0.0f, 0.0f, 0.0f);
				glVertex3f(d.x,d.y,d.z);
				glVertex3f(c.x,c.y,c.z);
			glEnd();
		}
	}
	if (true)
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		glDisable(GL_BLEND);
	}
#endif
}