#include "shader.h"
#include <cmath> // for some sin/cos/tan funcs

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifdef D3D9
//unsigned int ParticleUpdate::max_particles = 175000;

// Setting to a low value to keep FPS up
unsigned int ParticleUpdate::max_particles = 5000;

int mFont::init(Graphics *gfx)
{
	Shader::init(gfx, "media/hlsl/font.vsh", NULL, "media/hlsl/font.psh");
	return 0;
}

void mFont::prelink()
{
}

void mFont::Params(char c, float x, float y, float scale, vec3 &color)
{
	int col = c % 16;
	int row = 15 - c / 16;
	float offset = 16.0f / 256.0f;

	uniform_vs->SetFloat(gfx->device, "u_col", col * offset);
	uniform_vs->SetFloat(gfx->device, "u_row", row * offset);
	uniform_vs->SetFloat(gfx->device, "u_xpos", (2.0f * x));
	uniform_vs->SetFloat(gfx->device, "u_ypos", (2.0f * y));
	uniform_vs->SetFloat(gfx->device, "u_scale", scale);
	uniform_ps->SetFloatArray(gfx->device, "u_color", (float *)&color, 3);
}

int Global::init(Graphics *gfx)
{
	Shader::init(gfx, "media/hlsl/basic.vsh", NULL, "media/hlsl/basic.psh");
	return 0;
}

void Global::prelink()
{
}

void Global::Params(matrix4 &mvp, int tex0)
{
	uniform_vs->SetMatrix(gfx->device, "mvp", (D3DXMATRIX *)mvp.m);
}

int mLight2::init(Graphics *gfx)
{
	max_light = MAX_LIGHTS;
	//"media/glsl/mlighting3.gs"
	Shader::init(gfx, "media/hlsl/mlighting3.vsh", NULL, "media/hlsl/mlighting3.psh");
	return 0;
}

void mLight2::set_max(int max)
{
	max_light = max;
}

void mLight2::set_ambient(float ambient)
{
	m_ambient = ambient;
}

void mLight2::set_lightmap(float lightmap)
{
	m_lightmap = lightmap;
}




void mLight2::prelink()
{
}


void mLight2::Params(matrix4 &mvp, vector<Light *> &light_list, size_t num_lights, vec3 &offset, int u_time)
{
	vec4 position[MAX_LIGHTS];
	vec4 color[MAX_LIGHTS];
	unsigned int i, j;

	for(i = 0, j = 0; i < num_lights && j < MAX_LIGHTS; i++, j++)
	{
		if (light_list[i]->active)
		{
			vec3 pos = light_list[i]->entity->position - offset;

			position[j] = vec4(pos.x, pos.y, pos.z, light_list[i]->attenuation);


			color[j] = vec4(light_list[i]->color.x,
							light_list[i]->color.y,
							light_list[i]->color.z,
					(float)(light_list[i]->intensity / 500.0f));
		}
	}

	HRESULT ret = S_OK;

	ret = uniform_vs->SetMatrix(gfx->device, "mvp", (D3DXMATRIX *)mvp.m);
	if (FAILED(ret))
	{
		printf("Error: %s error description: %s\n",
			DXGetErrorString(ret), DXGetErrorDescription(ret));
	}

	ret = uniform_vs->SetVectorArray(gfx->device, "u_position", (D3DXVECTOR4 *)position, MAX_LIGHTS);
	if (FAILED(ret))
	{
		printf("Error: %s error description: %s\n",
			DXGetErrorString(ret), DXGetErrorDescription(ret));
	}
	ret = uniform_vs->SetVectorArray(gfx->device, "u_color", (D3DXVECTOR4 *)color, MAX_LIGHTS);
	if (FAILED(ret))
	{
		printf("Error: %s error description: %s\n",
			DXGetErrorString(ret), DXGetErrorDescription(ret));
	}
	ret = uniform_vs->SetInt(gfx->device, "u_num_lights", num_lights);
	if (FAILED(ret))
	{
		printf("Error: %s error description: %s\n",
			DXGetErrorString(ret), DXGetErrorDescription(ret));
	}
}

void mLight2::set_light(float ambient, float lightmap, int num_light)
{
}


void mLight2::set_lightmap_stage(int flag)
{
}


void mLight2::envmap(int stage, int env)
{

}

void mLight2::rgbgen_scale(int stage, float scale)
{

}

void mLight2::turb(int stage, int turb)
{

}

void mLight2::tcmod_scroll(vec2 &scroll, int index)
{

}

void mLight2::tcmod_scale(vec2 &scale, int index)
{

}



void mLight2::tcmod_rotate(float deg, int index)
{

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
	float x = (float)(0.5 * tick_num * freq + phase);
	value.x = (float)( amplitude * (1.0 - 2.0 * abs32(sign(x) - x)) );
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
	float x = (float)(0.5 * tick_num / 10.0f * freq + phase);
	value = (float)(amplitude * (1.0 - 2.0 * abs32(sign(x) - x)));

	rgbgen_scale(index, value);
}

void mLight2::rgbgen_wave_square(float amplitude, float phase, float freq, int tick_num, int index)
{
	float value;

	if (abs32((float)(freq - 0.001)) <= 0.0011)
		freq = 0.1f;

	value = (float)(amplitude * (4.0 / ((int)(freq * tick_num / 10.0f + phase) % 2 * MY_PI)));

	rgbgen_scale(index, value);
}

int Post::init(Graphics *gfx)
{
	Shader::init(gfx, "media/hlsl/post.vsh", NULL, "media/hlsl/post.psh");
	return 0;
}


void Post::prelink(void)
{
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

void Post::Params(int tex0, int tex1)
{
	uniform_ps->SetFloatArray(gfx->device, "tc_offset", texCoordOffsets, 9);
}

int ParticleUpdate::init(Graphics *gfx)
{

	return 0;
}


void ParticleUpdate::prelink(void)
{

}

void ParticleUpdate::Params(emitter_t &emit)
{

}

int ParticleUpdate::step(Graphics &gfx, emitter_t &emit)
{
	return ParticleBufferA;
}

#define QUAD_PARTICLES

int ParticleRender::init(Graphics *gfx)
{
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
}

void ParticleRender::Params(matrix4 &mvp, vec3 &quad1, vec3 &quad2, float x, float y)
{
}

void ParticleRender::render(Graphics &gfx, int start, int vbo, int num)
{
}
#endif