#ifndef OBJECT_H
#define OBJECT_H

#include "include.h"

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

using namespace std;

typedef struct
{
	vector<face_t> vec_face;
	char name[128];
	char matname[128];
} object_t;

class Object
{
public:
	void load(string &file_name);
	void pass_count(string &line);
	void pass_extract(string &line);

	void create_index(int **index_array, unsigned int &num_index, int k, bool winding);
	void create_vertex(vertex_t **vertex_array, int k);
	void scale(float scalar);

//private:
	vector<object_t> object;
	int num_vertex;
	int num_texture;
	int num_normal;
	int num_face;
	int num_object;
	int current_object;

	vector<vec3_t> vec_vertex;
	vector<vec2_t> vec_texture;
	vector<vec3_t> vec_normal;

};

#endif