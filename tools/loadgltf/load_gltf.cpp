#include <iostream>
#include <fstream>
#include <json/json.h> 
#include "base64.h"

// Got the jist of things from here, half tempted to write my own JSON parser
// https://wirewhiz.com/read-gltf-files/
//


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
	vec3_t	position;
	vec2_t	texCoord0;
	vec2_t	texCoord1;
	vec3_t	normal;
	int	color;
	vec4_t	tangent;
} vertex_t;
#pragma pack(8)

#include <string>
using namespace std;


/*
Component Types
	const GLenum BYTE                           = 0x1400;
	const GLenum UNSIGNED_BYTE                  = 0x1401;
	const GLenum SHORT                          = 0x1402;
	const GLenum UNSIGNED_SHORT                 = 0x1403;
	const GLenum INT                            = 0x1404;
	const GLenum UNSIGNED_INT                   = 0x1405;
	const GLenum FLOAT                          = 0x1406;
	const GLenum FLOAT_VEC2                     = 0x8B50;
	const GLenum FLOAT_VEC3                     = 0x8B51;
	const GLenum FLOAT_VEC4                     = 0x8B52;
	const GLenum INT_VEC2                       = 0x8B53;
	const GLenum INT_VEC3                       = 0x8B54;
	const GLenum INT_VEC4                       = 0x8B55;
	const GLenum BOOL                           = 0x8B56;
	const GLenum BOOL_VEC2                      = 0x8B57;
	const GLenum BOOL_VEC3                      = 0x8B58;
	const GLenum BOOL_VEC4                      = 0x8B59;
	const GLenum FLOAT_MAT2                     = 0x8B5A;
	const GLenum FLOAT_MAT3                     = 0x8B5B;
	const GLenum FLOAT_MAT4                     = 0x8B5C;
*/

// might need to reindex master version
std::vector<string> master_name;
std::vector<vertex_t> master_vertex_array;
std::vector<unsigned int> master_index_array;


int save_mesh(char *name, std::vector<vertex_t> &vertex_array, std::vector<unsigned int> &index_array)
{
	unsigned int num_index = index_array.size();

	string ibo_name = string(name) + ".ibo";
	cout << "\tWriting index buffer for " + ibo_name << endl;
	ofstream ibo(ibo_name.c_str(), ios::binary);
	ibo.write((char*)&num_index, sizeof(int));
	ibo.write((char*)&index_array[0], num_index * sizeof(int));
	ibo.close();

	unsigned int num_vertex = vertex_array.size();

	string vbo_name = string(name) + ".vbo";
	cout << "\tWriting vertex buffer for " + vbo_name << endl;

	ofstream vbo(vbo_name.c_str(), ios::binary);
	vbo.write((char*)&num_vertex, sizeof(int));
	vbo.write((char*)&vertex_array[0], num_vertex * sizeof(vertex_t));
	vbo.close();


	return 0;
}





