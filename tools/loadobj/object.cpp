#include "include.h"

using namespace std;

void Object::load(string &file_name)
{
	ifstream file(file_name.c_str());
	string line;

	while( getline(file, line) )
	{
		pass_extract(line);
	}

}

void Object::pass_count(string &line)
{
	switch (line[0])
	{
	case '#':
		break;
	case 'v':
		switch (line[1])
		{
		case 't':
			num_texture++;
			break;
		case 'n':
			num_normal++;
			break;
		case ' ':
		case '\t':
			num_vertex++;
			break;
		}
		break;
	case 'f':
		num_face++;
		break;
	case 'o':
		num_object++;
		break;
	case 'g':
//		sscanf(line, "g %s", name);
		break;
	case 's':
//		sscanf(line, "s %s", name);
		break;
	case 'u':
//		sscanf(line, "usemtl %s", name);
		break;
	}
}

void Object::pass_extract(string &line)
{
	vec3_t	vertex;
	face_t	face;
	int ret;


	switch (line[0])
	{
	case '#':
		break;
	case 'v':
		switch (line[1])
		{
		case 't':
			sscanf(line.c_str(), "vt %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			vec_texture.push_back(vertex);
			break;
		case 'n':
			sscanf(line.c_str(), "vn %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			vec_normal.push_back(vertex);
			break;
		case ' ':
		case '\t':
			sscanf(line.c_str(), "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			vec_vertex.push_back(vertex);
			break;
		}
		break;
	case 'f':
	{
		face_t quad1;
		face_t quad2;

		ret = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
			&quad1.vindex[0], &quad1.tindex[0], &quad1.nindex[0],
			&quad1.vindex[1], &quad1.tindex[1], &quad1.nindex[1],
			&quad1.vindex[2], &quad1.tindex[2], &quad1.nindex[2],
			&quad2.vindex[2], &quad2.tindex[2], &quad2.nindex[2]
		);

		if (ret == 12)
		{
			//we have a quad definition
			//    Triangle 1: 1/1/1 2/2/2 3/3/3
			object[current_object].vec_face.push_back(quad1);

			//    Triangle 2: 1/1/1 3/3/3 4/4/4.
			ret = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&quad2.vindex[0], &quad2.tindex[0], &quad2.nindex[0],
				&quad1.vindex[1], &quad1.tindex[1], &quad1.nindex[1],
				&quad2.vindex[1], &quad2.tindex[1], &quad2.nindex[1],
				&quad2.vindex[2], &quad2.tindex[2], &quad2.nindex[2]
			);

			object[current_object].vec_face.push_back(quad2);
			break;
		}


		ret = sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
			&face.vindex[0], &face.tindex[0], &face.nindex[0],
			&face.vindex[1], &face.tindex[1], &face.nindex[1],
			&face.vindex[2], &face.tindex[2], &face.nindex[2]);

		if (ret != 9)
		{
			ret = sscanf(line.c_str(), "f %d//%d %d//%d %d//%d",
				&face.vindex[0], &face.nindex[0],
				&face.vindex[1], &face.nindex[1],
				&face.vindex[2], &face.nindex[2]);
			face.tindex[0] = -1;
			face.tindex[1] = -1;
			face.tindex[2] = -1;
		}
		if (object.size() == 0)
		{
			object.resize(object.size() + 1);
			current_object = num_object;
		}

		object[current_object].vec_face.push_back(face);
		break;
	}
	case 'g':
//		sscanf(line, "g %s", name);
		break;
	case 's':
//		sscanf(line, "s %s", name);
		break;
	case 'o':
		object.resize(object.size() + 1);
		current_object = num_object;
		sscanf(line.c_str(), "o %s", &object[current_object].name[0]);
		//printf("\tLoading sub-object %s\r\n", object[current_object].name);
		num_object++;
		break;
	case 'u':
	{
		sscanf(line.c_str(), "usemtl %s", &object[current_object].matname[0]);
		printf("%s: %s\r\n", object[current_object].name, object[current_object].matname);
		break;
	}
	}
}

void Object::scale(float scalar)
{
		for (int i = 0; i < vec_vertex.size(); i++)
		{
			vec_vertex[i].x *= scalar;
			vec_vertex[i].y *= scalar;
			vec_vertex[i].z *= scalar;
		}
}

void Object::create_index(int **index_array, int &num_index, int k)
{
	num_index = object[k].vec_face.size() * 3;
	(*index_array) = new int [num_index];
	int j = 0;

	for(int i = 0; i < object[k].vec_face.size(); i++)
	{
		(*index_array)[j++] = object[k].vec_face[i].vindex[0] - 1;
		(*index_array)[j++] = object[k].vec_face[i].vindex[2] - 1;
		(*index_array)[j++] = object[k].vec_face[i].vindex[1] - 1;
	}
}

