#ifndef OBJECT_H
#define OBJECT_H

#include "include.h"

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
	int		color;
} vertex_t;

using namespace std;

class Object
{
public:
	void load(string &file_name);
	void pass_count(string &line);
	void pass_extract(string &line);
	void create_index(int **index_array, int &num_index);
	void create_vertex(vertex_t **vertex_array, int &num_vertex);
	void scale(float scalar);
private:
	int num_vertex;
	int num_texture;
	int num_normal;
	int num_face;

	vector<vec3_t> vec_vertex;
	vector<vec3_t> vec_texture;
	vector<vec3_t> vec_normal;
	vector<face_t> vec_face;
};

#endif