#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_file(char* filename, unsigned int* size)
{
	FILE* file;
	char* buffer;
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


#pragma pack(1)
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
	vec3_t normal;
	vec3_t p1;
	vec3_t p2;
	vec3_t p3;
	unsigned short byte_count;
} stl_triangle_t;

typedef struct
{
	char header[80];
	unsigned int num_triangle;
	stl_triangle_t triangle[1];
} stl_file_t;

typedef struct
{
	vec3_t	position;
	vec2_t	texCoord0;
	vec2_t	texCoord1;
	vec3_t	normal;
	int	color;
	vec4_t	tangent;
} vertex_t;

#pragma pack(8)




int main(int argc, char *argv[])
{
	unsigned int size = 0;
	char output[128] = { 0 };
	char filename[128] = { 0 };



	if (argc != 3)
	{
		printf("Usage: %s <obj file> <scalar>\r\n", argv[0]);
		return 0;
	}

	for (int i = 0; i < strlen(argv[1]); i++)
	{
		if (argv[1][i] == '.')
		{
			filename[i] = 0;
			break;
		}

		filename[i] = argv[1][i];
	}

	float scalar;
	sscanf(argv[2], "%f", &scalar);
	printf("Loading stl file %s\r\n", argv[1]);

	char* data = get_file(argv[1], &size);
	stl_file_t* stl = (stl_file_t*)data;







	printf("Header %80s\r\n", stl->header);
	printf("%d triangles\r\n", stl->num_triangle);





	unsigned int num_vertex = stl->num_triangle * 3;
	vertex_t* vertex_array = (vertex_t *) malloc(num_vertex * sizeof(vertex_t));
	unsigned int *index_array = (unsigned int*)malloc(num_vertex * sizeof(unsigned int));
	num_vertex = 0;

	for (int i = 0; i < stl->num_triangle; i++)
	{
		/*
		printf("triangle %d\r\n", i);
		printf("\tnormal %f %f %f\r\n", stl->triangle[i].normal.x, stl->triangle[i].normal.y, stl->triangle[i].normal.z);
		printf("\tp1 %f %f %f\r\n", stl->triangle[i].p1.x, stl->triangle[i].p1.y, stl->triangle[i].p1.z);
		printf("\tp2 %f %f %f\r\n", stl->triangle[i].p2.x, stl->triangle[i].p2.y, stl->triangle[i].p2.z);
		printf("\tp3 %f %f %f\r\n", stl->triangle[i].p3.x, stl->triangle[i].p3.y, stl->triangle[i].p3.z);
		*/


		vertex_array[num_vertex].position.x = stl->triangle[i].p3.x * scalar;
		vertex_array[num_vertex].position.y = stl->triangle[i].p3.y * scalar;
		vertex_array[num_vertex].position.z = stl->triangle[i].p3.z * scalar;
		vertex_array[num_vertex].texCoord0.x = 0;
		vertex_array[num_vertex].texCoord0.y = 0;
		vertex_array[num_vertex].texCoord1.x = 0;
		vertex_array[num_vertex].texCoord1.y = 0;
		vertex_array[num_vertex].normal = stl->triangle[i].normal;
		vertex_array[num_vertex].color = ~0;
		vertex_array[num_vertex].tangent.x = 0;
		vertex_array[num_vertex].tangent.y = 0;
		vertex_array[num_vertex].tangent.z = 0;
		vertex_array[num_vertex].tangent.w = 0;
		index_array[num_vertex] = num_vertex;
		num_vertex++;


		vertex_array[num_vertex].position.x = stl->triangle[i].p2.x * scalar;
		vertex_array[num_vertex].position.y = stl->triangle[i].p2.y * scalar;
		vertex_array[num_vertex].position.z = stl->triangle[i].p2.z * scalar;
		vertex_array[num_vertex].texCoord0.x = 0;
		vertex_array[num_vertex].texCoord0.y = 0;
		vertex_array[num_vertex].texCoord1.x = 0;
		vertex_array[num_vertex].texCoord1.y = 0;
		vertex_array[num_vertex].normal = stl->triangle[i].normal;
		vertex_array[num_vertex].color = ~0;
		vertex_array[num_vertex].tangent.x = 0;
		vertex_array[num_vertex].tangent.y = 0;
		vertex_array[num_vertex].tangent.z = 0;
		vertex_array[num_vertex].tangent.w = 0;
		index_array[num_vertex] = num_vertex;
		num_vertex++;
		vertex_array[num_vertex].position.x = stl->triangle[i].p1.x * scalar;
		vertex_array[num_vertex].position.y = stl->triangle[i].p1.y * scalar;
		vertex_array[num_vertex].position.z = stl->triangle[i].p1.z * scalar;
		vertex_array[num_vertex].texCoord0.x = 0;
		vertex_array[num_vertex].texCoord0.y = 0;
		vertex_array[num_vertex].texCoord1.x = 0;
		vertex_array[num_vertex].texCoord1.y = 0;
		vertex_array[num_vertex].normal = stl->triangle[i].normal;
		vertex_array[num_vertex].color = ~0;
		vertex_array[num_vertex].tangent.x = 0;
		vertex_array[num_vertex].tangent.y = 0;
		vertex_array[num_vertex].tangent.z = 0;
		vertex_array[num_vertex].tangent.w = 0;
		index_array[num_vertex] = num_vertex;
		num_vertex++;
	}





	sprintf(output, "%s.vbo", filename);

	FILE* fp = fopen(output, "wb");
	fwrite(&num_vertex, sizeof(unsigned int), 1, fp);
	fwrite(vertex_array, sizeof(vertex_t), num_vertex, fp);
	fclose(fp);

	sprintf(output, "%s.ibo", filename);
	FILE* fp2 = fopen(output, "wb");
	fwrite(&num_vertex, sizeof(unsigned int), 1, fp2);
	fwrite(index_array, sizeof(unsigned int), num_vertex, fp2);
	fclose(fp2);

	
	return 0;
}