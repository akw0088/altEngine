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
	void tcmod_stretch_sin(float amplitude, float phase, float freq, int tick_num);
	void tcmod_stretch_sawtooth(float amplitude, float phase, float freq, int tick_num);
	void tcmod_stretch_inverse_sawtooth(float amplitude, float phase, float freq, int tick_num);
	void tcmod_stretch_triangle(float amplitude, float phase, float freq, int tick_num);
	void tcmod_stretch_square(float amplitude, float phase, float freq, int tick_num);

	void tcmod_scroll(vec2 &scroll);
	void tcmod_scale(vec2 &scale);
	void tcmod_rotate(float deg);


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

	int u_tcmod_scroll;
	int u_tcmod_scale;
	int u_tcmod_rotate;

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

#endif
