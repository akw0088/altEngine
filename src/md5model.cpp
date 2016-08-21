#include "include.h"

void MD5Model::load(char *md5file, char *animationfile, Graphics &gfx)
{
	printf("Loading md5 %s animation %s\n", md5file, animationfile);
	md5.load_md5(md5file);
	md5.load_md5_animation(animationfile);

	// get bind pose vertex positions
	for (int i = 0; i < md5.model->num_mesh; i++)
	{
		md5.PrepareMesh(i, md5.model->joint, md5.num_index[i], md5.index_array[i], md5.vertex_array[i], md5.num_vertex[i]);
		md5.generate_tangent(md5.index_array[i], md5.num_index[i], md5.vertex_array[i], md5.num_vertex[i]);
	}

	md5.generate_animation(frame);
	generate_buffers(gfx);
	md5.destroy_animation(frame);
	printf("Done\n");
}

void MD5Model::generate_buffers(Graphics &gfx)
{
	frame_index = new int *[md5.anim->num_frame];
	frame_vao = new int *[md5.anim->num_frame];
	count_index = new int *[md5.anim->num_frame];
	frame_vertex = new int *[md5.anim->num_frame];
	count_vertex = new int *[md5.anim->num_frame];
	for (int j = 0; j < md5.anim->num_frame; j++)
	{
		frame_vao[j] = new int[md5.model->num_mesh];
		frame_index[j] = new int[md5.model->num_mesh];
		count_index[j] = new int[md5.model->num_mesh];
		frame_vertex[j] = new int[md5.model->num_mesh];
		count_vertex[j] = new int[md5.model->num_mesh];

		for (int i = 0; i < md5.model->num_mesh; i++)
		{
			int num_index, num_vertex;

			md5.PrepareMesh(i, frame[j], num_index, index_array, vertex_array, num_vertex);
//			frame_vao[j][i] = gfx.CreateVertexArrayObject();
			frame_index[j][i] = gfx.CreateIndexBuffer(index_array, num_index);
			count_index[j][i] = num_index;
			frame_vertex[j][i] = gfx.CreateVertexBuffer(vertex_array, num_vertex);
			count_vertex[j][i] = num_vertex;
		}
	}

	tex_object = new int[md5.model->num_mesh];
	normal_object = new int[md5.model->num_mesh];
	for (int i = 0; i < md5.model->num_mesh; i++)
	{
		char buffer[256];
		int width, height, components, format;
		char *bytes;

		sprintf(buffer, "media/%s.tga", md5.model->mesh[i].shader);
		bytes = (char *)gltLoadTGA(buffer, &width, &height, &components, &format);
		if (bytes == NULL)
		{
			debugf("Unable to load texture %s\n", buffer);
			continue;
		}
		tex_object[i] = gfx.LoadTexture(width, height, components, format, bytes);
		delete[] bytes;

		sprintf(buffer, "media/%s_normal.tga", md5.model->mesh[i].shader);
		bytes = (char *)gltLoadTGA(buffer, &width, &height, &components, &format);
		if (bytes == NULL)
		{
			debugf("Unable to load texture %s\n", buffer);
			continue;
		}
		normal_object[i] = gfx.LoadTexture(width, height, components, format, bytes);
		delete[] bytes;
	}
}

void MD5Model::destroy_buffers(Graphics &gfx)
{
	for (int j = 0; j < md5.anim->num_frame; j++)
	{
		for (int i = 0; i < md5.model->num_mesh; i++)
		{
#ifndef DIRECTX
			gfx.DeleteVertexArrayObject(frame_vao[j][i]);
#endif
			gfx.DeleteVertexBuffer(frame_vertex[j][i]);
			gfx.DeleteIndexBuffer(frame_index[j][i]);
		}
		delete[] frame_vao[j];
		delete[] frame_index[j];
		delete[] count_index[j];
		delete[] frame_vertex[j];
		delete[] count_vertex[j];
	}
	delete frame_vao;
	delete frame_index;
	delete count_index;
	delete frame_vertex;
	delete count_vertex;

	for (int i = 0; i < md5.model->num_mesh; i++)
	{
		gfx.DeleteTexture(tex_object[i]);
		gfx.DeleteTexture(normal_object[i]);
	}

	delete[] tex_object;
	delete[] normal_object;
}


void MD5Model::render(Graphics &gfx, int frame_step)
{
	frame_step = frame_step % md5.anim->num_frame;

	for (int i = 0; i < md5.model->num_mesh; ++i)
	{
		//		gfx.SelectVertexArrayObject(frame_vao[frame_step][i]);
		gfx.SelectTexture(0, tex_object[i]);
		gfx.SelectTexture(2, normal_object[i]);
		gfx.SelectIndexBuffer(frame_index[frame_step][i]);
		gfx.SelectVertexBuffer(frame_vertex[frame_step][i]);
		gfx.DrawArray(PRIM_TRIANGLES, 0, 0, count_index[frame_step][i], count_vertex[frame_step][i]);
		//		gfx.SelectVertexArrayObject(0);
		gfx.SelectVertexBuffer(0);
		gfx.SelectIndexBuffer(0);
	}
}
