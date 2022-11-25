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
	bool winding = false;
	unsigned int num_index = 0;

	vertex_t *vertex_array = NULL;
	size_t num_vertex;

	if (argc != 4)
	{
		cout << "Usage: " << argv[0] << " <obj file> <scalar> <single_file>" << endl;
		return 0;
	}



	string file_name = argv[1];
	float scalar;
	sscanf(argv[2], "%f", &scalar);

	int single_file = atoi(argv[3]);

	if (scalar > 0)
	{
		winding = true;
	}
	else
	{
		winding = false;
	}

	cout << "Loading obj file " << file_name << endl;
	obj.load(file_name);
	obj.scale(scalar);

	if (single_file)
	{
		obj.create_index_single(&index_array, num_index, winding);
		cout << "Writing index buffer" << endl;
		string ibo_name = file_name.replace(file_name.length() - 3, 3, "ibo");
		ofstream ibo(ibo_name.c_str(), ios::binary);
		ibo.write((char *)&num_index, sizeof(int));
		ibo.write((char *)index_array, num_index * sizeof(int));
		ibo.close();

		obj.create_vertex_single(&vertex_array, num_vertex);

		cout << "Writing vertex buffer" << endl;
		string vbo_name = file_name.substr(0, file_name.size() - 4);
		vbo_name.append(".vbo");

		ofstream vbo(vbo_name.c_str(), ios::binary);
		vbo.write((char *)&num_vertex, sizeof(int));
		vbo.write((char *)vertex_array, num_vertex * sizeof(vertex_t));
		vbo.close();

	}
	else
	{


		for (int k = 0; k < obj.object.size(); k++)
		{
			unsigned int num_index = 0;

			obj.create_index(&index_array, num_index, k, winding);
			string ibo_name = string(obj.object[k].name) + ".ibo";
			cout << "Writing index buffer for " + ibo_name << endl;
			ofstream ibo(ibo_name.c_str(), ios::binary);
			ibo.write((char *)&num_index, sizeof(int));
			ibo.write((char *)index_array, num_index * sizeof(int));
			ibo.close();
		}

		// create single vertex buffer from each index buffer
		num_vertex = obj.start_index;
		vertex_array = new vertex_t[num_vertex];

		//some verts are never referenced by any index
		memset(vertex_array, 0, num_vertex * sizeof(vertex_t));


		obj.start_index = 0;
		// run through all the faces, updating the attributes per face
		obj.create_vertex(&vertex_array);
		cout << "Writing vertex buffer" << endl;
		string vbo_name = file_name.substr(0, file_name.size() - 4);
		vbo_name.append(".vbo");

		ofstream vbo(vbo_name.c_str(), ios::binary);
		vbo.write((char *)&num_vertex, sizeof(int));
		vbo.write((char *)vertex_array, num_vertex * sizeof(vertex_t));
		vbo.close();
	}


	return 0;
}
