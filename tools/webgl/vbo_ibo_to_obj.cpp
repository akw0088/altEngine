#include <stdio.h>
#include <string.h>

typedef struct
{
	float x;
	float y;
	float z;
	float w;
} vec4_t;


typedef struct
{
	float x;
	float y;
	float z;
} vec3_t;

typedef struct
{
	float x;
	float y;
} vec2_t;

typedef struct
{
	int vindex[3];
	int tindex[3];
	int nindex[3];
} face_t;

typedef struct
{
	vec3_t	position;
	vec2_t	texCoord0;
	vec2_t	texCoord1;
	vec3_t	normal;
	int	color;
	vec4_t	tangent;
} vertex_t;


int convert(char *vbo, char *ibo, char *name)
{
	unsigned int *count = (unsigned int *)ibo;
	unsigned int *max = (unsigned int *)vbo;
	vertex_t *vert = (vertex_t *)(vbo + 4);
	unsigned int *index = (unsigned int *)(ibo + 4);
	
	unsigned int num_index = *count;
	unsigned int num_vert = *max;
	

	char file[128] = { 0 };

	sprintf(file, "%s.obj", name);
	FILE *fp = fopen(file, "w");
	if (fp == NULL)
	{
		perror("fp");
		return -1;
	}


	fprintf(fp, "\n# object %s\n\n", name);

	for (unsigned int i = 0; i < strlen(name); i++)
	{
		if (name[i] == '.')
		{
			name[i] = '\0';
			break;
		}
	}


	for (unsigned int i = 0; i < num_vert; i++)
	{
		fprintf(fp, "v %f %f %f\n",
			vert[i].position.x,
			vert[i].position.y,
			vert[i].position.z
		);
	}

	fprintf(fp, "\n\n");

	for (unsigned int i = 0; i < num_vert; i++)
	{
		fprintf(fp, "vt %f %f\n",
			vert[i].texCoord0.x,
			vert[i].texCoord0.y
		);
	}
	fprintf(fp, "\n\n");

	for (unsigned int i = 0; i < num_vert; i++)
	{
		fprintf(fp, "vn %f %f %f\n",
			vert[i].normal.x,
			vert[i].normal.y,
			vert[i].normal.z
		);
	}

	fprintf(fp, "\n\n");

	for (unsigned int i = 0; i < num_index; i += 3)
	{
		fprintf(fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", 
			index[i + 0] + 1, index[i + 0] + 1, index[i + 0] + 1,
			index[i + 2] + 1, index[i + 2] + 1, index[i + 2] + 1,
			index[i + 1] + 1, index[i + 1] + 1, index[i + 1] + 1
		);
	}


	fclose(fp);

	return 0;
}

char *get_file(char *filename, unsigned int *size)
{
	FILE	*file;
	char	*buffer;
	int	file_size, bytes_read;

	file = fopen(filename, "rb");
	if (file == NULL)
		return 0;
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = new char[file_size + 1];
	bytes_read = (int)fread(buffer, sizeof(char), file_size, file);
	if (bytes_read != file_size)
	{
		delete[] buffer;
		fclose(file);
		return 0;
	}
	fclose(file);
	buffer[file_size] = '\0';

	if (size != NULL)
	{
		*size = file_size;
	}

	return buffer;
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("Usage: %s <vbo> <ibo>\r\n", argv[0]);
		return 0;
	}


	unsigned int size = 0;
	char *vbo = get_file(argv[1], &size);
	char *ibo = get_file(argv[2], &size);
	if (vbo == NULL)
	{
		printf("Unable to open %s\r\n", argv[1]);
		return 0;
	}

	if (ibo == NULL)
	{
		printf("Unable to open %s\r\n", argv[2]);
		return 0;
	}
	convert(vbo, ibo, argv[2]);

	return 0;
}