void gltf_loader(Json::Value root, char* data, bool print)
{
	int num_mesh = root["meshes"].size();

	for (unsigned int i = 0; i < num_mesh; i++)
	{
		char name[128] = { 0 };
		std::vector<vertex_t> vertex_array;
		std::vector<unsigned int> index_array;





		Json::Value& json_name = root["meshes"][i]["name"];


		sprintf(&name[0], "%s", json_name.asCString());


		unsigned int master_base = master_vertex_array.size();
		unsigned int master_index_base = master_index_array.size();

		master_name.push_back(name);

		if (print)
		{
			printf("Loading %s\r\n", name);
		}

		// get the primitives array
		Json::Value& primitive = root["meshes"][i]["primitives"][0];
		int pos_index = primitive["attributes"]["POSITION"].asInt();
		int norm_index = primitive["attributes"]["NORMAL"].asInt();
		int tangent_index = primitive["attributes"]["TANGENT"].asInt();
		int tex_index = primitive["attributes"]["TEXCOORD_0"].asInt();

		int index_index = primitive["indices"].asInt();

		// 5123 == short, 5124 == int, 5125 == unsigned int, 5126 == float
		int type = root["accessors"][index_index]["componentType"].asInt();




		// Get the accessor for position: 
		Json::Value& position_Accessor = root["accessors"][pos_index];
		Json::Value& normal_Accessor = root["accessors"][norm_index];
		Json::Value& tangent_Accessor = root["accessors"][tangent_index];
		Json::Value& texture_Accessor = root["accessors"][tex_index];

		//indices
		Json::Value& index_Accessor = root["accessors"][primitive["indices"].asInt()]; // 4
		//Json::Value& index_type = root["accessors"][index_index]["componentType"].asInt();


		// Get the bufferView 
		Json::Value& position_BufferView = root["bufferViews"][position_Accessor["bufferView"].asInt()];
		Json::Value& normal_BufferView = root["bufferViews"][normal_Accessor["bufferView"].asInt()];
		Json::Value& tangent_BufferView = root["bufferViews"][tangent_Accessor["bufferView"].asInt()];
		Json::Value& texture_BufferView = root["bufferViews"][texture_Accessor["bufferView"].asInt()];

		Json::Value& index_BufferView = root["bufferViews"][index_Accessor["bufferView"].asInt()];


		// Now get the start of the float3 array by adding the bufferView byte offset to the bin pointer
		// It's a little sketchy to cast to a raw float array, but hey, it works.
		float* position_buffer = (float*)(data + position_BufferView["byteOffset"].asInt());
		float* normal_buffer = (float*)(data + normal_BufferView["byteOffset"].asInt());
		float* tangent_buffer = (float*)(data + tangent_BufferView["byteOffset"].asInt());
		float* texture_buffer = (float*)(data + texture_BufferView["byteOffset"].asInt());

		unsigned int* index_buffer = (unsigned int*)(data + index_BufferView["byteOffset"].asInt());
		unsigned short* index_buffer_short = (unsigned short*)(data + index_BufferView["byteOffset"].asInt());


		// And as a cherry on top, let's print out the total number of verticies
		if (print)
		{
			std::cout << "\tvertices: " << position_Accessor["count"].asInt() << std::endl;
		}
		// Print out all the vertex positions 
		for (int i = 0; i < position_Accessor["count"].asInt(); ++i)
		{
			vertex_t v = { 0 };


			if (print)
			{
				std::cout << "positions (" << position_buffer[i * 3 + 0] << ", " << position_buffer[i * 3 + 1] << ", " << position_buffer[i * 3 + 2] << ")" << std::endl;
			}
			v.position.x = position_buffer[i * 3 + 0];
			v.position.y = position_buffer[i * 3 + 1];
			v.position.z = position_buffer[i * 3 + 2];

			vertex_array.push_back(v);

			master_vertex_array.push_back(v);

		}



		if (print)
		{
			std::cout << "\tnormals: " << normal_Accessor["count"].asInt() << std::endl;
		}
		for (int i = 0; i < normal_Accessor["count"].asInt(); ++i)
		{
			if (print)
			{
				std::cout << "normal (" << normal_buffer[i * 3] << ", " << normal_buffer[i * 3 + 1] << ", " << normal_buffer[i * 3 + 2] << ")" << std::endl;
			}
			vertex_array[i].normal.x = normal_buffer[i * 3 + 0];
			vertex_array[i].normal.y = normal_buffer[i * 3 + 1];
			vertex_array[i].normal.z = normal_buffer[i * 3 + 2];


			master_vertex_array[i + master_base].normal = vertex_array[i].normal;

		}


		if (print)
		{
			std::cout << "\ttangents: " << tangent_Accessor["count"].asInt() << std::endl;
		}
		for (int i = 0; i < tangent_Accessor["count"].asInt(); ++i)
		{
			if (print)
			{
				std::cout << "tangents (" << tangent_buffer[i * 4] << ", " << tangent_buffer[i * 4 + 1] << ", " << tangent_buffer[i * 4 + 2] << ", " << tangent_buffer[i * 4 + 3] << ")" << std::endl;
			}
			vertex_array[i].tangent.x = tangent_buffer[i * 4 + 0];
			vertex_array[i].tangent.y = tangent_buffer[i * 4 + 1];
			vertex_array[i].tangent.z = tangent_buffer[i * 4 + 2];
			vertex_array[i].tangent.w = tangent_buffer[i * 4 + 3];

			master_vertex_array[i + master_base].tangent = vertex_array[i].tangent;

		}



		if (print)
		{
			std::cout << "\ttextures: " << texture_Accessor["count"].asInt() << std::endl;
		}
		for (int i = 0; i < texture_Accessor["count"].asInt(); ++i)
		{
			if (print)
			{
				std::cout << "textures (" << texture_buffer[i * 2] << ", " << texture_buffer[i * 2 + 1] << ")" << std::endl;
			}
			vertex_array[i].texCoord0.x = texture_buffer[i * 2 + 0];
			vertex_array[i].texCoord0.y = texture_buffer[i * 2 + 1];
			vertex_array[i].texCoord1 = vertex_array[i].texCoord0;

			master_vertex_array[i + master_base].texCoord0 = vertex_array[i].texCoord0;
			master_vertex_array[i + master_base].texCoord1 = vertex_array[i].texCoord1;
		}


		if (print)
		{
			std::cout << "\index: " << index_Accessor["count"].asInt() << std::endl;
		}
		for (int i = 0; i < index_Accessor["count"].asInt(); ++i)
		{
			if (print)
			{
				std::cout << "textures (" << texture_buffer[i * 2] << ", " << texture_buffer[i * 2 + 1] << ")" << std::endl;
			}
			if (type == 0x1405 || type == 0x1404)
			{
				index_array.push_back(index_buffer[i]);
				master_index_array.push_back(index_buffer[i] + master_index_base);
			}
			else if (type == 0x1402 || type == 0x1403)
			{
				index_array.push_back(index_buffer_short[i]);
				master_index_array.push_back(index_buffer_short[i] + master_index_base);
			}
		}



		save_mesh(&name[0], vertex_array, index_array);
	}

}



