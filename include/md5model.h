#include "include.h"

#ifndef MD5MODEL_H
#define MD5MODEL_H



/*
md5_anim_t *anim = anim_list->anim;
for (;anim != NULL;)
{
delete anim->aabb;
delete anim->base;
delete anim->frame;
delete anim->hierarchy;
delete anim;
anim = anim->next;
}
*/

typedef struct
{
	md5_joint_t		**frame;
	int			index_array[8192];
	vertex_t		vertex_array[8192];
	int			**frame_index;
	int			**count_index;
	int			**frame_vertex;
	int			**count_vertex;
	int			*tex_object;
	int			*normal_object;
} md5_buffer_t;


class MD5Model
{
public:
	void load(char *md5file, char **animation, int num_anim, Graphics &gfx);
	void generate_buffers(Graphics &gfx, md5_anim_t *anim, md5_buffer_t *buffer);
	void destroy_buffers(Graphics &gfx);
	void render(Graphics &gfx, int frame_step);
	void select_animation(int index);

private:
	MD5			md5;
	md5_buffer_t *buffer[16];
	int num_buffer;
	md5_buffer_t *current_buffer;
	anim_list_t anim_list;
	md5_anim_t *current_anim;
};

#endif
