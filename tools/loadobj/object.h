#ifndef OBJECT_H
#define OBJECT_H

#include "include.h"

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

typedef struct
{
	std::vector<face_t> vec_face;
	char name[128];
	char matname[128];
} object_t;
#pragma pack(8)

class Object
{
public:
	void load(std::string &file_name);
	void pass_count(std::string &line);
	void pass_extract(std::string &line);

	void create_index(int **index_array, unsigned int &num_index, int k, bool winding);
	void create_index_single(int **index_array, unsigned int &num_index, bool winding);
	void create_vertex(vertex_t **vertex_array);
	void create_vertex_single(vertex_t **vertex_array, std::size_t &num_vertex);
	void scale(float scalar);

//private:
	std::vector<object_t> object;
	int num_vertex;
	int num_texture;
	int num_normal;
	int num_face;
	int num_object;
	int current_object;

	std::vector<vec3_t> vec_vertex;
	std::vector<vec2_t> vec_texture;
	std::vector<vec3_t> vec_normal;
	std::vector<face_t> vec_face; // for all in one index array


	unsigned int start_index;
};

#endif