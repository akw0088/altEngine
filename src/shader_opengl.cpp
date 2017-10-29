#include "shader.h"
#include <cmath> // for some sin/cos/tan funcs

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifdef OPENGL
//unsigned int ParticleUpdate::max_particles = 175000;

// Setting to a low value to keep FPS up
unsigned int ParticleUpdate::max_particles = 5000;

int mFont::init(Graphics *gfx)
{
#ifdef __OBJC__
	if (Shader::init(gfx, "media/glsl/ver410/font.vs", NULL, "media/glsl/ver410/font.fs"))
	{
		program_handle = -1;
		return -1;
	}
#else
	if (Shader::init(gfx, "media/glsl/ver440/font.vs", NULL, "media/glsl/ver440/font.fs"))
	{
		program_handle = -1;
		return -1;
	}
#endif
	u_scale = glGetUniformLocation(program_handle, "u_scale");
	u_row = glGetUniformLocation(program_handle, "u_row");
	u_col = glGetUniformLocation(program_handle, "u_col");
	u_xpos = glGetUniformLocation(program_handle, "u_xpos");
	u_ypos = glGetUniformLocation(program_handle, "u_ypos");
	u_color = glGetUniformLocation(program_handle, "u_color");
	texture0 = glGetUniformLocation(program_handle, "texture0");
	return 0;
}

void mFont::prelink()
{
	glBindAttribLocation(program_handle, 0, "attr_position");
	glBindAttribLocation(program_handle, 1, "attr_TexCoord");
	glBindAttribLocation(program_handle, 4, "attr_color");
}

void mFont::Params(char c, float x, float y, float scale, vec3 &color)
{
	int col = c % 16;
	int row = 15 - c / 16;
	float offset = 16.0f / 256.0f;

	glUniform1f(u_scale, scale);
	glUniform1f(u_col, col * offset);
	glUniform1f(u_row, row * offset);
	glUniform1f(u_xpos, (2.0f * x - 1.0f));
	glUniform1f(u_ypos, (2.0f * (1.0f - y) - 1.0f));
	glUniform3fv(u_color, 1, (float *)&color);
	glUniform1i(texture0, 0);
}

int Global::init(Graphics *gfx)
{
#ifdef __OBJC__
	if (Shader::init(gfx, "media/glsl/ver410/global.vs", NULL, "media/glsl/ver410/global.fs"))
	{
		program_handle = -1;
		return -1;
	}
#else
	if (Shader::init(gfx, "media/glsl/ver440/global.vs", NULL, "media/glsl/ver440/global.fs"))
	{
		program_handle = -1;
		return -1;
	}
#endif
	matrix = glGetUniformLocation(program_handle, "mvp");
	texture0 = glGetUniformLocation(program_handle, "texture0");
	u_depth = glGetUniformLocation(program_handle, "u_depth");
	return 0;
}

void Global::prelink()
{
	glBindAttribLocation(program_handle, 0, "attr_position");
	glBindAttribLocation(program_handle, 1, "attr_TexCoord");
	glBindAttribLocation(program_handle, 4, "attr_color");
}

void Global::Params(matrix4 &mvp, int tex0, int depth)
{
	glUniformMatrix4fv(matrix, 1, GL_FALSE, mvp.m);
	glUniform1i(texture0, tex0);
	glUniform1i(u_depth, depth);
}

