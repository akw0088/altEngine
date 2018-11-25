//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#include "shader.h"
//#include <cmath> // for some sin/cos/tan funcs

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#ifdef D3D11
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
}

int Global::init(Graphics *gfx)
{
	Shader::init(gfx, "media/hlsl/basic.vsh", NULL, "media/hlsl/basic.psh");
	return 0;
}

Global::~Global()
{
}

void Global::prelink()
{
}

void Global::Params(matrix4 &mvp, int depth)
{
	ID3D11Buffer*   g_pConstantBuffer11 = NULL;
	HRESULT ret;

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(mvp);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &(mvp.m);
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	// Create the buffer.
	ret = gfx->device->CreateBuffer(&cbDesc, &InitData,
		&g_pConstantBuffer11);

	if (FAILED(ret))
		return;

	// Set the buffer.
	gfx->context->VSSetConstantBuffers(0, 1, &g_pConstantBuffer11);
}

int mLight2::init(Graphics *gfx, bool pixel)
{
//	max_light = MAX_LIGHTS;
	//"media/glsl/mlighting3.gs"
	Shader::init(gfx, "media/hlsl/mlighting3.vsh", NULL, "media/hlsl/mlighting3.psh");
	return 0;
}

void mLight2::set_normalmap(int value)
{
}


void mLight2::set_shadow_matrix(int index, matrix4 &proj)
{
}

void mLight2::set_brightness(float value)
{
}

void mLight2::set_dissolve(float value)
{
}


void mLight2::set_shadowmap(float value)
{
}

void mLight2::set_contrast(float value)
{
}

void mLight2::set_matrix(matrix4 &matrix)
{
	return;
}

void mLight2::set_num_shadowmap(int value)
{

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

	ID3D11Buffer*   constant_buffer = NULL;
	HRESULT ret;


	typedef struct
	{
		matrix4 mvp;
	} vertex_constant_t;

	vertex_constant_t data;

	data.mvp = mvp;

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(vertex_constant_t);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &data;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	// Create the buffer.
	ret = gfx->device->CreateBuffer(&cbDesc, &InitData,	&constant_buffer);

	if (FAILED(ret))
		return;

	// Set the buffer.
	gfx->context->VSSetConstantBuffers(0, 1, &constant_buffer);
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

	// for jumppad aesthetics
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

void mLight2::set_alpha(float value)
{
}

void mLight2::portal(int value)
{
}

void mLight2::Select()
{
}



void mLight2::set_tone(int tone)
{

}

void mLight2::alphatest(int value, int p)
{
}


void mLight2::set_exposure(float value)
{

}

void mLight2::set_clip(vec4 *plane)
{

}


void mLight2::set_fog(float fog, float start, float end, vec3 &color)
{

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
}

void Post::BloomParams(int dir, float amount, float strength, float scale, int xres, int yres)
{
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

void ParticleRender::Params(matrix4 &mvp, vec3 &quad1, vec3 &quad2, float x, float y, float z)
{
}

void ParticleRender::render(Graphics &gfx, int start, int vbo, int num)
{
}

int ScreenSpace::init(Graphics *gfx)
{
	return 0;
}


void ScreenSpace::prelink(void)
{

}

void ScreenSpace::Params(float radius, float objectlevel, float ssaolevel, bool randomize_points, int point_count, int width, int height)
{

}

#endif
