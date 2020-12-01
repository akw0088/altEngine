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

#ifndef MD5MODEL_H
#define MD5MODEL_H

class MD5Model
{
public:
	MD5Model();
	~MD5Model();
	void load(char *md5file, char **animation, int num_anim, Graphics &gfx, int anisotropic);
	void generate_buffers(Graphics &gfx, md5_anim_t *anim, md5_buffer_t *buffer);
	void destroy_buffers(Graphics &gfx);
	void load_textures(Graphics &gfx, int anisotropic);
	void render(Graphics &gfx, int frame_step);
	void select_animation(int index, bool once);

	bool		done;
private:
	MD5			md5;
	md5_buffer_t *buffer[32];
	md5_buffer_t *current_buffer;
	anim_list_t anim_list;
	md5_anim_t *current_anim;
	int			*tex_object;
	int			*normal_object;
	bool		loaded;
	int			num_buffer;
	bool		play_once;

	int			animation_frame;
};

#endif
