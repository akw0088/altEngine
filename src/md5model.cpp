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

// This just wraps class md5 for animation, essentially making a new vertex buffer for each frame

#include "include.h"
#include <stack>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "stb_image.h"

MD5Model::MD5Model()
{
	loaded = false;
	num_buffer = 0;
	normal_object = NULL;
	current_buffer = NULL;
	current_anim = NULL;
	tex_object = NULL;
	animation_frame = 0;
}

MD5Model::~MD5Model() 
{
	if (loaded == false)
		return;

	anim_list_t *plist = &anim_list;
	for (; plist != NULL;)
	{
		if (plist->anim)
		{
			delete [] plist->anim->aabb;
			delete [] plist->anim->base;
			delete [] plist->anim->frame;
			delete [] plist->anim->hierarchy;
			delete plist->anim;
		}
		plist = plist->next;
	}
}

void MD5Model::load(char *md5file, char **animation, int num_anim, Graphics &gfx, int anisotropic)
{
	anim_list_t *plist = &anim_list;

	printf("Loading md5 %s\n", md5file);
	if (md5.load_md5(md5file) != 0)
		return;

	// get bind pose vertex positions
	for (int i = 0; i < md5.model->num_mesh; i++)
	{
		md5.PrepareMesh(i, md5.model->joint, md5.num_index[i], md5.index_array[i], md5.vertex_array[i], md5.num_vertex[i]);
		md5.generate_tangent(md5.index_array[i], md5.num_index[i], md5.vertex_array[i], md5.num_vertex[i]);
	}

	load_textures(gfx, anisotropic);
	for (int i = 0; i < num_anim; i++)
	{
		printf("Loading animation %s\n", animation[i]);
		if (md5.load_md5_animation(animation[i], plist) != 0)
		{
			printf("Error loading md5\n");
			return;
		}
	}

	num_buffer = 0;
	for (;plist != NULL;)
	{
		buffer[num_buffer++] = new md5_buffer_t;

		//TODO: Interpolate between frames for smoother animation
		//TBH: animation looks fine as is
		md5.generate_animation(buffer[num_buffer-1]->frame, plist->anim);
		generate_buffers(gfx, plist->anim, buffer[num_buffer - 1]);
		md5.destroy_animation(buffer[num_buffer - 1]->frame, plist->anim);
		current_anim = plist->anim;
		plist = plist->next;
	}
	current_buffer = buffer[num_buffer-1];
	loaded = true;
	printf("Done\n");

	select_animation(ANIM_IDLE, true);
}

void MD5Model::generate_buffers(Graphics &gfx, md5_anim_t *anim, md5_buffer_t *buffer)
{
	static int			temp_index[8192];
	static vertex_t		temp_vertex[8192];

	buffer->frame_index = new int *[anim->num_frame];
	buffer->count_index = new int *[anim->num_frame];
	buffer->frame_vertex = new int *[anim->num_frame];
	buffer->count_vertex = new int *[anim->num_frame];
	for (int j = 0; j < anim->num_frame; j++)
	{
		buffer->frame_index[j] = new int[md5.model->num_mesh];
		buffer->count_index[j] = new int[md5.model->num_mesh];
		buffer->frame_vertex[j] = new int[md5.model->num_mesh];
		buffer->count_vertex[j] = new int[md5.model->num_mesh];

		for (int i = 0; i < md5.model->num_mesh; i++)
		{
			int num_index, num_vertex;

			md5.PrepareMesh(i, buffer->frame[j], num_index, temp_index, temp_vertex, num_vertex);
			buffer->frame_index[j][i] = gfx.CreateIndexBuffer(temp_index, num_index);
			buffer->count_index[j][i] = num_index;
			buffer->frame_vertex[j][i] = gfx.CreateVertexBuffer(temp_vertex, num_vertex);
			buffer->count_vertex[j][i] = num_vertex;
		}
	}
}

