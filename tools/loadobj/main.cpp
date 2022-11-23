#include "include.h"

using namespace std;

typedef struct
{
	char name[4];
	int num;
} model_header;

int main(int argc, char *argv[])
{
	Object obj;
	int *index_array = NULL;
	

	vertex_t *vertex_array = NULL;
	size_t num_vertex;

	if (argc != 3)
	{
		cout << "Usage: " << argv[0] << " <obj file> <scalar>" << endl;
		return 0;
	}

	string file_name = argv[1];
	float scalar;
	sscanf(argv[2], "%f", &scalar);
	cout << "Loading obj file " << file_name << endl;
	obj.load(file_name);
	obj.scale(scalar);


	for (int k = 0; k < obj.object.size(); k++)
	{
		unsigned int num_index = 0;

		obj.create_index(&index_array, num_index, k);
		string ibo_name = string(obj.object[k].name) + ".ibo";
		cout << "Writing index buffer for " + ibo_name << endl;
		ofstream ibo(ibo_name.c_str(), ios::binary);
		ibo.write((char *)&num_index, sizeof(int));
		ibo.write((char *)index_array, num_index * sizeof(int));
		ibo.close();
	}

	// create single vertex buffer from each index buffer
	num_vertex = obj.vec_vertex.size();
	vertex_array = new vertex_t[num_vertex];

	//some verts are never referenced by any index
	memset(vertex_array, 0, num_vertex * sizeof(vertex_t));

	// run through all the faces, updating the attributes per face
	for (int k = 0; k < obj.object.size(); k++)
	{
		obj.create_vertex(&vertex_array, k);
	}

	cout << "Writing vertex buffer" << endl;
	string vbo_name = file_name.substr(0, file_name.size() - 4);
	vbo_name.append(".vbo");

	ofstream vbo(vbo_name.c_str(), ios::binary);
	vbo.write((char *)&num_vertex, sizeof(int));
	vbo.write((char *)vertex_array, num_vertex * sizeof(vertex_t));
	vbo.close();


	return 0;
}