int mLight2::init(Graphics *gfx)
{

	max_light = MAX_LIGHTS;
	m_brightness = 0.0f;
	m_contrast = 1.0f;
	m_exposure = 1.0f;
	//"media/glsl/mlighting3.gs"
#ifdef __OBJC__
	if (Shader::init(gfx, "media/glsl/ver410/mlighting3.vs", "media/glsl/ver410/mlighting3.gs", "media/glsl/ver410/mlighting3.fs"))
	{
		program_handle = -1;
		return -1;
	}
#else
//	if (Shader::init(gfx, "media/glsl/ver440/mlighting3.vs", "media/glsl/ver440/mlighting3.gs", "media/glsl/ver440/mlighting3.fs"))
	if (Shader::init(gfx, "media/glsl/ver440/mlighting3.vs", NULL, "media/glsl/ver440/mlighting3.fs")) 
	{
		program_handle = -1;
		return -1;
	}
#endif

	m_alpha = -1.0f;

	matrix = glGetUniformLocation(program_handle, "mvp");
	shadow_matrix[0] = glGetUniformLocation(program_handle, "shadow_matrix[0]");
	shadow_matrix[1] = glGetUniformLocation(program_handle, "shadow_matrix[1]");
	shadow_matrix[2] = glGetUniformLocation(program_handle, "shadow_matrix[2]");
	shadow_matrix[3] = glGetUniformLocation(program_handle, "shadow_matrix[3]");
	shadow_matrix[4] = glGetUniformLocation(program_handle, "shadow_matrix[4]");
	shadow_matrix[5] = glGetUniformLocation(program_handle, "shadow_matrix[5]");
	shadow_matrix[6] = glGetUniformLocation(program_handle, "shadow_matrix[6]");
	shadow_matrix[7] = glGetUniformLocation(program_handle, "shadow_matrix[7]");
	shadow_matrix[8] = glGetUniformLocation(program_handle, "shadow_matrix[8]");
	shadow_matrix[9] = glGetUniformLocation(program_handle, "shadow_matrix[9]");
	shadow_matrix[10] = glGetUniformLocation(program_handle, "shadow_matrix[10]");
	shadow_matrix[11] = glGetUniformLocation(program_handle, "shadow_matrix[11]");
	shadow_matrix[12] = glGetUniformLocation(program_handle, "shadow_matrix[12]");
	shadow_matrix[13] = glGetUniformLocation(program_handle, "shadow_matrix[13]");
	shadow_matrix[14] = glGetUniformLocation(program_handle, "shadow_matrix[14]");
	shadow_matrix[15] = glGetUniformLocation(program_handle, "shadow_matrix[15]");
	shadow_matrix[16] = glGetUniformLocation(program_handle, "shadow_matrix[16]");
	shadow_matrix[17] = glGetUniformLocation(program_handle, "shadow_matrix[17]");

	texture[0] = glGetUniformLocation(program_handle, "tex[0]");
	texture[1] = glGetUniformLocation(program_handle, "tex[1]");
	texture[2] = glGetUniformLocation(program_handle, "tex[2]");
	texture[3] = glGetUniformLocation(program_handle, "tex[3]");

	depth[0] = glGetUniformLocation(program_handle, "depth[0]");
	depth[1]= glGetUniformLocation(program_handle, "depth[1]");
	depth[2]= glGetUniformLocation(program_handle, "depth[2]");
	depth[3]= glGetUniformLocation(program_handle, "depth[3]");
	depth[4]= glGetUniformLocation(program_handle, "depth[4]");
	depth[5]= glGetUniformLocation(program_handle, "depth[5]");
	depth[6]= glGetUniformLocation(program_handle, "depth[6]");
	depth[7]= glGetUniformLocation(program_handle, "depth[7]");
	depth[8]= glGetUniformLocation(program_handle, "depth[8]");
	depth[9]= glGetUniformLocation(program_handle, "depth[9]");
	depth[10] = glGetUniformLocation(program_handle, "depth[10]");
	depth[11] = glGetUniformLocation(program_handle, "depth[11]");
	depth[12] = glGetUniformLocation(program_handle, "depth[12]");
	depth[13] = glGetUniformLocation(program_handle, "depth[13]");
	depth[14] = glGetUniformLocation(program_handle, "depth[14]");
	depth[15] = glGetUniformLocation(program_handle, "depth[15]");
	depth[16] = glGetUniformLocation(program_handle, "depth[16]");
	depth[17] = glGetUniformLocation(program_handle, "depth[17]");


	texture_lightmap = glGetUniformLocation(program_handle, "texture_lightmap");
	texture_normalmap = glGetUniformLocation(program_handle, "texture_normalmap");

	u_tcmod_scroll[0] = glGetUniformLocation(program_handle, "u_tcmod_scroll[0]");
	u_tcmod_scroll[1] = glGetUniformLocation(program_handle, "u_tcmod_scroll[1]");
	u_tcmod_scroll[2] = glGetUniformLocation(program_handle, "u_tcmod_scroll[2]");
	u_tcmod_scroll[3] = glGetUniformLocation(program_handle, "u_tcmod_scroll[3]");

	u_tcmod_scale[0] = glGetUniformLocation(program_handle, "u_tcmod_scale[0]");
	u_tcmod_scale[1] = glGetUniformLocation(program_handle, "u_tcmod_scale[1]");
	u_tcmod_scale[2] = glGetUniformLocation(program_handle, "u_tcmod_scale[2]");
	u_tcmod_scale[3] = glGetUniformLocation(program_handle, "u_tcmod_scale[3]");

	u_tcmod_sin[0] = glGetUniformLocation(program_handle, "u_tcmod_sin[0]");
	u_tcmod_sin[1] = glGetUniformLocation(program_handle, "u_tcmod_sin[1]");
	u_tcmod_sin[2] = glGetUniformLocation(program_handle, "u_tcmod_sin[2]");
	u_tcmod_sin[3] = glGetUniformLocation(program_handle, "u_tcmod_sin[3]");

	u_tcmod_cos[0] = glGetUniformLocation(program_handle, "u_tcmod_cos[0]");
	u_tcmod_cos[1] = glGetUniformLocation(program_handle, "u_tcmod_cos[1]");
	u_tcmod_cos[2] = glGetUniformLocation(program_handle, "u_tcmod_cos[2]");
	u_tcmod_cos[3] = glGetUniformLocation(program_handle, "u_tcmod_cos[3]");

	u_env[0] = glGetUniformLocation(program_handle, "u_env[0]");
	u_env[1] = glGetUniformLocation(program_handle, "u_env[1]");
	u_env[2] = glGetUniformLocation(program_handle, "u_env[2]");
	u_env[3] = glGetUniformLocation(program_handle, "u_env[3]");

	u_water[0] = glGetUniformLocation(program_handle, "u_water[0]");
	u_water[1] = glGetUniformLocation(program_handle, "u_water[1]");
	u_water[2] = glGetUniformLocation(program_handle, "u_water[2]");
	u_water[3] = glGetUniformLocation(program_handle, "u_water[3]");

	u_alpha = glGetUniformLocation(program_handle, "u_alpha");
	u_alphatest[0] = glGetUniformLocation(program_handle, "u_alphatest[0]");
	u_alphatest[1] = glGetUniformLocation(program_handle, "u_alphatest[1]");
	u_alphatest[2] = glGetUniformLocation(program_handle, "u_alphatest[2]");
	u_alphatest[3] = glGetUniformLocation(program_handle, "u_alphatest[3]");

	u_rgbgen_scale[0] = glGetUniformLocation(program_handle, "u_rgbgen_scale[0]");
	u_rgbgen_scale[1] = glGetUniformLocation(program_handle, "u_rgbgen_scale[1]");
	u_rgbgen_scale[2] = glGetUniformLocation(program_handle, "u_rgbgen_scale[2]");
	u_rgbgen_scale[3] = glGetUniformLocation(program_handle, "u_rgbgen_scale[3]");

	u_time = glGetUniformLocation(program_handle, "u_time");
	u_portal = glGetUniformLocation(program_handle, "u_portal");
	u_normalmap = glGetUniformLocation(program_handle, "u_normalmap");

	u_ambient = glGetUniformLocation(program_handle, "u_ambient");
	u_brightness = glGetUniformLocation(program_handle, "u_brightness");
	u_exposure = glGetUniformLocation(program_handle, "u_exposure");
	u_contrast = glGetUniformLocation(program_handle, "u_contrast");
	u_lightmap = glGetUniformLocation(program_handle, "u_lightmap");
	u_shadowmap = glGetUniformLocation(program_handle, "u_shadowmap");
	u_num_lights = glGetUniformLocation(program_handle, "u_num_lights");
	u_position = glGetUniformLocation(program_handle, "u_position");
	u_color = glGetUniformLocation(program_handle, "u_color");
	u_lightmap_stage = glGetUniformLocation(program_handle, "u_lightmap_stage");
	u_depth = glGetUniformLocation(program_handle, "u_depth");

	u_fog = glGetUniformLocation(program_handle, "u_fog");
	u_fog_start = glGetUniformLocation(program_handle, "u_fog_start");
	u_fog_end = glGetUniformLocation(program_handle, "u_fog_end");
	u_fog_color = glGetUniformLocation(program_handle, "u_fog_color");


	u_clip0 = glGetUniformLocation(program_handle, "u_clip0");
	u_clip1 = glGetUniformLocation(program_handle, "u_clip1");
	u_clip2 = glGetUniformLocation(program_handle, "u_clip2");
	u_clip3 = glGetUniformLocation(program_handle, "u_clip3");

	return 0;
}

