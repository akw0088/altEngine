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
	glGenTextures(6, &quad_tex[0]);
	glGenTextures(6, &depth_tex[0]);

	for (int i = 0; i < 6; i++)
	{
		glBindTexture(GL_TEXTURE_2D, quad_tex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glBindTexture(GL_TEXTURE_2D, depth_tex[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	}

}

void Light::select_shadowmap(Graphics &gfx, int face)
{
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, quad_tex[face], 0);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex[face], 0);
	gfx.checkFramebuffer();
}

void Light::render_shadowmap(Graphics &gfx, Bsp &bsp, Global &global)
{
	matrix4 mvp[6];

	// Generate matrices
	mat_right(mvp[0], entity->position);
	mat_left(mvp[1], entity->position);
	mat_top(mvp[2], entity->position);
	mat_bottom(mvp[3], entity->position);
	mat_forward(mvp[4], entity->position);
	mat_backward(mvp[5], entity->position);

	for (int i = 0; i < 1; i++)
	{
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, quad_tex[i], 0);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex[i], 0);
		gfx.checkFramebuffer();

		gfx.clear();
		global.Select();
		global.Params(mvp[i], 0);
		bsp.render(entity->position, NULL, gfx);
		gfx.SelectShader(0);
	}
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
	map.find_edges(entity->position, edge_list);
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


void Light::mat_forward(matrix4 &mvp, vec3 &position)
{
	vec3 right(1.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 forward(0.0f, 0.0f, -1.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = right.y;
	mvp.m[2] = right.z;
	mvp.m[3] = 0.0f;

	mvp.m[4] = up.x;
	mvp.m[5] = up.y;
	mvp.m[6] = up.z;
	mvp.m[7] = 0.0f;

	mvp.m[8]  = forward.x;
	mvp.m[9]  = forward.y;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = right * position;
	mvp.m[13] = up * position;
	mvp.m[14] = forward * position;
	mvp.m[15] = 1.0f;
}

void Light::mat_left(matrix4 &mvp, vec3 &position)
{
	vec3 right(0.0f, 0.0f, -1.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 forward(-1.0f, 0.0f, 0.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = right.y;
	mvp.m[2] = right.z;
	mvp.m[3] = 0.0f;

	mvp.m[4] = up.x;
	mvp.m[5] = up.y;
	mvp.m[6] = up.z;
	mvp.m[7] = 0.0f;

	mvp.m[8] = forward.x;
	mvp.m[9] = forward.y;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = right * position;
	mvp.m[13] = up * position;
	mvp.m[14] = forward * position;
	mvp.m[15] = 1.0f;
}

void Light::mat_backward(matrix4 &mvp, vec3 &position)
{
	vec3 right(-1.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 forward(0.0f, 0.0f, 1.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = right.y;
	mvp.m[2] = right.z;
	mvp.m[3] = 0.0f;

	mvp.m[4] = up.x;
	mvp.m[5] = up.y;
	mvp.m[6] = up.z;
	mvp.m[7] = 0.0f;

	mvp.m[8] = forward.x;
	mvp.m[9] = forward.y;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = right * position;
	mvp.m[13] = up * position;
	mvp.m[14] = forward * position;
	mvp.m[15] = 1.0f;
}

void Light::mat_right(matrix4 &mvp, vec3 &position)
{
	vec3 right(0.0f, 0.0f, 1.0f);
	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 forward(1.0f, 0.0f, 0.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = right.y;
	mvp.m[2] = right.z;
	mvp.m[3] = 0.0f;

	mvp.m[4] = up.x;
	mvp.m[5] = up.y;
	mvp.m[6] = up.z;
	mvp.m[7] = 0.0f;

	mvp.m[8] = forward.x;
	mvp.m[9] = forward.y;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = right * position;
	mvp.m[13] = up * position;
	mvp.m[14] = forward * position;
	mvp.m[15] = 1.0f;
}

void Light::mat_top(matrix4 &mvp, vec3 &position)
{
	vec3 right(1.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 0.0f, -1.0f);
	vec3 forward(0.0f, -1.0f, 0.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = right.y;
	mvp.m[2] = right.z;
	mvp.m[3] = 0.0f;

	mvp.m[4] = up.x;
	mvp.m[5] = up.y;
	mvp.m[6] = up.z;
	mvp.m[7] = 0.0f;

	mvp.m[8] = forward.x;
	mvp.m[9] = forward.y;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = right * position;
	mvp.m[13] = up * position;
	mvp.m[14] = forward * position;
	mvp.m[15] = 1.0f;
}

void Light::mat_bottom(matrix4 &mvp, vec3 &position)
{
	vec3 right(1.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 0.0f, 1.0f);
	vec3 forward(0.0f, 1.0f, 0.0f);

	mvp.m[0] = right.x;
	mvp.m[1] = right.y;
	mvp.m[2] = right.z;
	mvp.m[3] = 0.0f;

	mvp.m[4] = up.x;
	mvp.m[5] = up.y;
	mvp.m[6] = up.z;
	mvp.m[7] = 0.0f;

	mvp.m[8] = forward.x;
	mvp.m[9] = forward.y;
	mvp.m[10] = forward.z;
	mvp.m[11] = 0.0f;

	mvp.m[12] = right * position;
	mvp.m[13] = up * position;
	mvp.m[14] = forward * position;
	mvp.m[15] = 1.0f;
}


void Light::mat_cube(float *cube, vec3 &position)
{
	matrix4 mvp[6];
	int j = 0;

	// Generate matrices
	mat_top(mvp[0], position);
	mat_bottom(mvp[1], position);
	mat_left(mvp[2], position);
	mat_right(mvp[3], position);
	mat_forward(mvp[4], position);
	mat_backward(mvp[5], position);

	// Combine them, could probably generate them directly
	for (int i = 0; i < 96; i++)
	{
		cube[i] = mvp[j++].m[i];
		if (j == 16)
			j = 0;
	}
}

