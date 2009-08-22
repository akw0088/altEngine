#include "voxel.h"

voxel::voxel()
{
	x = 512;
	y = 512;
	z = 512;

	data = new char [512 * 512 * 512];

	// Its a box! ?:)
	vertex_array = new vertex_t[8];
	vertex_array[0].vPosition = vec3(-0.5f, -0.5f, -0.5f);
	vertex_array[1].vPosition = vec3(-0.5f, -0.5f, 0.5f);
	vertex_array[2].vPosition = vec3(-0.5f, 0.5f, -0.5f);
	vertex_array[3].vPosition = vec3(-0.5f, 0.5f, 0.5f);
	vertex_array[4].vPosition = vec3(0.5f, -0.5f, -0.5f);
	vertex_array[5].vPosition = vec3(0.5f, -0.5f, 0.5f);
	vertex_array[6].vPosition = vec3(0.5f, 0.5f, -0.5f);
	vertex_array[7].vPosition = vec3(0.5f, 0.5f, 0.5f);
	num_vertex = 8;

	index_array = new int[36];
	index_array[0] = 0;
	index_array[1] = 2;
	index_array[2] = 1;

	index_array[3] = 1;
	index_array[4] = 2;
	index_array[5] = 3;

	index_array[6] = 7;
	index_array[7] = 5;
	index_array[8] = 1;

	index_array[9] = 7;
	index_array[10] = 1;
	index_array[11] = 3;

	index_array[12] = 5;
	index_array[13] = 7;
	index_array[14] = 6;

	index_array[15] = 5;
	index_array[16] = 6;
	index_array[17] = 4;

	index_array[18] = 0;
	index_array[19] = 6;
	index_array[20] = 2;

	index_array[21] = 0;
	index_array[22] = 4;
	index_array[23] = 6;

	index_array[24] = 0;
	index_array[25] = 1;
	index_array[26] = 4;

	index_array[27] = 1;
	index_array[28] = 5;
	index_array[29] = 4;

	index_array[30] = 2;
	index_array[31] = 6;
	index_array[32] = 3;

	index_array[33] = 3;
	index_array[34] = 6;
	index_array[35] = 7;

	num_index = 36;
}

void voxel::render(vec3 &position, Graphics &graphics)
{
	//brute force simple
	for(int i = 0; i < x; i++)
	{
		for(int j = 0; j < y; j++)
		{
			for(int k = 0; k < z; k++)
			{
				if (data[i + j * 512 + k * 512 * 512])
				{
					//render box
					glLoadIdentity();
					glTranslatef(i, j, k);
					glColor3f(1.0f, 0.0f, 0.0f);
					graphics.VertexArray(vertex_array, num_vertex);
					graphics.DrawArray(index_array, num_index);
				}
			}
		}
	}
}