int gltf_load_external_bin(char * gltfFilename, bool print)
{
	// open the gltf file
	std::ifstream jsonFile(gltfFilename, std::ios::binary);

	// parse the json so we can use it later
	Json::Value root;

	if (print)
	{
		printf("Parsing json...\r\n");
	}

	try
	{
		jsonFile >> root;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Json parsing error: " << e.what() << std::endl;
	}
	jsonFile.close();

	// Extract the name of the bin file, for the sake of simplicity I'm assuming there's only one
	std::string binFilename = root["buffers"][0]["uri"].asString();

	// Open it with the cursor at the end of the file so we can determine it's size,
	// We could techincally read the filesize from the gltf file, but I trust the file itself more
	std::ifstream binFile = std::ifstream(binFilename, std::ios::binary | std::ios::ate);

	// Read file length and then reset cursor
	size_t binLength = binFile.tellg();
	binFile.seekg(0);


	std::vector<char> bin(binLength);
	binFile.read(bin.data(), binLength);
	binFile.close();

	gltf_loader(root, bin.data(), print);

	printf("Loading %s as single ibo/vbo\r\n", gltfFilename);
	save_mesh(gltfFilename, master_vertex_array, master_index_array);

	return 0;
}


int gltf_load_base64_bin(char* gltfFilename, bool print)
{
	// open the gltf file
	std::ifstream jsonFile(gltfFilename, std::ios::binary);

	// parse the json so we can use it later
	Json::Value root;

	if (print)
	{ 
		printf("Parsing json...\r\n");
	}

	try
	{
		jsonFile >> root;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Json parsing error: " << e.what() << std::endl;
	}
	jsonFile.close();

	// Extract the name of the bin file, for the sake of simplicity I'm assuming there's only one
//	std::string binFilename = root["buffers"][0]["uri"].asString();


	std::string base64_string = root["buffers"][0]["uri"].asString();
	unsigned int bas64_bin_size = root["buffers"][0]["byteLength"].asUInt();

	// output should be smaller than the base64, but double it just in case
	unsigned char* binfile = (unsigned char *)malloc(bas64_bin_size * 2);
	size_t bin_file_size = bas64_bin_size * 2;


	const char* base64_bin = strstr(base64_string.c_str(), "base64,");
	if (base64_bin == NULL)
	{
		printf("failed to bind base64 string\r\n");
		return -1;
	}
	base64_bin += strlen("base64,");

	if (print)
	{
		printf("decoding base64...\r\n");
	}

	int ret = base64decode(base64_bin, bas64_bin_size, binfile, &bin_file_size);
	if (ret != 0)
	{
		printf("base64decode failed\r\n");
		return -1;
	}
	

	gltf_loader(root, (char *)binfile, print);


	printf("Loading %s as single ibo/vbo\r\n", gltfFilename);
	save_mesh(gltfFilename, master_vertex_array, master_index_array);

	return 0;
}


int glb_load(char *glbFilename, bool print)
{
	std::ifstream binFile = std::ifstream(glbFilename, std::ios::binary);

	binFile.seekg(12); //Skip past the 12 byte header, to the json header
	uint32_t jsonLength;
	binFile.read((char*)&jsonLength, sizeof(uint32_t)); //Read the length of the json file from it's header

	std::string jsonStr;
	jsonStr.resize(jsonLength);
	binFile.seekg(20); // Skip the rest of the JSON header to the start of the string
	binFile.read((char*)jsonStr.data(), jsonLength); // Read out the json string

											  // Parse the json
	Json::Reader reader;
	Json::Value root;


	if (print)
	{
		printf("Parsing json...\r\n");
	}

	if (!reader.parse(jsonStr, root))
		std::cerr << "Problem parsing assetData: " << jsonStr << std::endl;

	// After reading from the json, the file cusor will automatically be at the start of the binary header

	uint32_t binLength;
	binFile.read((char*)&binLength, sizeof(binLength)); // Read out the bin length from it's header
	binFile.seekg(sizeof(uint32_t), std::ios_base::cur); // skip chunk type

	std::vector<char> bin(binLength);
	binFile.read(bin.data(), binLength);


	gltf_loader(root, bin.data(), print);


	printf("Loading %s as single ibo/vbo\r\n", glbFilename);
	save_mesh(glbFilename, master_vertex_array, master_index_array);

	return 0;
}




typedef enum
{
	EXTERNAL_BIN,
	BASE64_BIN,
	FULL_BINARY
} glft_type_t;


int main(int argc, char *argv[])
{
	glft_type_t type;

	if (argc < 3)
	{
		printf("Usage %s <file> <type> [0 = external bin, 1 = base64 bin, 2 = glb]\r\n", argv[1]);
		return 0;
	}


	type = (glft_type_t)atoi(argv[2]);


	switch (type)
	{
	case EXTERNAL_BIN:
		gltf_load_external_bin(argv[1], true);
		break;
	case BASE64_BIN:
		gltf_load_base64_bin(argv[1], true);
		break;
	case FULL_BINARY:
		glb_load(argv[1], true);
		break;
	}


	return 0;
}