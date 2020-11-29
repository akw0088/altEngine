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


//unsigned int ParticleUpdate::max_particles = 175000;

#ifdef SOFTWARE

// Setting to a low value to keep FPS up
unsigned int ParticleUpdate::max_particles = 5000;

int mFont::init(Graphics *gfx)
{
	return 0;
}

void mFont::prelink()
{

}

void mFont::Params(char c, float x, float y, float scale, vec3 &color)
{

}

int Global::init(Graphics *gfx)
{
	return 0;
}

void Global::prelink()
{

}

void Global::Params(matrix4 &mvp, int depth)
{
	Graphics::current_mvp = mvp;
}

Global::~Global()
{

}

int mLight2::init(Graphics *gfx, bool pixel)
{
	return 0;
}

void mLight2::set_num_shadowmap(int value)
{

}

void mLight2::set_specular_factor(float value)
{
}

void mLight2::set_diffuse_factor(float value)
{
}

void mLight2::set_specular_exponent(float value)
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

void mLight2::set_specular_min(float value)
{
}

void mLight2::set_diffuse_min(float value)
{
}

void mLight2::set_max(int max)
{

}

void mLight2::Select()
{

}

void mLight2::set_tone(int tone)
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


void mLight2::set_ambient(float ambient)
{

}

void mLight2::set_lightmap(float lightmap)
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

void mLight2::portal(int value)
{
}

void mLight2::alphatest(int value, int p)
{
}

void mLight2::set_normalmap(float value)
{
}

void mLight2::set_normalmap_scale(vec3 &value)
{
}


void mLight2::set_matrix(matrix4 &matrix)
{
	return;
}

void mLight2::prelink()
{

}

void mLight2::set_alpha(float value)
{
}



void mLight2::Params(matrix4 &mvp, vector<Light *> &light_list, size_t num_lights, vec3 &offset, int u_time)
{
	Graphics::current_mvp = mvp;
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

}

void mLight2::tcmod_stretch_sawtooth(float amplitude, float phase, float freq, int tick_num, int index)
{

}

void mLight2::tcmod_stretch_inverse_sawtooth(float amplitude, float phase, float freq, int tick_num, int index)
{

}

void mLight2::tcmod_stretch_triangle(float amplitude, float phase, float freq, int tick_num, int index)
{

}

void mLight2::tcmod_stretch_square(float amplitude, float phase, float freq, int tick_num, int index)
{

}

//rgbgen
void mLight2::rgbgen_wave_sin(float amplitude, float phase, float freq, int tick_num, int index)
{

}

void mLight2::rgbgen_wave_sawtooth(float amplitude, float phase, float freq, int tick_num, int index)
{

}

void mLight2::rgbgen_wave_inverse_sawtooth(float amplitude, float phase, float freq, int tick_num, int index)
{

}

void mLight2::rgbgen_wave_triangle(float amplitude, float phase, float freq, int tick_num, int index)
{

}

void mLight2::rgbgen_wave_square(float amplitude, float phase, float freq, int tick_num, int index)
{

}

int Post::init(Graphics *gfx)
{
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

#define QUAD_PARTICLES

int ParticleRender::init(Graphics *gfx)
{
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
