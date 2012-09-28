#include "include.h"

#ifndef SHADER_H
#define SHADER_H

class Font : public Shader
{
public:
	void init(Graphics *gfx);
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
	void init(Graphics *gfx);
	void Params(matrix4 &mvp, int tex0);
	virtual void prelink(void);
private:
	int matrix;
	int	texture0;
};

#define MAX_LIGHTS 24

class mLight2 : public Shader
{
public:
	void init(Graphics *gfx);
	void Params(matrix4 &mvp, int tex0, int tex1, int tex2, vector<Light *> &light_list, size_t num_lights);
	virtual void prelink(void);

private:
	int matrix;
	int	texture0;
	int texture1;
	int texture2;
	int u_num_lights;
	int u_position;
	int u_color;
	int u_intensity;
};

class Post : public Shader
{
public:
	void init(Graphics *gfx);
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

#endif