Global::~Global()
{
}

void mLight2::set_max(int max)
{
	max_light = max;
}


void mLight2::set_clip(vec4 *plane)
{
	glUniform4f(u_clip0, plane[0].x, plane[0].y, plane[0].z, plane[0].w);
	glUniform4f(u_clip1, plane[1].x, plane[1].y, plane[1].z, plane[1].w);
	glUniform4f(u_clip2, plane[2].x, plane[2].y, plane[2].z, plane[2].w);
	glUniform4f(u_clip3, plane[3].x, plane[3].y, plane[3].z, plane[3].w);

	glEnable(GL_CLIP_DISTANCE0);
	glEnable(GL_CLIP_DISTANCE1);
	glEnable(GL_CLIP_DISTANCE2);
	glEnable(GL_CLIP_DISTANCE3);

}


void mLight2::set_fog(float fog, float start, float end, vec3 &color)
{
	m_fog = fog;
	m_start = start;
	m_end = end;
	m_color = color;

	glUniform1f(u_fog, m_fog);
	glUniform1f(u_fog_start, m_start);
	glUniform1f(u_fog_end, m_end);
	glUniform3f(u_fog_color, m_color.x, m_color.y, m_color.z);

}

//=============================================================================
// Mostly effects things not lit by lightmaps
//=============================================================================
void mLight2::set_ambient(float ambient)
{
	m_ambient = ambient;
}

//=============================================================================
// bias added to final fragment color, tends to wash out color when above 1.0
//=============================================================================
void mLight2::set_brightness(float value)
{
	m_brightness = value;
}

void mLight2::set_exposure(float value)
{
	m_exposure = value;
}


//=============================================================================
// scalar multiplied by final fragment color, high contrast makes things stand out
//=============================================================================
void mLight2::set_contrast(float value)
{
	m_contrast = value;
}


//=============================================================================
// Used to scale between dynamic light and lightmaps
// 1.0 means 100% lightmap
// 0.5 means 50% lightmap 50% dynamic lighting
// 0.0 means 100% dynamic lighting
//=============================================================================
void mLight2::set_lightmap(float lightmap)
{
	m_lightmap = lightmap;
}

void mLight2::set_shadowmap(float value)
{
	m_shadowmap = value;
}

void mLight2::set_alpha(float value)
{
	m_alpha = value;
	glUniform1f(u_alpha, value);
}


void mLight2::prelink()
{
	glBindAttribLocation(program_handle, 0, "attr_position");
	glBindAttribLocation(program_handle, 1, "attr_TexCoord");
	glBindAttribLocation(program_handle, 2, "attr_LightCoord");
	glBindAttribLocation(program_handle, 3, "attr_normal");
	glBindAttribLocation(program_handle, 4, "attr_color");
	glBindAttribLocation(program_handle, 5, "attr_tangent");
}

void mLight2::set_shadow_matrix(int index, matrix4 &mvp)
{
	glUniformMatrix4fv(shadow_matrix[index], 1, GL_FALSE, mvp.m);
}


void mLight2::set_matrix(matrix4 &mvp)
{
	glUniformMatrix4fv(matrix, 1, GL_FALSE, mvp.m);
}

