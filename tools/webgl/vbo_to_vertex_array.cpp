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


int convert(char *data, char *name)
{
	unsigned int *count = (unsigned int *)data;
	vertex_t *vert = (vertex_t *)(data + 4);

	char file[128] = { 0 };

	sprintf(file, "%s.txt", name);
	FILE *fp = fopen(file, "w");
	if (fp == NULL)
	{
		perror("fp");
		return -1;
	}


	for (int i = 0; i < strlen(name); i++)
	{
		if (name[i] == '.')
		{
			name[i] = '\0';
			break;
		}
	}
	fprintf(fp, "\tconst tri_num_%s = %d;\r\n", name, *count);
	fprintf(fp, "\tconst tri_%s = new Float32Array([\r\n", name);
	for (int i = 0; i < *count; i++)
	{
		fprintf(fp, "%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, ",
			vert[i].position.x,
			vert[i].position.y,
			vert[i].position.z,
			vert[i].tangent.x, //r
			vert[i].tangent.y, //g
			vert[i].tangent.z, //b
			vert[i].normal.x,
			vert[i].normal.y,
			vert[i].normal.z,
			vert[i].texCoord0.x,
			vert[i].texCoord0.y
		);
	}
	fprintf(fp, "\t]);\r\n");
	fprintf(fp, "\tnum_tri.push(tri_num_%s);\r\n", name);
	fprintf(fp, "\ttri.push(tri_%s);\r\n", name);
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
	if (argc < 2)
	{
		printf("Usage: %s <file>\r\n", argv[0]);
		return 0;
	}


	unsigned int size = 0;
	char *data = get_file(argv[1], &size);
	if (data == NULL)
	{
		printf("Unable to open %s\r\n", argv[1]);
		return 0;
	}
	convert(data, argv[1]);

	return 0;
}
