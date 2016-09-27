#include "include.h"

#ifndef MD5MODEL_H
#define MD5MODEL_H

typedef struct
{
	md5_joint_t	**frame;
	int			**frame_index;
	int			**count_index;
	int			**frame_vertex;
	int			**count_vertex;
} md5_buffer_t;


class MD5Model
{
public:
	MD5Model();
	~MD5Model();
	void load(char *md5file, char **animation, int num_anim, Graphics &gfx);
	void generate_buffers(Graphics &gfx, md5_anim_t *anim, md5_buffer_t *buffer);
	void destroy_buffers(Graphics &gfx);
	void load_textures(Graphics &gfx);
	void render(Graphics &gfx, int frame_step);
	void select_animation(int index);

private:
	MD5			md5;
	md5_buffer_t *buffer[32];
	md5_buffer_t *current_buffer;
	anim_list_t anim_list;
	md5_anim_t *current_anim;
	int			*tex_object;
	int			*normal_object;
	bool		loaded;
	int num_buffer;

};

#endif