void mLight2::Params(matrix4 &mvp, vector<Light *> &light_list, size_t num_lights, vec3 &offset, int time)
{
	vec4 position[MAX_LIGHTS];
	vec4 color[MAX_LIGHTS];
	unsigned int i, j;

	for(i = 0, j = 0; i < num_lights && j < MAX_LIGHTS; i++, j++)
	{
		Light *light = light_list[i];

		if (light->active)
		{
			vec3 pos = light->entity->position - offset;

			position[j] = vec4(pos.x, pos.y, pos.z, light->attenuation);


			color[j] = vec4(light->color.x,
							light->color.y,
							light->color.z,
					(float)(light->intensity / 500.0f));
		}
	}

	glUniformMatrix4fv(matrix, 1, GL_FALSE, mvp.m);
	glUniform1i(texture[0], 0);
	glUniform1i(texture[1], 1);
	glUniform1i(texture[2], 2);
	glUniform1i(texture[3], 3);
//	glUniform1i(texture[4], 4);
//	glUniform1i(texture[5], 5);
//	glUniform1i(texture[6], 6);
//	glUniform1i(texture[7], 7);

	glUniform1f(u_alpha, -1.0f);


	glUniform1i(depth[0], 10);
	glUniform1i(depth[1], 11);
	glUniform1i(depth[2], 12);
	glUniform1i(depth[3], 13);
	glUniform1i(depth[4], 14);
	glUniform1i(depth[5], 15);
	glUniform1i(depth[6], 16);
	glUniform1i(depth[7], 17);
	glUniform1i(depth[8], 18);
	glUniform1i(depth[9], 19);
	glUniform1i(depth[10], 20);
	glUniform1i(depth[11], 21);
	glUniform1i(depth[12], 22);
	glUniform1i(depth[13], 23);
	glUniform1i(depth[14], 24);
	glUniform1i(depth[15], 25);
	glUniform1i(depth[16], 26);
	glUniform1i(depth[17], 27);


	glUniform1f(u_fog, m_fog);
	glUniform1f(u_fog_start, m_start);
	glUniform1f(u_fog_end, m_end);
	glUniform3f(u_fog_color, m_color.x, m_color.y, m_color.z);



	glUniform1i(u_time, time);


// Going to treat normals and lightmaps like normal textures
	glUniform1i(texture_lightmap, 8);
	glUniform1i(texture_normalmap, 9);

	vec2 tcmod_scroll = vec2(0.0f, 0.0f);
	vec2 tcmod_scale = vec2(1.0f, 1.0f); // 2.0 makes it half the size
	float tcmod_sin = 0.0f;
	float tcmod_cos = 1.0f;
	int env = 0;

	glUniform2fv(u_tcmod_scroll[0], 1, (float *)&tcmod_scroll);
	glUniform2fv(u_tcmod_scroll[1], 1, (float *)&tcmod_scroll);
	glUniform2fv(u_tcmod_scroll[2], 1, (float *)&tcmod_scroll);
	glUniform2fv(u_tcmod_scroll[3], 1, (float *)&tcmod_scroll);
//	glUniform2fv(u_tcmod_scroll4, 1, (float *)&tcmod_scroll);
//	glUniform2fv(u_tcmod_scroll5, 1, (float *)&tcmod_scroll);
//	glUniform2fv(u_tcmod_scroll6, 1, (float *)&tcmod_scroll);
//	glUniform2fv(u_tcmod_scroll7, 1, (float *)&tcmod_scroll);

	glUniform2fv(u_tcmod_scale[0], 1, (float *)&tcmod_scale);
	glUniform2fv(u_tcmod_scale[1], 1, (float *)&tcmod_scale);
	glUniform2fv(u_tcmod_scale[2], 1, (float *)&tcmod_scale);
	glUniform2fv(u_tcmod_scale[3], 1, (float *)&tcmod_scale);
//	glUniform2fv(u_tcmod_scale4, 1, (float *)&tcmod_scale);
//	glUniform2fv(u_tcmod_scale5, 1, (float *)&tcmod_scale);
//	glUniform2fv(u_tcmod_scale6, 1, (float *)&tcmod_scale);
//	glUniform2fv(u_tcmod_scale7, 1, (float *)&tcmod_scale);


	glUniform1fv(u_rgbgen_scale[0], 1, (float *)&tcmod_scale);
	glUniform1fv(u_rgbgen_scale[1], 1, (float *)&tcmod_scale);
	glUniform1fv(u_rgbgen_scale[2], 1, (float *)&tcmod_scale);
	glUniform1fv(u_rgbgen_scale[3], 1, (float *)&tcmod_scale);


	glUniform1fv(u_tcmod_sin[0], 1, &tcmod_sin);
	glUniform1fv(u_tcmod_sin[1], 1, &tcmod_sin);
	glUniform1fv(u_tcmod_sin[2], 1, &tcmod_sin);
	glUniform1fv(u_tcmod_sin[3], 1, &tcmod_sin);
//	glUniform1fv(u_tcmod_sin4, 1, &tcmod_sin);
//	glUniform1fv(u_tcmod_sin5, 1, &tcmod_sin);
//	glUniform1fv(u_tcmod_sin6, 1, &tcmod_sin);
//	glUniform1fv(u_tcmod_sin7, 1, &tcmod_sin);

	glUniform1fv(u_tcmod_cos[0], 1, &tcmod_cos);
	glUniform1fv(u_tcmod_cos[1], 1, &tcmod_cos);
	glUniform1fv(u_tcmod_cos[2], 1, &tcmod_cos);
	glUniform1fv(u_tcmod_cos[3], 1, &tcmod_cos);
//	glUniform1fv(u_tcmod_cos4, 1, &tcmod_cos);
//	glUniform1fv(u_tcmod_cos5, 1, &tcmod_cos);
//	glUniform1fv(u_tcmod_cos6, 1, &tcmod_cos);
//	glUniform1fv(u_tcmod_cos7, 1, &tcmod_cos);

	glUniform1i(u_env[0], env);
	glUniform1i(u_env[1], env);
	glUniform1i(u_env[2], env);
	glUniform1i(u_env[3], env);
	//	glUniform1fv(u_tcmod_cos4, 1, &tcmod_cos);
	//	glUniform1fv(u_tcmod_cos5, 1, &tcmod_cos);
	//	glUniform1fv(u_tcmod_cos6, 1, &tcmod_cos);
	//	glUniform1fv(u_tcmod_cos7, 1, &tcmod_cos);

	glUniform1f(u_ambient, m_ambient);
	glUniform1f(u_lightmap, m_lightmap);
	glUniform1f(u_brightness, m_brightness);
	glUniform1f(u_exposure, m_exposure);
	glUniform1f(u_contrast, m_contrast);
	glUniform1f(u_shadowmap, m_shadowmap);
	glUniform1i(u_num_lights, MIN(j, max_light));
	glUniform4fv(u_position, j, (float *)&position);
	glUniform4fv(u_color, j, (float *)&color);

	glUniform1i(u_lightmap_stage, 0);

	m_num_light = j;
}

void mLight2::set_light(float ambient, float lightmap, int num_light)
{
	glUniform1i(u_num_lights, num_light);
	glUniform1f(u_ambient, ambient);
	glUniform1f(u_lightmap, lightmap);
	//	m_num_light = num_light;
	//	m_ambient = ambient;
	//	m_lightmap = lightmap;
}


void mLight2::set_lightmap_stage(int flag)
{
	glUniform1i(u_lightmap_stage, flag);
}

void mLight2::set_depth(int flag)
{
	glUniform1i(u_depth, flag);
}


void mLight2::alphatest(int stage, int mode)
{
	glUniform1i(u_alphatest[stage], mode);
}

void mLight2::envmap(int stage, int env)
{
	glUniform1i(u_env[stage], env);
}

