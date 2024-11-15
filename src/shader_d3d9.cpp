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

void Global::Params(matrix4 &mvp, int depth)
{
	uniform_vs->SetMatrix(gfx->device, "mvp", (D3DXMATRIX *)mvp.m);
}

Global::~Global()
{

}

int mLight2::init(Graphics *gfx, bool pixel)
{
//	max_light = MAX_LIGHTS;
	//"media/glsl/mlighting3.gs"
	Shader::init(gfx, "media/hlsl/mlighting3.vsh", NULL, "media/hlsl/mlighting3.psh");
	return 0;
}

void mLight2::set_shadow_matrix(int index, matrix4 &matrix)
{
	return;
}

void mLight2::Select()
{
	Shader::Select();
}

void mLight2::set_tone(int tone)
{

}

void mLight2::set_num_shadowmap(int value)
{

}

void mLight2::set_fog(float fog, float start, float end, vec3 &color)
{

}


void mLight2::set_matrix(matrix4 &matrix)
{
	return;
}

void mLight2::set_exposure(float value)
{
	return;
}


void mLight2::set_max(int max)
{
	max_light = max;
}

void mLight2::set_ambient(float ambient)
{
	m_ambient = ambient;
}

void mLight2::set_brightness(float value)
{
	m_brightness = value;
}

void mLight2::set_contrast(float value)
{
	m_contrast = value;
}

void mLight2::set_shadowmap(float value)
{
	m_shadowmap = value;
}


void mLight2::set_dissolve(float value)
{
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

	ret = uniform_ps->SetVectorArray(gfx->device, "u_position", (D3DXVECTOR4 *)position, MAX_LIGHTS);
	if (FAILED(ret))
	{
		static int once = 0;

		if (once)
		{
			printf("Failed to set u_position: %s error description: %s\n",
				DXGetErrorString(ret), DXGetErrorDescription(ret));
			once = 1;
		}
	}
	ret = uniform_ps->SetVectorArray(gfx->device, "u_color", (D3DXVECTOR4 *)color, MAX_LIGHTS);
	if (FAILED(ret))
	{
		static int once = 0;

		if (once)
		{
			printf("Failed to set u_color: %s error description: %s\n",
				DXGetErrorString(ret), DXGetErrorDescription(ret));
			once = 1;
		}

	}
	ret = uniform_ps->SetInt(gfx->device, "u_num_lights", num_lights);
	if (FAILED(ret))
	{
		static int once = 0;

		if (once)
		{
			printf("Failed to set u_num_lights: %s error description: %s\n",
				DXGetErrorString(ret), DXGetErrorDescription(ret));
			once = 1;
		}

	}

	D3DXHANDLE tex0c = uniform_ps->GetConstantByName(NULL, "tex0");
	int tex0 = uniform_ps->GetSamplerIndex(tex0c);
	gfx->device->SetSamplerState(tex0, D3DSAMP_ELEMENTINDEX, 0);

	D3DXHANDLE tex1c = uniform_ps->GetConstantByName(NULL, "tex1");
	int tex1 = uniform_ps->GetSamplerIndex(tex1c);
	gfx->device->SetSamplerState(tex1, D3DSAMP_ELEMENTINDEX, 1);

	D3DXHANDLE tex2c = uniform_ps->GetConstantByName(NULL, "tex2");
	int tex2 = uniform_ps->GetSamplerIndex(tex2c);
	gfx->device->SetSamplerState(tex2, D3DSAMP_ELEMENTINDEX, 2);

	D3DXHANDLE tex3c = uniform_ps->GetConstantByName(NULL, "tex3");
	int tex3 = uniform_ps->GetSamplerIndex(tex3c);
	gfx->device->SetSamplerState(tex3, D3DSAMP_ELEMENTINDEX, 3);

	D3DXHANDLE tex8c = uniform_ps->GetConstantByName(NULL, "texture_lightmap");
	int tex8 = uniform_ps->GetSamplerIndex(tex8c);
	gfx->device->SetSamplerState(tex8, D3DSAMP_ELEMENTINDEX, 8);

	D3DXHANDLE tex9c = uniform_ps->GetConstantByName(NULL, "texture_normalmap");
	int tex9 = uniform_ps->GetSamplerIndex(tex9c);
	gfx->device->SetSamplerState(tex9, D3DSAMP_ELEMENTINDEX, 9);


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

void mLight2::portal(int value)
{
}

void mLight2::set_alpha(float value)
{
}

void  mLight2::set_normalmap(float value)
{
}

void  mLight2::set_diffuse_min(float value)
{
}

void  mLight2::set_normalmap_scale(vec3 &value)
{
}

void  mLight2::set_specular_min(float value)
{
}


void mLight2::set_atten_exponent(float value)
{
}

void mLight2::set_atten_scale(float value)
{
}

void mLight2::set_atten_min(float value)
{
}

void  mLight2::set_specular_exponent(float value)
{
}

void  mLight2::set_specular_factor(float value)
{
}

void mLight2::set_diffuse_factor(float value)
{
}


void mLight2::alphatest(int value, int p)
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

void Post::Params(int type, int framestep)
{
	uniform_ps->SetFloatArray(gfx->device, "tc_offset", texCoordOffsets, 9);
}

void Post::BloomParams(int dir, float amount, float strength, float scale, int width, int height)
{

}

int ScreenSpace::init(Graphics *gfx)
{
//	Shader::init(gfx, "media/hlsl/post.vsh", NULL, "media/hlsl/post.psh");
	return 0;
}


void ScreenSpace::prelink(void)
{
}

void ScreenSpace::Params(float radius, float objectlevel, float ssaolevel, bool randomize_points, int point_count, int width, int height)
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
#endif
