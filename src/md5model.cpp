#include "include.h"

void MD5Model::load(char *md5file, char **animation, int num_anim, Graphics &gfx)
{
	anim_list_t *plist = &anim_list;

	printf("Loading md5 %s\n", md5file);
	md5.load_md5(md5file);

	// get bind pose vertex positions
	for (int i = 0; i < md5.model->num_mesh; i++)
	{
		md5.PrepareMesh(i, md5.model->joint, md5.num_index[i], md5.index_array[i], md5.vertex_array[i], md5.num_vertex[i]);
		md5.generate_tangent(md5.index_array[i], md5.num_index[i], md5.vertex_array[i], md5.num_vertex[i]);
	}


	for (int i = 0; i < num_anim; i++)
	{
		printf("Loading animation %s\n", animation[i]);
		if (md5.load_md5_animation(animation[i], plist) != 0)
		{
			printf("Error loading md5\n");
			return;
		}
	}

	for (;plist != NULL;)
	{
		buffer[num_buffer++] = new md5_buffer_t;

		md5.generate_animation(buffer[num_buffer-1]->frame, plist->anim);
		generate_buffers(gfx, plist->anim, buffer[num_buffer - 1]);
		md5.destroy_animation(buffer[num_buffer - 1]->frame, plist->anim);
		current_anim = plist->anim;
		plist = plist->next;
	}
	current_buffer = buffer[num_buffer-1];
	printf("Done\n");
}

void MD5Model::generate_buffers(Graphics &gfx, md5_anim_t *anim, md5_buffer_t *buffer)
{
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

			md5.PrepareMesh(i, buffer->frame[j], num_index, buffer->index_array, buffer->vertex_array, num_vertex);
			buffer->frame_index[j][i] = gfx.CreateIndexBuffer(buffer->index_array, num_index);
			buffer->count_index[j][i] = num_index;
			buffer->frame_vertex[j][i] = gfx.CreateVertexBuffer(buffer->vertex_array, num_vertex);
			buffer->count_vertex[j][i] = num_vertex;
		}
	}

	buffer->tex_object = new int[md5.model->num_mesh];
	buffer->normal_object = new int[md5.model->num_mesh];
	for (int i = 0; i < md5.model->num_mesh; i++)
	{
		char file[256];
		int width, height, components, format;
		char *bytes;

		sprintf(file, "media/%s.tga", md5.model->mesh[i].shader);
		bytes = (char *)gltLoadTGA(file, &width, &height, &components, &format);
		if (bytes == NULL)
		{
			debugf("Unable to load texture %s\n", file);
			continue;
		}
		buffer->tex_object[i] = gfx.LoadTexture(width, height, components, format, bytes);
		delete[] bytes;

		sprintf(file, "media/%s_normal.tga", md5.model->mesh[i].shader);
		bytes = (char *)gltLoadTGA(file, &width, &height, &components, &format);
		if (bytes == NULL)
		{
			debugf("Unable to load texture %s\n", file);
			continue;
		}
		buffer->normal_object[i] = gfx.LoadTexture(width, height, components, format, bytes);
		delete[] bytes;
	}
}

void MD5Model::select_animation(int index)
{
	anim_list_t *plist = &anim_list;


	if (index > num_buffer - 1)
	{
		debugf("Invalid animation index\n");
		return;
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

	for (; plist != NULL;)
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
		delete buffer[k]->frame_index;
		delete buffer[k]->count_index;
		delete buffer[k]->frame_vertex;
		delete buffer[k]->count_vertex;

		plist = plist->next;
		k++;
	}

	for (int k = 0; k < num_buffer; k++)
	{
		for (int i = 0; i < md5.model->num_mesh; i++)
		{
			gfx.DeleteTexture(buffer[k]->tex_object[i]);
			gfx.DeleteTexture(buffer[k]->normal_object[i]);
		}

		delete[] buffer[k]->tex_object;
		delete[] buffer[k]->normal_object;
	}

}


void MD5Model::render(Graphics &gfx, int frame_step)
{
	frame_step = frame_step % current_anim->num_frame;

	for (int i = 0; i < md5.model->num_mesh; ++i)
	{
		gfx.SelectTexture(0, current_buffer->tex_object[i]);
		gfx.SelectTexture(2, current_buffer->normal_object[i]);
		gfx.SelectIndexBuffer(current_buffer->frame_index[frame_step][i]);
		gfx.SelectVertexBuffer(current_buffer->frame_vertex[frame_step][i]);
		gfx.DrawArray(PRIM_TRIANGLES, 0, 0, current_buffer->count_index[frame_step][i], current_buffer->count_vertex[frame_step][i]);
		gfx.SelectVertexBuffer(0);
		gfx.SelectIndexBuffer(0);
	}
}