void mLight2::rgbgen_scale(int stage, float scale)
{
	glUniform1f(u_rgbgen_scale[stage], scale);
}

void mLight2::turb(int stage, int turb)
{
	glUniform1i(u_water[stage], turb);
}

void mLight2::portal(int portal)
{
	glUniform1i(u_portal, portal);
}

void mLight2::set_normalmap(int value)
{
	glUniform1i(u_normalmap, value);
}

void mLight2::tcmod_scroll(vec2 &scroll, int index)
{
	glUniform2fv(u_tcmod_scroll[index], 1, (float *)&scroll);
}

void mLight2::tcmod_scale(vec2 &scale, int index)
{
	glUniform2fv(u_tcmod_scale[index], 1, (float *)&scale);
}



void mLight2::tcmod_rotate(float deg, int index)
{
	//convert to radians
	deg = (float)(MY_PI * deg / 180.0f);
	
	float sinval = sinf(deg);
	float cosval = cosf(deg);

	glUniform1fv(u_tcmod_sin[index], 1, &sinval);
	glUniform1fv(u_tcmod_cos[index], 1, &cosval);
}

void mLight2::tcmod_stretch_sin(float amplitude, float phase, float freq, int tick_num, int index)
{
	vec2 value;

	if (abs32((float)(freq - 0.001)) <= 0.0011)
		freq = 0.1f;

	// or jumppad aesthetics
	amplitude *= 0.8f;
	amplitude += amplitude / 2.25f;

	value.x = (float)( amplitude * fsin(freq * tick_num + phase) );
	value.y = value.x;

	tcmod_scale(value, index);
}

void mLight2::tcmod_stretch_sawtooth(float amplitude, float phase, float freq, int tick_num, int index)
{
	vec2 value;

	//cot(x)=1/tan(x)
	value.x = (float) ( amplitude * ( -0.5 * atan(1.0 / tan(freq * tick_num / MY_PI + phase)) ) );
	value.y = value.x;

	tcmod_scale(value, index);
}

void mLight2::tcmod_stretch_inverse_sawtooth(float amplitude, float phase, float freq, int tick_num, int index)
{
	tcmod_stretch_sawtooth(amplitude, phase, freq, -tick_num, index);
}

void mLight2::tcmod_stretch_triangle(float amplitude, float phase, float freq, int tick_num, int index)
{
	vec2 value;
	float x = (float)(0.5f * tick_num * freq + phase);
	value.x = (float)( amplitude * (1.0f - 2.0f * abs32(sign(x) - x)) );
	value.y = value.x;

	tcmod_scale(value, index);
}

void mLight2::tcmod_stretch_square(float amplitude, float phase, float freq, int tick_num, int index)
{
	vec2 value;

	value.x = (float)( amplitude * (4.0 / ((int)(freq * tick_num + phase) % 2 * MY_PI)) );
	value.y = value.x;

	tcmod_scale(value, index);
}

//rgbgen
void mLight2::rgbgen_wave_sin(float amplitude, float phase, float freq, int tick_num, int index)
{
	float value;

//	if (abs32((float)(freq - 0.001)) <= 0.0011)
//		freq = 0.1f;

	value = (float)(amplitude * fsin(freq * tick_num / 10.0f + phase));

	rgbgen_scale(index, value);
}

void mLight2::rgbgen_wave_sawtooth(float amplitude, float phase, float freq, int tick_num, int index)
{
	float value;

	//cot(x)=1/tan(x)
	value = (float)(amplitude * (-0.5 * atan(1.0 / tan(freq / 10.0f * tick_num / MY_PI + phase))));

	rgbgen_scale(index, value);
}

void mLight2::rgbgen_wave_inverse_sawtooth(float amplitude, float phase, float freq, int tick_num, int index)
{
	rgbgen_wave_sawtooth(amplitude, phase, freq, -tick_num, index);
}

void mLight2::rgbgen_wave_triangle(float amplitude, float phase, float freq, int tick_num, int index)
{
	float value;
	float x = (float)(0.5f * tick_num / 10.0f * freq + phase);
	value = (float)(amplitude * (1.0f - 2.0f * abs32(sign(x) - x)));

	rgbgen_scale(index, value);
}

void mLight2::rgbgen_wave_square(float amplitude, float phase, float freq, int tick_num, int index)
{
	float value;
	float denom;

	if (abs32((float)(freq - 0.001f)) <= 0.0011f)
		freq = 0.1f;

	denom = ((int)(freq * tick_num / 10.0f + phase) % 2 * MY_PI);

	if (abs32(denom) > 0.0001f)
	{
		value = (float)(amplitude * (4.0f / denom)) + 0.5f;
		rgbgen_scale(index, value);
	}
}

int Post::init(Graphics *gfx)
{
#ifdef __OBJC__
    if (Shader::init(gfx, "media/glsl/ver410/post.vs", NULL, "media/glsl/ver410/post.fs"))
    {
        program_handle = -1;
        return -1;
    }
#else
    if (Shader::init(gfx, "media/glsl/ver440/post.vs", NULL, "media/glsl/ver440/post.fs"))
    {
        program_handle = -1;
        return -1;
    }
#endif

	texture0 = glGetUniformLocation(program_handle, "texture0");
	texture1 = glGetUniformLocation(program_handle, "texture1");
	texture2 = glGetUniformLocation(program_handle, "texture2");

	tc_offset = glGetUniformLocation(program_handle, "tc_offset");
	u_type = glGetUniformLocation(program_handle, "u_type");
	u_time = glGetUniformLocation(program_handle, "u_time");

	u_dir = glGetUniformLocation(program_handle, "u_dir");
	u_scale = glGetUniformLocation(program_handle, "u_scale");
	u_amount = glGetUniformLocation(program_handle, "u_amount");
	u_strength = glGetUniformLocation(program_handle, "u_strength");


	glGenTextures(1, &image);
	glBindTexture(GL_TEXTURE_2D, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);   
	glGenTextures(1, &swap);
	glBindTexture(GL_TEXTURE_2D, swap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glGenTextures(1, &swap2);
	glBindTexture(GL_TEXTURE_2D, swap2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	return 0;
}


void Post::prelink(void)
{
	glBindAttribLocation(program_handle, 0, "attr_position");
	glBindAttribLocation(program_handle, 1, "attr_TexCoord");
}

/*
	Could be done in fragment shader, but this should be faster
*/
void Post::resize(int width, int height)
{
	float xInc = 1.0f / width;
    float yInc = 1.0f / height;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            texCoordOffsets[((i * 3 + j) * 2)] = (-1.0f * xInc) + ((float)i * xInc);
            texCoordOffsets[((i * 3 + j) * 2) + 1] = (-1.0f * yInc) + ((float)j * yInc);
        }
    }
}

