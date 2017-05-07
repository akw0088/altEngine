#include "shader.h"
#include <cmath> // for some sin/cos/tan funcs

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//unsigned int ParticleUpdate::max_particles = 175000;

#ifdef DEDICATED

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

void Global::Params(matrix4 &mvp, int tex0)
{

}

int mLight2::init(Graphics *gfx)
{

	return 0;
}

void mLight2::set_max(int max)
{

}

void mLight2::set_ambient(float ambient)
{

}

void mLight2::set_lightmap(float lightmap)
{

}




void mLight2::prelink()
{

}


void mLight2::Params(matrix4 &mvp, vector<Light *> &light_list, size_t num_lights, vec3 &offset, int u_time)
{

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