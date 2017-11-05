
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
#define NUM_STAGE 4

class mLight2 : public Shader
{
public:
	int init(Graphics *gfx);
	void Select();
	void Params(matrix4 &mvp, vector<Light *> &light_list, size_t num_lights, vec3 &offset, int u_time);
	void set_shadow_matrix(int index, matrix4 &mvp);
	void set_matrix(matrix4 &mvp);
	virtual void prelink(void);
	void set_max(int max);
	void set_brightness(float value);
	void set_exposure(float value);
	void set_contrast(float value);	
	void set_ambient(float ambient);
	void set_lightmap(float lightmap);
	void set_shadowmap(float value);
	void set_light(float ambient, float lightmap, int num_light);
	void set_lightmap_stage(int flag);
	void set_depth(int flag);
	void set_normalmap(int value);
	void set_alpha(float value);
	void set_tone(int value);

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

	void set_fog(float fog, float start, float end, vec3 &color);
	void set_clip(vec4 *plane);


	void tcmod_scroll(vec2 &scroll, int index);
	void tcmod_scale(vec2 &scale, int index);
	void tcmod_rotate(float deg, int index);


	void alphatest(int stage, int mode);
	void envmap(int stage, int env);
	void turb(int stage, int turb);
	void rgbgen_scale(int stage, float scale);
	void portal(int portal);
	


	int m_num_light;
	float m_ambient;
	float m_brightness;
	float m_exposure;
	float m_lightmap;
	float m_shadowmap;
	float m_contrast;
	float m_alpha;
	int m_tone;
private:
	int matrix;

	int	texture[NUM_STAGE];
	int texture_normalmap;
	int texture_lightmap;

	int u_tcmod_scroll[NUM_STAGE];
	int u_tcmod_scale[NUM_STAGE];
	int u_tcmod_sin[NUM_STAGE];
	int u_tcmod_cos[NUM_STAGE];
	int u_env[NUM_STAGE];
	int u_water[NUM_STAGE];
	int u_rgbgen_scale[NUM_STAGE];
	int u_alphatest[NUM_STAGE];
	int u_portal;
	int u_normalmap;
	int u_alpha;


	int shadow_matrix[18];
	int depth[18];


	int u_time;
	int u_ambient;
	int u_lightmap;
	int u_brightness;
	int u_exposure;
	int u_contrast;
	int u_shadowmap;
	int u_lightmap_stage;
	int u_depth;
	int u_num_lights;
	int u_position;
	int u_color;

	int u_tone;


	int u_fog;
	int u_fog_start;
	int u_fog_end;
	int u_fog_color;


	int u_clip0;
	int u_clip1;
	int u_clip2;
	int u_clip3;



	float m_fog;
	vec3 m_color;
	float m_start;
	float m_end;


	int m_normalmap;
	unsigned int max_light;
};

#define POST_EDGE		0
#define POST_BLUR		1
#define POST_EMBOSS		2
#define POST_BLOOM		3
#define POST_COMBINE	4
#define POST_MASK		5
#define POST_RAY		6
#define POST_RADIAL		7
#define POST_GRADING	8
#define POST_WAVE		9
#define POST_HSV		10
#define POST_DOF		11


class Post : public Shader
{
public:
	int init(Graphics *gfx);
	void resize(int width, int height);
	void Params(int type, int framestep);
	void BloomParams(int dir, float amount, float strength, float scale, int xres, int yres);

	virtual void prelink(void);

	unsigned int image, swap, swap2;
private:
	int texture0;
	int texture1;
	int texture2;

	int tc_offset;
	int u_type;
	int u_time;

	int u_dir;
	int u_xres;
	int u_yres;

	int u_amount;
	int u_strength;
	int u_scale;
	float texCoordOffsets[18];
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
	void Params(matrix4 &mvp, vec3 &quad1, vec3 &quad2, float x, float y, float z);
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
	int u_zshift;
};



class ScreenSpace : public Shader
{
public:
	int init(Graphics *gfx);
	void Params(float radius, float objectlevel, float ssaolevel, bool show_ao, bool randomize_points, int point_count, int width, int height);
	virtual void prelink(void);

private:
	int u_radius;
	int u_objectlevel;
	int u_ssaolevel;
	int u_randomize_points;
	int u_point_count;
	int u_width;
	int u_height;

	unsigned int points_buffer;
};

#endif