void Post::Params(int type, int frame_step)
{
	glUniform1i(texture0, 0);
	glUniform1i(texture1, 1);
	glUniform1i(texture2, 2);
	glUniform2fv(tc_offset, 9, texCoordOffsets);
	glUniform1i(u_type, type);
	glUniform1i(u_time, frame_step);
}

void Post::BloomParams(int dir, float amount, float strength, float scale)
{
	glUniform1i(u_dir, dir);
	glUniform1f(u_amount, amount);
	glUniform1f(u_strength, strength);
	glUniform1f(u_scale, scale);
}

#ifdef NOPE
int mLight3::init(Graphics *gfx)
{
	//"media/glsl/mlighting3.gs"
#ifdef DIRECTX
	Shader::init(gfx, "media/hlsl/mlighting3.vsh", NULL, "media/hlsl/mlighting3.psh");
#else
#ifdef __OBJC__
	if (Shader::init(gfx, "media/glsl/ver410/mlight3.vs", "media/glsl/ver410/mlight3.gs", "media/glsl/ver410/mlight3.fs"))
	{
		program_handle = -1;
		return -1;
	}
#else
	if (Shader::init(gfx, "media/glsl/ver440/mlight3.vs", "media/glsl/ver440/mlight3.gs", "media/glsl/ver440/mlight3.fs"))
	{
		program_handle = -1;
		return -1;
	}
#endif

	matrix = glGetUniformLocation(program_handle, "mvp");
	texture0 = glGetUniformLocation(program_handle, "texture0");
	texture1 = glGetUniformLocation(program_handle, "texture1");
	texture2 = glGetUniformLocation(program_handle, "texture2");

	texture3 = glGetUniformLocation(program_handle, "shadowtex0");
	texture4 = glGetUniformLocation(program_handle, "shadowtex1");
	texture5 = glGetUniformLocation(program_handle, "shadowtex2");
	texture6 = glGetUniformLocation(program_handle, "shadowtex3");
	texture7 = glGetUniformLocation(program_handle, "shadowtex4");
	texture8 = glGetUniformLocation(program_handle, "shadowtex5");


	u_num_lights = glGetUniformLocation(program_handle, "u_num_lights");
	u_position = glGetUniformLocation(program_handle, "u_position");
	u_color = glGetUniformLocation(program_handle, "u_color");

#endif
	return 0;
}

void mLight3::prelink()
{
#ifndef DIRECTX
	glBindAttribLocation(program_handle, 0, "attr_position");
	glBindAttribLocation(program_handle, 1, "attr_TexCoord");
	glBindAttribLocation(program_handle, 2, "attr_LightCoord");
	glBindAttribLocation(program_handle, 3, "attr_normal");
	glBindAttribLocation(program_handle, 4, "attr_color");
	glBindAttribLocation(program_handle, 5, "attr_tangent");
#endif
}


void mLight3::Params(matrix4 &mvp, vector<Light *> &light_list, size_t num_lights)
{
	vec3 position[MAX_LIGHTS];
	vec4 color[MAX_LIGHTS];
	unsigned int i, j;

	for (i = 0, j = 0; i < num_lights && j < MAX_LIGHTS; i++, j++)
	{
		if (light_list[i]->entity->light->active)
		{
			position[j] = light_list[i]->entity->position;
			color[j] = vec4(light_list[i]->color.x, light_list[i]->color.y,
				light_list[i]->color.z, (float)(light_list[i]->intensity / 500.0f));
		}
	}

#ifdef DIRECTX
	uniform->SetMatrix(gfx->device, "mvp", (D3DXMATRIX *)mvp.m);
	uniform->SetFloatArray(gfx->device, "u_position", (float *)position, num_lights);
	uniform->SetFloatArray(gfx->device, "u_color", (float *)color, num_lights);
	uniform->SetInt(gfx->device, "u_num_lights", num_lights);
#else
	glUniformMatrix4fv(matrix, 1, GL_FALSE, mvp.m);
	glUniform1i(texture0, 0);
	glUniform1i(texture1, 1);
	glUniform1i(texture2, 2);
	glUniform1i(texture3, 3);
	glUniform1i(texture4, 4);
	glUniform1i(texture5, 5);
	glUniform1i(texture6, 6);
	glUniform1i(texture7, 7);
	glUniform1i(texture8, 8);

	glUniform1i(u_num_lights, j);
	glUniform3fv(u_position, j, (float *)&position);
	glUniform4fv(u_color, j, (float *)&color);
#endif
}

int ShadowMap::init(Graphics *gfx)
{
#ifndef DIRECTX

#ifdef __OBJC__
	if (Shader::init(gfx, "media/glsl/ver410/shadow.vs", NULL, "media/glsl/ver410/shadow.fs"))
	{
		program_handle = -1;
		return -1;
	}
#else
	if (Shader::init(gfx, "media/glsl/ver440/shadow.vs", NULL, "media/glsl/ver440/shadow.fs"))
	{
		program_handle = -1;
		return -1;
	}
#endif

	matrix = glGetUniformLocation(program_handle, "mvp");
	shadowmatrix = glGetUniformLocation(program_handle, "shadowmvp");
	shadowmap = glGetUniformLocation(program_handle, "ShadowMap");
	u_color = glGetUniformLocation(program_handle, "color");
#endif
	return 0;
}

