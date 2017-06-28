
#include "include.h"

#ifndef SHADER_H
#define SHADER_H

class mFont : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(char c, float x, float y, float scale, vec3 &color);
	virtual void prelink(void);
private:
	int u_scale;
	int u_xpos;
	int u_ypos;
	int u_row;
	int u_col;
	int u_color;
	int texture0;
};

class Global : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(matrix4 &mvp, int tex0, int depth = 0);
	virtual void prelink(void);
	virtual ~Global();
private:
	int matrix;
	int	texture0;
	int u_depth;
};



// Max lights hardware can pass through shaders, gfx card specfic
#define MAX_LIGHTS 64
#define AMBIENT_LIGHT 0.125f

class mLight2 : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(matrix4 &mvp, vector<Light *> &light_list, size_t num_lights, vec3 &offset, int u_time);
	void set_shadow_matrix(int index, matrix4 &mvp);
	virtual void prelink(void);
	void set_max(int max);
	void set_ambient(float ambient);
	void set_lightmap(float lightmap);
	void set_light(float ambient, float lightmap, int num_light);
	void set_lightmap_stage(int flag);
	void set_depth(int flag);

	//sin tri square saw inverse saw
	//tcMod stretch <func> <base> <amplitude> <phase> <frequency>
	void tcmod_stretch_sin(float amplitude, float phase, float freq, int tick_num, int index);
	void tcmod_stretch_sawtooth(float amplitude, float phase, float freq, int tick_num, int index);
	void tcmod_stretch_inverse_sawtooth(float amplitude, float phase, float freq, int tick_num, int index);
	void tcmod_stretch_triangle(float amplitude, float phase, float freq, int tick_num, int index);
	void tcmod_stretch_square(float amplitude, float phase, float freq, int tick_num, int index);

	void rgbgen_wave_sin(float amplitude, float phase, float freq, int tick_num, int index);
	void rgbgen_wave_sawtooth(float amplitude, float phase, float freq, int tick_num, int index);
	void rgbgen_wave_inverse_sawtooth(float amplitude, float phase, float freq, int tick_num, int index);
	void rgbgen_wave_triangle(float amplitude, float phase, float freq, int tick_num, int index);
	void rgbgen_wave_square(float amplitude, float phase, float freq, int tick_num, int index);



	void tcmod_scroll(vec2 &scroll, int index);
	void tcmod_scale(vec2 &scale, int index);
	void tcmod_rotate(float deg, int index);

	void envmap(int stage, int env);
	void turb(int stage, int turb);
	void rgbgen_scale(int stage, float scale);


	int m_num_light;
	float m_ambient;
	float m_lightmap;
private:
	int matrix;
	int shadow_matrix[17];
	int depth[17];

	int	texture0;
	int texture1;
	int texture2;
	int	texture3;
	int texture4;
	int texture5;
	int	texture6;
	int texture7;



	int texture_normalmap;
	int texture_lightmap;

	int u_tcmod_scroll0;
	int u_tcmod_scroll1;
	int u_tcmod_scroll2;
	int u_tcmod_scroll3;
	int u_tcmod_scroll4;
	int u_tcmod_scroll5;
	int u_tcmod_scroll6;
	int u_tcmod_scroll7;

	int u_tcmod_scale0;
	int u_tcmod_scale1;
	int u_tcmod_scale2;
	int u_tcmod_scale3;
	int u_tcmod_scale4;
	int u_tcmod_scale5;
	int u_tcmod_scale6;
	int u_tcmod_scale7;

	int u_tcmod_sin0;
	int u_tcmod_sin1;
	int u_tcmod_sin2;
	int u_tcmod_sin3;
	int u_tcmod_sin4;
	int u_tcmod_sin5;
	int u_tcmod_sin6;
	int u_tcmod_sin7;
	
	int u_tcmod_cos0;
	int u_tcmod_cos1;
	int u_tcmod_cos2;
	int u_tcmod_cos3;
	int u_tcmod_cos4;
	int u_tcmod_cos5;
	int u_tcmod_cos6;
	int u_tcmod_cos7;

	int u_env0;
	int u_env1;
	int u_env2;
	int u_env3;
	int u_env4;
	int u_env5;
	int u_env6;
	int u_env7;

	int u_water0;
	int u_water1;
	int u_water2;
	int u_water3;
	int u_water4;
	int u_water5;
	int u_water6;
	int u_water7;

	int u_time;

	int u_rgbgen_scale0;
	int u_rgbgen_scale1;
	int u_rgbgen_scale2;
	int u_rgbgen_scale3;
	int u_rgbgen_scale4;
	int u_rgbgen_scale5;
	int u_rgbgen_scale6;
	int u_rgbgen_scale7;


	int u_ambient;
	int u_lightmap;
	int u_lightmap_stage;
	int u_depth;
	int u_num_lights;
	int u_position;
	int u_color;

	unsigned int max_light;
};

class mLightDepth : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(matrix4 &mvp);
	virtual void prelink(void);

private:
	int matrix;
};

class mLight3 : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(matrix4 &mvp, vector<Light *> &light_list, size_t num_lights);
	virtual void prelink(void);

private:
	int matrix;
	int	texture0;
	int texture1;
	int texture2;

	int texture3;
	int texture4;
	int texture5;
	int texture6;
	int texture7;
	int texture8;



	int u_num_lights;
	int u_position;
	int u_color;
};

class Post : public Shader
{
public:
	int init(Graphics *gfx);
	void resize(int width, int height);
	void Params(int tex0, int tex1);

	virtual void prelink(void);

	unsigned int image, swap;
private:
	int texture0;
	int texture1;
	int tc_offset;
	float texCoordOffsets[18];
};



class ShadowMap : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(matrix4 &mvp, matrix4 &shadowmvp);
	virtual void prelink(void);

private:
	int matrix;
	int shadowmatrix;
	int u_color;
	int shadowmap;
};

class ParticleUpdate : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(emitter_t &emit);
	int step(Graphics &gfx, emitter_t &emit);

	virtual void prelink(void);

	unsigned int ParticleBufferA;
	unsigned int ParticleBufferB;
	static unsigned int max_particles;

private:
	int u_emit_position;
	int u_emit_vel_min;
	int u_emit_vel_range;
	int u_emit_color;
	int u_emit_size;
	int u_emit_life_min;
	int u_emit_life_range;
	int u_emit_num;

	int u_gravity;
	int u_delta_time;
	int u_seed;

	// non uniform

	// we dont store all particles, just generators to get it started, everything stays on GPU
	vertex_t		particle[5];

	// This count is *all* the particles
	unsigned int	num_particle;

	unsigned int query;
//	emitter_t emitter;
};

class ParticleRender : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(matrix4 &mvp, vec3 &quad1, vec3 &quad2, float x, float y);
	void render(Graphics &gfx, int start, int vbo, int num);

	virtual void prelink(void);

	int ibo;
private:
	int u_mvp;
	int u_quad1;
	int u_quad2;
	int u_texture0;
	int u_xshift;
	int u_yshift;
};

#endif
