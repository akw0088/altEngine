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
	int num_index;
	model_header	header;

	vertex_t *vertex_array = NULL;
	int num_vertex;

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

	obj.create_index(&index_array, num_index);
	cout << "Writing index buffer" << endl;
	string ibo_name = file_name.replace(file_name.length() - 3, 3, "ibo");
	ofstream ibo(ibo_name.c_str(), ios::binary);
	ibo.write((char *)&num_index, sizeof(int));
	ibo.write((char *)index_array, num_index * sizeof(int));
	ibo.close();

	obj.create_vertex(&vertex_array, num_vertex);
	cout << "Writing vertex buffer" << endl;
	string vbo_name = file_name.replace(file_name.length() - 3, 3, "vbo");
	ofstream vbo(vbo_name.c_str(), ios::binary);
	vbo.write((char *)&num_vertex, sizeof(int));
	vbo.write((char *)vertex_array, num_vertex * sizeof(vertex_t));
	vbo.close();

	return 0;
}