void ShadowMap::prelink()
{
#ifndef DIRECTX
	glBindAttribLocation(program_handle, 0, "attr_position");
	glBindAttribLocation(program_handle, 1, "attr_TexCoord");
	glBindAttribLocation(program_handle, 2, "attr_LightCoord");
	glBindAttribLocation(program_handle, 3, "attr_normal");
	glBindAttribLocation(program_handle, 4, "attr_color");
	glBindAttribLocation(program_handle, 5, "attr_tangent");
#endif
}


void ShadowMap::Params(matrix4 &mvp, matrix4 &shadowmvp)
{
#ifdef DIRECTX
//	uniform->SetMatrix(gfx->device, "mvp", (D3DXMATRIX *)mvp.m);
//	uniform->SetFloatArray(gfx->device, "u_position", (float *)position, num_lights);
//	uniform->SetFloatArray(gfx->device, "u_color", (float *)color, num_lights);
//	uniform->SetInt(gfx->device, "u_num_lights", num_lights);
#else
	glUniformMatrix4fv(matrix, 1, GL_FALSE, mvp.m);
	glUniformMatrix4fv(shadowmatrix, 1, GL_FALSE, shadowmvp.m);
	glUniform1i(shadowmap, 4);
#endif
}
#endif


int ParticleUpdate::init(Graphics *gfx)
{
	if (Shader::init(gfx,
		"media/glsl/ver440/particle_update.vs",
		"media/glsl/ver440/particle_update.gs",
		NULL))
	{
		program_handle = -1;
		return -1;
	}

	u_emit_position = glGetUniformLocation(program_handle, "u_emit_position");
	u_emit_vel_min = glGetUniformLocation(program_handle, "u_emit_vel_min");
	u_emit_vel_range = glGetUniformLocation(program_handle, "u_emit_vel_range");
	u_emit_color = glGetUniformLocation(program_handle, "u_emit_color");
	u_emit_size = glGetUniformLocation(program_handle, "u_emit_size");
	u_emit_life_min = glGetUniformLocation(program_handle, "u_emit_life_min");
	u_emit_life_range = glGetUniformLocation(program_handle, "u_emit_life_range");
	u_emit_num = glGetUniformLocation(program_handle, "u_emit_num");

	u_gravity = glGetUniformLocation(program_handle, "u_gravity");
	u_delta_time = glGetUniformLocation(program_handle, "u_delta_time");
	u_seed = glGetUniformLocation(program_handle, "u_seed");

	glGenQueries(1, &query);



	vertex_t data[] = {
		{ vec3(1.0f, 1.0f, 1.0f), vec2(1.0f, 1.0f), vec2(1.0f, 1.0f), vec3(1.0f, 0.0f, 1.0f), 1, vec4(1.0f, 1.0f, 1.0f, 1.0f) },
	};

	memset(particle, 0, sizeof(particle));
	memcpy(particle, data, sizeof(data));

	glGenBuffers(1, &ParticleBufferA);
	glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferA);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * ParticleUpdate::max_particles, 0, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * 5, &particle[0]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ParticleBufferB);
	glBindBuffer(GL_ARRAY_BUFFER, ParticleBufferB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * ParticleUpdate::max_particles, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return 0;
}


void ParticleUpdate::prelink(void)
{
	const GLchar* feedbackVaryings[] = {
		"vary_position",
		"vary_TexCoord",
		"vary_LightCoord",
		"vary_velocity",
		"vary_color",
		"vary_tangent"
	};
	glTransformFeedbackVaryings(program_handle, 6, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

	glBindAttribLocation(program_handle, 0, "attr_position");
	glBindAttribLocation(program_handle, 1, "attr_TexCoord");
	glBindAttribLocation(program_handle, 2, "attr_LightCoord");
	glBindAttribLocation(program_handle, 3, "attr_velocity");
	glBindAttribLocation(program_handle, 4, "attr_color");
	glBindAttribLocation(program_handle, 5, "attr_tangent");
}

void ParticleUpdate::Params(emitter_t &emit)
{
	glUniform3fv(u_emit_position, 1, (float *)&(emit.position));
	glUniform3fv(u_emit_vel_min, 1, (float *)&(emit.vel_min));
	glUniform3fv(u_emit_vel_range, 1, (float *)&(emit.vel_range));
	glUniform1i(u_emit_color, emit.color);
	glUniform1f(u_emit_size, emit.size);
	glUniform1f(u_emit_life_min, emit.life_min);
	glUniform1f(u_emit_life_range, emit.life_range);
	glUniform1i(u_emit_num, emit.num);
	glUniform3fv(u_gravity, 1, (float *)&(emit.gravity));

	glUniform1f(u_delta_time, emit.delta_time);
	glUniform3fv(u_seed, 1, (float *)&(emit.seed));
	//emitter = emit;
}

int ParticleUpdate::step(Graphics &gfx, emitter_t &emit)
{
	if (emit.num > max_particles)
		emit.num = max_particles;

	if (max_particles == 0)
		return 0;

	// Source buffer (input)
	gfx.SelectVertexBuffer(ParticleBufferA);

	// Dont rasterize
	glEnable(GL_RASTERIZER_DISCARD);

	// Target Buffer (output)
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, ParticleBufferB);

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, emit.num);
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glDisable(GL_RASTERIZER_DISCARD);
	glFlush();

	// Swap buffers
	int temp = ParticleBufferA;
	ParticleBufferA = ParticleBufferB;
	ParticleBufferB = temp;

	// Get num primitives generated
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &num_particle);

	if (num_particle == 0 && max_particles != 0)
	{
		printf("ParticleSystem Error: %u particles written!\n\n", num_particle);
	}

	//emitter.num = num_particle;
	emit.num = num_particle;
	return ParticleBufferA;
}

#define QUAD_PARTICLES

