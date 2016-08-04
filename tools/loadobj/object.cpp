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
		vec_face.push_back(face);
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

void Object::scale(float scalar)
{
	for(int i = 0; i < vec_vertex.size(); i++)
	{
		vec_vertex[i].x *= scalar;
		vec_vertex[i].y *= scalar;
		vec_vertex[i].z *= scalar;
	}
}

void Object::create_index(int **index_array, int &num_index)
{
	num_index = vec_face.size() * 3;
	(*index_array) = new int [num_index];
	int j = 0;

	for(int i = 0; i < vec_face.size(); i++)
	{
		(*index_array)[j++] = vec_face[i].vindex[0] - 1;
		(*index_array)[j++] = vec_face[i].vindex[2] - 1;
		(*index_array)[j++] = vec_face[i].vindex[1] - 1;
	}
}

void Object::create_vertex(vertex_t **vertex_array, int &num_vertex)
{
	num_vertex = vec_vertex.size();
	(*vertex_array) = new vertex_t [num_vertex];

	//some verts are never referenced by any index
	memset((*vertex_array), 0, num_vertex * sizeof(vertex_t));

	for(int i = 0; i < vec_face.size(); i++)
	{
		for(int j = 0; j < 3; j++)
		{
			//indexes are one based
			int vec_index = vec_face[i].vindex[j] - 1;
			int tex_index = vec_face[i].tindex[j] - 1;
			int norm_index = vec_face[i].nindex[j] - 1;

			(*vertex_array)[ vec_index ].position = vec_vertex[ vec_index ];
			if (tex_index != -2)
			{
				(*vertex_array)[ vec_index ].texCoord0.x = vec_texture[ tex_index ].x;
				(*vertex_array)[ vec_index ].texCoord0.y = vec_texture[ tex_index ].y;
			}
			(*vertex_array)[ vec_index ].normal = vec_normal[ norm_index ];
		}
	}
}