/*
typedef struct
{
	int vindex[3];
	int tindex[3];
	int nindex[3];
} face_t;
*/

// really dont remember why I didnt want to include my vector classes in here
void create_tangent(face_t &face, vertex_t *vertex_array, int length)
{
	vec3_t p, q, n;
	vec3_t t, b;
	float s1, s2, t1, t2;
	float denom;


	if (face.vindex[0] - 1 >= length)
	{
		printf("create_tangent vertex out of range\r\n");
		return;
	}

	if (face.vindex[1] - 1 >= length)
	{
		printf("create_tangent vertex out of range\r\n");
		return;
	}

	if (face.vindex[2] - 1 >= length)
	{
		printf("create_tangent vertex out of range\r\n");
		return;
	}


	// triangle span vectors
	p.x = vertex_array[face.vindex[1] - 1].position.x - vertex_array[face.vindex[0] - 1].position.x;
	p.y = vertex_array[face.vindex[1] - 1].position.y - vertex_array[face.vindex[0] - 1].position.y;
	p.z = vertex_array[face.vindex[1] - 1].position.z - vertex_array[face.vindex[0] - 1].position.z;

	q.x = vertex_array[face.vindex[2] - 1].position.x - vertex_array[face.vindex[0] - 1].position.x;
	q.y = vertex_array[face.vindex[2] - 1].position.y - vertex_array[face.vindex[0] - 1].position.y;
	q.z = vertex_array[face.vindex[2] - 1].position.z - vertex_array[face.vindex[0] - 1].position.z;

	//cross product for normal
	n.x = p.y * q.z - p.z * q.y;
	n.y = p.z * q.x - p.x * q.x;
	n.z = p.x * q.z - p.z * q.x;

	// texture coordinate vectors
	s1 = vertex_array[face.vindex[1] - 1].texCoord0.x - vertex_array[face.vindex[0] - 1].texCoord0.x;
	t1 = vertex_array[face.vindex[1] - 1].texCoord0.y - vertex_array[face.vindex[0] - 1].texCoord0.y;

	s2 = vertex_array[face.vindex[2] - 1].texCoord0.x - vertex_array[face.vindex[0] - 1].texCoord0.x;
	t2 = vertex_array[face.vindex[2] - 1].texCoord0.y - vertex_array[face.vindex[0] - 1].texCoord0.y;

	// tangent and bitangent
	denom = (s1 * t2 - s2 * t1);
	if (denom != 0)
	{
		t.x = (p.x * t2 - q.x * t1) / denom;
		t.y = (p.y * t2 - q.y * t1) / denom;
		t.z = (p.z * t2 - q.z * t1) / denom;
		
		b.x = (q.x * s1 - p.x * s2) / denom;
		b.y = (q.y * s1 - p.y * s2) / denom;
		t.z = (q.z * s1 - p.z * s2) / denom;
	}
	else
	{
		t.x = 1.0f;
		t.y = 0.0f;
		t.z = 0.0f;
		b.x = 0.0f;
		b.y = 0.0f;
		b.z = 1.0f;
	}

	
	vertex_array[face.vindex[0] - 1].tangent.x = t.x;
	vertex_array[face.vindex[0] - 1].tangent.y = t.y;
	vertex_array[face.vindex[0] - 1].tangent.z = t.z;
	vertex_array[face.vindex[0] - 1].tangent.w = 1.0f;
}

void Object::create_vertex(vertex_t **vertex_array, int k)
{

	for(int i = 0; i < object[k].vec_face.size(); i++)
	{
		for(int j = 0; j < 3; j++)
		{
			//indexes are one based
			int vec_index = object[k].vec_face[i].vindex[j] - 1;
			int tex_index = object[k].vec_face[i].tindex[j] - 1;
			int norm_index = object[k].vec_face[i].nindex[j] - 1;

			if (vec_index >= vec_vertex.size())
			{
				printf("Warning object %s had out of range vertex\r\n", object[k].name);
				continue;
			}

			if (tex_index >= vec_texture.size())
			{
				printf("Warning object %s had out of range texcoord\r\n", object[k].name);
				continue;
			}

			if (norm_index >= vec_normal.size())
			{
				printf("Warning object %s had out of range normal\r\n", object[k].name);
				continue;
			}

			(*vertex_array)[ vec_index ].position = vec_vertex[ vec_index ];
			if (tex_index != -2 && tex_index < vec_texture.size() )
			{
				(*vertex_array)[ vec_index ].texCoord0.x = vec_texture[ tex_index ].x;
				(*vertex_array)[ vec_index ].texCoord0.y = vec_texture[ tex_index ].y;
			}
			(*vertex_array)[ vec_index ].normal = vec_normal[ norm_index ];
		}
//		create_tangent(object[k].vec_face[i], (*vertex_array), vec_vertex.size());
	}
}