void MD5Model::load_textures(Graphics &gfx, int anisotropic)
{
	tex_object = new int[md5.model->num_mesh];
	normal_object = new int[md5.model->num_mesh];

	for (int i = 0; i < md5.model->num_mesh; i++)
		tex_object[i] = (int)-1;
	for (int i = 0; i < md5.model->num_mesh; i++)
		normal_object[i] = (int)-1;

	for (int i = 0; i < md5.model->num_mesh; i++)
	{
		char file[256];
		unsigned char *bytes;
		int width = 0;
		int height = 0;
		int components = 4;
		int format = 0;

		sprintf(file, "media/%s.tga", md5.model->mesh[i].shader);

		bytes = (unsigned char *)stbi_load(file, &width, &height, &components, STBI_rgb_alpha);

#ifdef OPENGL
		format = GL_RGBA;
		components = GL_RGBA8;
#endif
#ifdef DIRECTX
		format = 4;
		components = 4;
#endif
		if (bytes == NULL)
		{
			debugf("Unable to load texture %s\n", file);
			continue;
		}
		tex_object[i] = gfx.LoadTexture(width, height, 4, format, bytes, false, anisotropic);
		free((void *)bytes);

		sprintf(file, "media/%s_normal.tga", md5.model->mesh[i].shader);
		bytes = (unsigned char *)stbi_load(file, &width, &height, &components, STBI_rgb_alpha);
#ifdef OPENGL
		format = GL_RGBA;
		components = GL_RGBA8;
#endif
		if (bytes == NULL)
		{
			debugf("Unable to load texture %s\n", file);
			continue;
		}
		normal_object[i] = gfx.LoadTexture(width, height, components, format, bytes, false, anisotropic);
		free((void *)bytes);
	}
}

void MD5Model::select_animation(int index, bool loop)
{
	anim_list_t *plist = &anim_list;

	if (loaded == false)
		return;


	if (index > num_buffer - 1)
	{
		debugf("Invalid animation index\n");
		return;
	}

	if (loop == false)
	{
		play_once = true;
		animation_frame = 0;
		done = false;
	}
	

	for (int i = 0; i < index + 1; i++)
	{
		current_anim = plist->anim;
		plist = plist->next;
	}
	current_buffer = buffer[index];
}

void MD5Model::destroy_buffers(Graphics &gfx)
{
	anim_list_t *plist = &anim_list;
	int k = 0;

	if (loaded == false)
		return;

	for (; plist != NULL && k < num_buffer;)
	{
		md5_anim_t *anim;

		anim = plist->anim;
		for (int j = 0; j < anim->num_frame; j++)
		{
			for (int i = 0; i < md5.model->num_mesh; i++)
			{
				gfx.DeleteVertexBuffer(buffer[k]->frame_vertex[j][i]);
				gfx.DeleteIndexBuffer(buffer[k]->frame_index[j][i]);
			}
			delete[] buffer[k]->frame_index[j];
			delete[] buffer[k]->count_index[j];
			delete[] buffer[k]->frame_vertex[j];
			delete[] buffer[k]->count_vertex[j];
		}
		delete [] buffer[k]->frame_index;
		delete [] buffer[k]->count_index;
		delete [] buffer[k]->frame_vertex;
		delete [] buffer[k]->count_vertex;

		delete[] plist->anim->hierarchy;
		delete[] plist->anim->aabb;
		delete[] plist->anim->base;
		delete[] plist->anim->frame;

		delete plist->anim;
		plist = plist->next;
		k++;
	}

	
	for (int i = 0; i < md5.model->num_mesh; i++)
	{
		if (tex_object[i] != -1)
			gfx.DeleteTexture(tex_object[i]);
		if (normal_object[i] != -1)
			gfx.DeleteTexture(normal_object[i]);
	}

	delete[] tex_object;
	delete[] normal_object;

	for (int i = 0; i < num_buffer; i++)
	{
		delete buffer[i];
	}
	loaded = false;
}


void MD5Model::render(Graphics &gfx, int frame_step)
{
	if (loaded == false)
		return;

	if (play_once == false)
	{
		animation_frame = frame_step % current_anim->num_frame;
	}
	else
	{
		if (animation_frame < current_anim->num_frame - 1)
		{
			animation_frame++;
		}
	}


	for (int i = 0; i < md5.model->num_mesh; ++i)
	{
		gfx.SelectTexture(0, tex_object[i]);
		gfx.SelectIndexBuffer(current_buffer->frame_index[animation_frame][i]);
		gfx.SelectVertexBuffer(current_buffer->frame_vertex[animation_frame][i]);
		gfx.DrawArrayTri(0, 0, current_buffer->count_index[animation_frame][i], current_buffer->count_vertex[animation_frame][i]);
	}

	if (animation_frame == current_anim->num_frame - 1)
	{
		done = true;
		select_animation(ANIM_IDLE, true);
		animation_frame = 0;
	}

}
