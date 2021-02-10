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

#include "include.h"

#ifndef LIGHT_H
#define LIGHT_H

#define MAX_SHADOWVOL 4
#define SHADOWVOL_MAX_DIST (400.0f)

#define NUM_CUBE_FACE 6

class EntLight
{
public:
	EntLight(Entity *entity, Graphics &gfx, int num, float scale);
	void render_shadow_volume(Graphics &gfx, int index);
	void render_map_shadowvol(Graphics &gfx);
	void generate_map_volumes(Graphics &gfx, Bsp &map, int current_light);
	void generate_ent_volumes(Graphics &gfx, vector<Entity *> &entity_list);

	void destroy(Graphics &gfx);
	void generate_cubemaps(Graphics &gfx);

	Entity	*entity;

	// Shadow maps
	unsigned int fbo_shadowmaps[NUM_CUBE_FACE];
	unsigned int quad_tex[NUM_CUBE_FACE];
	unsigned int depth_tex[NUM_CUBE_FACE];
	unsigned int array_tex;


	matrix4	shadow_projection;
	matrix4	shadow_matrix[NUM_CUBE_FACE];
	int		shadow_flag;


	// Shadow Volumes
#ifdef SHADOWVOL
	ShadowVolume	shadow[MAX_SHADOWVOL];
	int				num_shadowvol;


	ShadowVolume map_shadow;
#endif

	vec3			color;
	float			intensity;
	float			attenuation;
	bool			active;
	bool			timer_flag;
	float			lightmap_scale;
	int				timer;
	unsigned int	light_num;
};

#endif
