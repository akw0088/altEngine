
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
	void Params(matrix4 &mvp, int tex0);
	virtual void prelink(void);
private:
	int matrix;
	int	texture0;
};



// Max lights hardware can pass through shaders, gfx card specfic
#define MAX_LIGHTS 64

class mLight2 : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(matrix4 &mvp, vector<Light *> &light_list, size_t num_lights, vec3 &offset);
	virtual void prelink(void);

	//sin tri square saw inverse saw
	//tcMod stretch <func> <base> <amplitude> <phase> <frequency>
	void tcmod_stretch_sin(float amplitude, float phase, float freq, int tick_num, int index);
	void tcmod_stretch_sawtooth(float amplitude, float phase, float freq, int tick_num, int index);
	void tcmod_stretch_inverse_sawtooth(float amplitude, float phase, float freq, int tick_num, int index);
	void tcmod_stretch_triangle(float amplitude, float phase, float freq, int tick_num, int index);
	void tcmod_stretch_square(float amplitude, float phase, float freq, int tick_num, int index);

	void tcmod_scroll(vec2 &scroll, int index);
	void tcmod_scale(vec2 &scale, int index);
	void tcmod_rotate(float deg, int index);


private:
	int matrix;
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


	int u_num_lights;
	int u_position;
	int u_color;
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

#define MAX_PARTICLES 100

class ParticleUpdate : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(generator_t &gen);
	int step(Graphics &gfx, generator_t &gen);

	virtual void prelink(void);


	unsigned int vbo; // geometry shader input
	unsigned int tbo; // geometry shader output

private:
	int u_gen_position;
	int u_gen_vel_min;
	int u_gen_vel_range;
	int u_gen_color;
	int u_gen_size;
	int u_gen_life_min;
	int u_gen_life_range;
	int u_gen_num;

	int u_gravity;
	int u_delta_time;
	int u_seed;

	// non uniform

	vertex_t	particle[MAX_PARTICLES];
	unsigned int			num_particle;

	unsigned int query;
	generator_t generator;
};

class ParticleRender : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(matrix4 &mvp, vec3 &quad1, vec3 &quad2);
	void render(Graphics &gfx, int vbo, int num);

	virtual void prelink(void);

	int ibo;
private:
	int u_mvp;
	int u_quad1;
	int u_quad2;
	int u_texture0;

	// non uniform

	generator_t generator;
};

#endif