int ParticleRender::init(Graphics *gfx)
{

#ifdef QUAD_PARTICLES
	// quad particles
	if (Shader::init(gfx,
		"media/glsl/ver440/particle_render.vs",
		"media/glsl/ver440/particle_render.gs",
		"media/glsl/ver440/particle_render.fs"))
	{
		program_handle = -1;
		return -1;
	}
#else
	// point textured particles
	if (Shader::init(gfx,
		"media/glsl/ver440/particle_render2.vs",
		NULL,
		"media/glsl/ver440/particle_render2.fs"))
	{
		program_handle = -1;
		return -1;
	}
#endif

	u_mvp = glGetUniformLocation(program_handle, "u_mvp");
	u_quad1 = glGetUniformLocation(program_handle, "u_quad1");
	u_quad2 = glGetUniformLocation(program_handle, "u_quad2");
	u_texture0 = glGetUniformLocation(program_handle, "u_texture0");
	u_xshift = glGetUniformLocation(program_handle, "u_xshift");
	u_yshift = glGetUniformLocation(program_handle, "u_yshift");
	u_zshift = glGetUniformLocation(program_handle, "u_zshift");

	unsigned int *index = new unsigned int [ParticleUpdate::max_particles];
	for (unsigned int i = 0; i < ParticleUpdate::max_particles; i++)
	{
		index[i] = i;
	}
	ibo = gfx->CreateIndexBuffer(index, ParticleUpdate::max_particles);
	delete [] index;
	index = NULL;
	return 0;
}


void ParticleRender::prelink(void)
{
	glBindAttribLocation(program_handle, 0, "attr_position");
	glBindAttribLocation(program_handle, 1, "attr_TexCoord");
	glBindAttribLocation(program_handle, 2, "attr_LightCoord");
	glBindAttribLocation(program_handle, 3, "attr_velocity");
	glBindAttribLocation(program_handle, 4, "attr_color");
	glBindAttribLocation(program_handle, 5, "attr_tangent");
}

void ParticleRender::Params(matrix4 &mvp, vec3 &quad1, vec3 &quad2, float x, float y, float z)
{
	glUniformMatrix4fv(u_mvp, 1, GL_FALSE, mvp.m);
	glUniform3fv(u_quad1, 1, (float *)&quad1);
	glUniform3fv(u_quad2, 1, (float *)&quad2);
	glUniform1f(u_xshift, x);
	glUniform1f(u_yshift, y);
	glUniform1f(u_zshift, z);
	glUniform1i(u_texture0, 0);
}

void ParticleRender::render(Graphics &gfx, int start, int vbo, int num)
{
//	gfx.Blend(true);
	gfx.Depth(false);
//	gfx.BlendFunc(NULL, NULL);
	gfx.SelectVertexBuffer(vbo);
	gfx.SelectIndexBuffer(ibo);
	gfx.DrawArrayPoint(start, 0, num, num);
	gfx.Depth(true);
//	gfx.Blend(false);
}


int ScreenSpace::init(Graphics *gfx)
{
	if (Shader::init(gfx,
		"media/glsl/ver440/screen_space.vs",
		NULL,
		"media/glsl/ver440/screen_space.fs"))
	{
		program_handle = -1;
		return -1;
	}


	/*
	uniform float ssao_level = 1.0;
	uniform float object_level = 1.0;
	uniform float ssao_radius = 5.0;
	uniform bool weight_by_angle = true;
	uniform uint point_count = 8;
	uniform bool randomize_points = true;

	*/

	u_radius = glGetUniformLocation(program_handle, "ssao_radius");
	u_ssaolevel = glGetUniformLocation(program_handle, "ssao_level");
	u_objectlevel = glGetUniformLocation(program_handle, "object_level");
	u_randomize_points = glGetUniformLocation(program_handle, "randomize_points");
	u_point_count = glGetUniformLocation(program_handle, "point_count");
	u_width = glGetUniformLocation(program_handle, "u_width");
	u_height = glGetUniformLocation(program_handle, "u_height");


	int i;
	struct SAMPLE_POINTS
	{
		vec4     point[256];
		vec4     random_vectors[256];
	};

	SAMPLE_POINTS point_data;

	for (i = 0; i < 256; i++)
	{
		do
		{
			point_data.point[i].x = random_float() * 2.0f - 1.0f;
			point_data.point[i].y = random_float() * 2.0f - 1.0f;
			point_data.point[i].z = random_float(); //  * 2.0f - 1.0f;
			point_data.point[i].w = 0.0f;
		} while (point_data.point[i].magnitude() > 1.0f);
		point_data.point[i].normalize();
	}

	for (i = 0; i < 256; i++)
	{
		point_data.random_vectors[i].x = random_float();
		point_data.random_vectors[i].y = random_float();
		point_data.random_vectors[i].z = random_float();
		point_data.random_vectors[i].w = random_float();
	}

	glGenBuffers(1, &points_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, points_buffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SAMPLE_POINTS), &point_data, GL_STATIC_DRAW);

	return 0;
}

void ScreenSpace::prelink(void)
{
	glBindAttribLocation(program_handle, 0, "attr_position");
	glBindAttribLocation(program_handle, 1, "attr_TexCoord");
	glBindAttribLocation(program_handle, 2, "attr_LightCoord");
	glBindAttribLocation(program_handle, 3, "attr_velocity");
	glBindAttribLocation(program_handle, 4, "attr_color");
	glBindAttribLocation(program_handle, 5, "attr_tangent");
}


void ScreenSpace::Params(float radius, float objectlevel, float ssaolevel, bool show_ao, bool randomize_points, int point_count, int width, int height)
{
	glUniform1f(u_radius, radius * ((width) / 1000.0f));
	glUniform1f(u_objectlevel, objectlevel);
	glUniform1f(u_ssaolevel, ssaolevel);
	glUniform1i(u_randomize_points, randomize_points);
	glUniform1ui(u_point_count, point_count);
	glUniform1ui(u_width, width);
	glUniform1ui(u_height, height);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, points_buffer);
}



#endif
