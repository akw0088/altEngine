#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "common.h"
#include "junzip.h"
#include "md5sum.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdarg.h> // for vargs
#include <math.h> // for ceil


#define DMESG_SIZE 256
extern char dmesg[DMESG_SIZE][1024];
extern int dmesg_index;

float newtonSqrt(float x);

extern "C" {
	void md5sum(char *data, int size, char *hash);
}



/*
Transforms leaf aabb to clip space
if any point within clip [-1,1] box
then visible
(might need to trace lines for odd cases of partially in
but vertex outside scenarios)

1. So view frustum converted to a regular square pyramid by
aspect ratio encoded in projection matrix

2. Square pyramid converted to a box by m[10] and m[14]
of projection matrix

3. Divide by w scales box to [-1,1] range for clip testing
*/
bool aabb_visible(vec3 &min, vec3 &max, matrix4 &mvp)
{
	vec4 aabb[8];
	bool visible = true;
	int point_visible = 0;

	//binary counting
	aabb[0] = mvp * vec4(min.x, min.y, min.z, 1.0f);
	aabb[1] = mvp * vec4(min.x, min.y, max.z, 1.0f);
	aabb[2] = mvp * vec4(min.x, max.y, min.z, 1.0f);
	aabb[3] = mvp * vec4(min.x, max.y, max.z, 1.0f);
	aabb[4] = mvp * vec4(max.x, min.y, min.z, 1.0f);
	aabb[5] = mvp * vec4(max.x, min.y, max.z, 1.0f);
	aabb[6] = mvp * vec4(max.x, max.y, min.z, 1.0f);
	aabb[7] = mvp * vec4(max.x, max.y, max.z, 1.0f);

	// Assume visible, if all points lay outside one plane, not visible
	for (int i = 0; i < 8; i++)
	{
		int xp = 0;
		int xn = 0;
		int yp = 0;
		int yn = 0;
		int zp = 0;
		int zn = 0;

		// Going to NDC as I was having issues with clipspace
		aabb[i].x = aabb[i].x / aabb[i].w;
		aabb[i].y = -aabb[i].y / aabb[i].w;
		aabb[i].z = aabb[i].z / aabb[i].w;

		aabb[i].x = 0.5f + (aabb[i].x * 0.5f);
		aabb[i].y = 0.5f + (aabb[i].y * 0.5f);



		if (aabb[i].x > 1.0)
		{
			xp = 1;
		}
		if (aabb[i].x < -1.0)
		{
			xn = 1;
		}
		if (aabb[i].y > 1.0)
		{
			yp = 1;
		}
		if (aabb[i].y < -1.0)
		{
			yn = 1;
		}
		/*
		if (aabb[i].z > aabb[i].w)
		{
			zp = 1;
		}
		if (aabb[i].z < -aabb[i].w)
		{
			zn = 1;
		}
		*/

		// point weas outside of view
		if ((xp + xn + yp + yn + zp + zn) > 0)
		{
			point_visible++;
		}
	}


	if (point_visible == 8)
		visible = false;

	return visible;
}

bool RayTriangleMT(vec3 &origin, vec3 &dir, vec3 &a, vec3 &b, vec3 &c, float &t, float &u, float &v)
{
	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 pvec = vec3::crossproduct(dir, ac);

	float det = ab * pvec;

	// ray and triangle are parallel if det is close to 0
	if (abs32(det) < 0.001f)
		return false;

	float invDet = 1 / det;

	vec3 tvec = origin - a;

	u = (tvec * pvec) * invDet;

	if (u < 0 || u > 1)
		return false;

	vec3 qvec = vec3::crossproduct(tvec, ab);
	v = (dir * qvec) * invDet;

	if (v < 0 || u + v > 1)
		return false;

	t = (ac * qvec) * invDet;

	return true;
}

bool RaySphere(vec3 &origin, vec3 &dir, vec3 &sphere, float radius, float &t)
{
	vec3 dist = sphere - origin;
	float B = dir * dist;
	float D = B * B - dist * dist + radius * radius;

	if (D < 0.0f)
		return false;

	float t0 = B - newtonSqrt(D);
	float t1 = B + newtonSqrt(D);

	bool ret = false;

	if ((t0 > 0.1f) && (t0 < t))
	{
		t = t0;
		ret = true;
	}
	if ((t1 > 0.1f) && (t1 < t))
	{
		t = t1;
		ret = true;
	}
	return ret;
}

//intersect ray plane
bool RayPlane(vec3 &origin, vec3 &dir, vec3 &normal, float d, vec3 &point)
{
	float denom = dir * normal;
	float time;

	if (abs32(denom) <= 0.0001f)
	{
		return false;
	}

	time = -(origin * normal + d) / denom;
	point = origin + dir * time;
	return true;
}

/*
Intersect ray with aabb planes
get tmin and tmax values for each pair
can compare interval with other pairs and determine
if hit or miss occurs
*/
bool RayBoxSlab(vec3 &origin, vec3 &dir, vec3 &min, vec3 &max, float &distance)
{
	float tmin = -10000, tmax = 10000;

	// X coordinate
	if (abs32(dir.x) <= 0.0001f)
	{
		float t1 = (min.x - origin.x) / dir.x;
		float t2 = (max.x - origin.x) / dir.x;

		tmin = MAX(tmin, MIN(t1, t2));
		tmax = MIN(tmax, MAX(t1, t2));
	}

	// Y coordinate
	if (abs32(dir.y) <= 0.0001f)
	{
		float t1 = (min.y - origin.y) / dir.y;
		float t2 = (max.y - origin.y) / dir.y;

		tmin = MAX(tmin, MIN(t1, t2));
		tmax = MIN(tmax, MAX(t1, t2));
	}

	// Z coordinate
	if (dir.z != 0.0)
	{
		float t1 = (min.z - origin.z) / dir.z;
		float t2 = (max.z - origin.z) / dir.z;

		tmin = MAX(tmin, MIN(t1, t2));
		tmax = MIN(tmax, MAX(t1, t2));
	}


	if (tmax > tmin && tmax > 0.0)
	{
		distance = tmax;
		return true;
	}
	else
	{
		return false;
	}
}


// Watch this for the lerps to make sense
// https://acko.net/tv/wdcode/

//lerp put into header due to release mode linkage issue

// Lerp between A and C, curving towards B
void quadratic_bezier_curve(vec3 &a, vec3 &b, vec3 &c, float time, vec3 &out)
{
	vec3 temp1, temp2;

	lerp(a, b, time, temp1);
	lerp(b, c, time, temp2);
	lerp(temp1, temp2, time, out);
}

// Lerp between A and D, curving towards B and C
void cubic_bezier_curve(vec3 &a, vec3 &b, vec3 &c, vec3 &d, float time, vec3 &out)
{
	vec3 temp1, temp2, temp3;
	vec3 intermediate1, intermediate2;

	lerp(a, b, time, temp1);
	lerp(b, c, time, temp2);
	lerp(c, d, time, temp3);

	lerp(temp1, temp2, time, intermediate1);
	lerp(temp2, temp3, time, intermediate2);
	lerp(intermediate1, intermediate2, time, out);
}

// Create surface given 16 control points
// ie: four curves with 4 points each trace X
// output from those four points create new curve
// New curve used to generate Y
// Four corner points (0, 3, 12, 15), rest control curvature
void bicubic_bezier_surface(vec3 *control, float time_x, float time_y, vec3 &out)
{
	vec3 temp1, temp2, temp3, temp4;

	cubic_bezier_curve(control[0], control[1], control[2], control[3], time_x, temp1);
	cubic_bezier_curve(control[4], control[5], control[6], control[7], time_x, temp2);
	cubic_bezier_curve(control[8], control[9], control[10], control[11], time_x, temp3);
	cubic_bezier_curve(control[12], control[13], control[14], control[15], time_x, temp4);

	cubic_bezier_curve(temp1, temp2, temp3, temp4, time_y, out);
}


// Same as above, but using 9 control points and quadratic lines
void quadratic_bezier_surface(vec3 *control, float time_x, float time_y, vec3 &out)
{
	vec3 temp1, temp2, temp3;
	vec3 intermediate1, intermediate2;


	quadratic_bezier_curve(control[0], control[1], control[2], time_x, temp1);
	quadratic_bezier_curve(control[3], control[4], control[5], time_x, temp2);
	quadratic_bezier_curve(control[6], control[7], control[8], time_x, temp3);

	quadratic_bezier_curve(temp1, temp2, temp3, time_y, out);
}




// Seems to work, should probably be <= num_row though on loops
void tessellate_quadratic_bezier_surface(vec3 *control, vertex_t *&vertex, int *&index, int &num_vertex, int &num_index, float level)
{
	int num_row = (int)ceil(1.0 / level);

	int x = 0;
	int y = 0;
	int i = 0;

	// generate all vertices
	for (y = 0; y < num_row; y++)
	{
		for (x = 0; x < num_row; x++)
		{
			quadratic_bezier_surface(control, x * level, y * level, vertex[i++].position);
		}
	}
	num_vertex = i;

	// generate index array
	int j = 0;
	for (i = 0; j < num_row * (num_row - 1);)
	{
		// Dont connect top of row to bottom of next row
		if ((j + 1) % (num_row) == 0)
		{
			j++;
			continue;
		}

		index[i + 0] = j;
		index[i + 1] = num_row + j;
		index[i + 2] = j + 1;

		index[i + 3] = num_row + j;
		index[i + 4] = num_row + j + 1;
		index[i + 5] = j + 1;

		// generate normals
		vec3 a = vertex[index[i + 1]].position - vertex[index[i + 0]].position;
		vec3 b = vertex[index[i + 2]].position - vertex[index[i + 0]].position;
		vec3 normal = vec3::crossproduct(a, b);

		vertex[index[i + 0]].normal = normal;
		vertex[index[i + 1]].normal = normal;
		vertex[index[i + 2]].normal = normal;

		vertex[index[i + 3]].normal = normal;
		vertex[index[i + 4]].normal = normal;
		vertex[index[i + 5]].normal = normal;

		j++;
		i += 6;
	}
	num_index = i;
}

int debugf(const char *format, ...)
{
    va_list args;
    char str[512] = { 0 };
	unsigned int width = 60;

    
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);
    printf("%s", str);
    
    
    char *pstr = str;
    while (1)
    {
        if (strlen(pstr) < width)
        {
            char *line = dmesg[dmesg_index++];
		    if (dmesg_index >= DMESG_SIZE)
				dmesg_index  = 0;

			sprintf(line, "%s", pstr);
		    //line[size] = '\0';
            Menu::console_buffer.push_back(line);
            break;
        }
        else
        {
            int size = width + 1;
            char *line = dmesg[dmesg_index++];;
			if (dmesg_index >= DMESG_SIZE)
				dmesg_index = 0;

			memcpy(line, pstr, size);
            line[width] = '\0';
            Menu::console_buffer.push_back(line);
            pstr += width;
        }	
	}
    
    return 0;
}

char *get_file(char *filename, int *size)
{
    FILE	*file;
    char	*buffer;
    int	file_size, bytes_read;
    
    file = fopen(filename, "rb");
    if (file == NULL)
        return 0;
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = new char [file_size + 1];
    bytes_read = (int)fread(buffer, sizeof(char), file_size, file);
    if (bytes_read != file_size)
    {
		delete [] buffer;
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

int write_file(char *filename, const char *bytes, int size)
{
    FILE *fp = fopen(filename, "wb");
    int ret;
    
    if (fp == NULL)
    {
        perror("Unable to open file for writing");
        return -1;
    }
    
    ret = fwrite(bytes, sizeof(char), size, fp);
    
    if (ret != size)
    {
        printf("fwrite didnt write all data\n");
	fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

int processFile(JZFile *zip, userdata_t *user)
{
	JZFileHeader header;
	char filename[1024];
	unsigned char *data;

	if (jzReadLocalFileHeader(zip, &header, filename, sizeof(filename)))
	{
		printf("Couldn't read local file header!");
		return -1;
	}

	if ((data = (unsigned char *)malloc(header.uncompressedSize + 1)) == NULL)
	{
		printf("Couldn't allocate memory!");
		return -1;
	}

	//printf("%s, %d / %d bytes at offset %08X\n", filename, header.compressedSize, header.uncompressedSize, header.offset);

	if (jzReadData(zip, &header, data) != Z_OK)
	{
		printf("Couldn't read file data!");
		free(data);
		return -1;
	}

	user->data = (unsigned char *)data;
	user->size = header.uncompressedSize;

	return 0;
}

int recordCallback(JZFile *zip, int idx, JZFileHeader *header, char *filename, void *user_data)
{
	long offset;
	userdata_t *user = (userdata_t *)user_data;

	if (strcmp(filename, user->file) != 0)
	{
		return 1;
	}

	//printf("Found file %s\n", filename);
	offset = zip->tell(zip); // store current position

	if (zip->seek(zip, header->offset, SEEK_SET))
	{
		printf("Cannot seek in zip file!");
		return 0; // abort
	}

	processFile(zip, user); // alters file offset
	zip->seek(zip, offset, SEEK_SET); // return to position

	return 0; // continue
}

int listCallback(JZFile *zip, int idx, JZFileHeader *header, char *filename, void *filelist)
{
	strcat((char *)filelist, filename);
	strcat((char *)filelist, "\n");
	return 1; // continue
}

int get_zipfile(char *zipfile, char *file, unsigned char **data, int *size)
{
	FILE *fp;
	JZEndRecord endRecord;
	JZFile *zip;
	int retval = -1;
	userdata_t user;

	user.file = file;
	user.data = NULL;
	user.size = 0;

	fp = fopen(zipfile, "rb");
	if (fp == NULL)
	{
//		printf("Couldn't open zip file %s\n", zipfile);
		return -1;
	}

	zip = jzfile_from_stdio_file(fp);

	if (jzReadEndRecord(zip, &endRecord))
	{
		printf("Couldn't read ZIP file end record.");
		zip->close(zip);
		return retval;
	}

	if (jzReadCentralDirectory(zip, &endRecord, recordCallback, (void *)&user))
	{
		printf("Couldn't read ZIP file central record.");
		zip->close(zip);
		return retval;
	}

	*data = user.data;
	if (size != NULL)
	{
		*size = user.size;
	}
	retval = 0;
	zip->close(zip);
	return retval;
}

int list_zipfile(char *zipfile, char *filelist)
{
	FILE *fp;
	JZEndRecord endRecord;
	JZFile *zip;
	int retval = -1;

	fp = fopen(zipfile, "rb");
	if (fp == NULL)
	{
		//		printf("Couldn't open zip file %s\n", zipfile);
		return -1;
	}

	zip = jzfile_from_stdio_file(fp);

	if (jzReadEndRecord(zip, &endRecord))
	{
		printf("Couldn't read ZIP file end record.");
		zip->close(zip);
		return retval;
	}

	if (jzReadCentralDirectory(zip, &endRecord, listCallback, filelist))
	{
		printf("Couldn't read ZIP file central record.");
		zip->close(zip);
		return retval;
	}

	retval = 0;
	zip->close(zip);
	return retval;
}

void newlinelist(char *filename, char **list, int &num)
{
	if (filename == NULL || list == NULL)
	{
		printf("newlinelist given null values\n");
		return;
	}

	char *file = get_file(filename, NULL);
	if (file == NULL)
	{
		printf("Unable to open %s\n", filename);
		return;
	}

	num = 0;

	char *line = strtok(file, "\n");
	while (line)
	{
		if (strlen(line) >= 2 && line[0] != '/' && line[1] != '/')
		{
			int last = strlen(line) - 1;
			if (line[last] == '\r')
				line[last] = '\0';
			list[num++] = line;
		}
		line = strtok(NULL, "\n");
	}
}

int load_texture_pk3(Graphics &gfx, char *file_name, char **pk3_list, int num_pk3, bool clamp, bool bgr)
{
	int width, height, components, format;
	int tex_object;

	int size = 0;
	unsigned char *data = NULL;
	char pk3_name[1024];

	memset(pk3_name, 0, sizeof(pk3_name));
	sprintf(pk3_name, "%s", file_name + strlen("media/"));

	for (int i = 0; i < num_pk3; i++)
	{
		get_zipfile(pk3_list[i], pk3_name, &data, &size);
		if (data != NULL)
			break;
	}

	// tga failed tried jpg
	if (data == NULL)
	{
		if (strlen(pk3_name) > 4)
		{
			char *ext = strstr(pk3_name, ".tga");
			if (ext != NULL)
			{
				*ext = '\0';
				strcat(pk3_name, ".jpg");
				//printf("Trying jpeg texture [%s]\n", texture_name);

				for (int i = 0; i < num_pk3; i++)
				{
					get_zipfile(pk3_list[i], pk3_name, &data, &size);
					if (data != NULL)
						break;
				}
			}
		}

	}

	if (data == NULL)
	{
		debugf("Unable to load texture %s from pk3\n", file_name);
		return load_texture(gfx, file_name, clamp, false);
	}
	else
	{
		//debugf("Loaded %s from pk3\n", file_name);
	}



#ifdef OPENGL32
	unsigned char *bytes = stbi_load_from_memory(data, size, &width, &height, &components, 0);


	if (components == 4)
	{
		format = GL_RGBA;
		components = GL_RGBA8;
	}
	else if (components == 3)
	{
		format = GL_RGB;
		components = GL_RGB8;
	}
	else if (components == 1)
	{
		format = GL_RED;
		components = GL_RED;
	}
	else
	{
		printf("Unknown component: %s %d\n", file_name, components);
		stbi_image_free(bytes);
		free((void *)data);
		return 0;
	}
#endif
#ifdef DIRECTX
	unsigned char *bytes = stbi_load_from_memory(data, size, &width, &height, &components, 0);
	byte *pBits = NULL;
	if (components == 3)
	{
		pBits = tga_24to32(width, height, (byte *)bytes, bgr);
	}


	format = 4;
//	components = 4;
#endif

#if 0
	if (format == GL_RGB)
	{
		unsigned char *normal = new unsigned char[width * height * 4];
		gen_normalmap(1.0f, (pixel_t *)bytes, (pixel_t *)normal, width, height);
		tex_object = gfx.LoadTexture(width, height, components, format, normal, clamp);
	}
	else
	{
		tex_object = gfx.LoadTexture(width, height, components, format, bytes, clamp);
	}
#endif
#ifdef DIRECTX
	if (components == 3)
		tex_object = gfx.LoadTexture(width, height, 4, format, pBits, clamp);
	else
		tex_object = gfx.LoadTexture(width, height, components, format, bytes, clamp);
#endif
#ifdef OPENGL32
	tex_object = gfx.LoadTexture(width, height, components, format, bytes, clamp);
#endif
#ifdef VULKAN
	format = -1;
	unsigned char *bytes = stbi_load_from_memory(data, size, &width, &height, &components, 0);
	tex_object = gfx.LoadTexture(width, height, components, format, bytes, clamp);
#endif



#ifndef DEDICATED
	stbi_image_free(bytes);
	free((void *)data);
#endif

#ifdef OPENGL32
	if (format != GL_RGBA)
	{
		// negative means it has an alpha channel
		return -tex_object;
	}
#endif
	return tex_object;
}

// Need asset manager class so things arent doubly loaded
int load_texture(Graphics &gfx, char *file_name, bool clamp, bool bgr)
{
	int width = 0;
	int height = 0;
	int components = 0;
	int format = 0;
	int tex_object = -1;

	int size = 0;
	unsigned char *data = (unsigned char *)get_file(file_name, &size);

	if (data == NULL)
	{
//		debugf("Unable to load texture %s\n", file_name);
		return 0;
	}
	else
	{
		//debugf("Loaded %s from pk3\n", file_name);
	}

	//tex_object[face->material].texObj[0]
#ifdef OPENGL32
	unsigned char *bytes = stbi_load_from_memory(data, size, &width, &height, &components, 0);

	if (components == 4)
	{
		format = GL_RGBA;
		components = GL_RGBA8;
	}
	else if (components == 3)
	{
		format = GL_RGB;
		components = GL_RGB8;
	}
	else if (components == 1)
	{
		format = GL_RED;
		components =  GL_RED;
	}
	else
	{
		printf("Unknown component: %s %d\n", file_name, components);
		stbi_image_free(bytes);
		delete [] data;
		return 0;
	}
#endif
#ifdef DIRECTX
	unsigned char *bytes = stbi_load_from_memory(data, size, &width, &height, &components, 0);
	format = 4;
//	components = 4;
#endif
#ifdef VULKAN
	format = -1;
	unsigned char *bytes = stbi_load_from_memory(data, size, &width, &height, &components, 0);
#endif
#ifndef DEDICATED
	byte *pBits = NULL;
	if (components == 3)
	{
		pBits = tga_24to32(width, height, (byte *)bytes, bgr);
	}
#endif
#ifndef DEDICATED
	if (components == 3)
	{
		tex_object = gfx.LoadTexture(width, height, 4, format, pBits, clamp);
	}
	else
	{
		tex_object = gfx.LoadTexture(width, height, components, format, bytes, clamp);
	}
	stbi_image_free(bytes);
	delete [] data;
#endif
#ifdef OPENGL32
	if (format != GL_RGBA)
	{
		// negative means it has an alpha channel
		return -tex_object;
	}
#endif
	return tex_object;
}

bool check_hash(char *filename, char *md5match, char *hash)
{
	int size = 0;

	char *data = get_file(filename, &size);
	if (data == NULL)
	{
		sprintf(hash, "missing file");
		return false;
	}

	memset(hash, 0, 17);
	md5sum(data, size, hash);
	delete [] data;
	if (strcmp(hash, md5match) == 0)
		return true;
	else
		return false;
}

byte *tga_24to32(int width, int height, byte *pBits, bool bgr)
{
	int lImageSize = width * height * 4;
	byte *pNewBits = new byte[lImageSize * sizeof(byte)];

	for (int i = 0, j = 0; i < lImageSize; i += 4)
	{
		if (bgr)
		{
			pNewBits[i + 2] = pBits[j++];
			pNewBits[i + 1] = pBits[j++];
			pNewBits[i + 0] = pBits[j++];
		}
		else
		{
			pNewBits[i + 0] = pBits[j++];
			pNewBits[i + 1] = pBits[j++];
			pNewBits[i + 2] = pBits[j++];
		}
		pNewBits[i + 3] = 0;
	}
	return pNewBits;
}

void navdata_to_graph(ref_t *&ref, graph_node_t *&node, vector<Entity *> &entity_list, int start)
{
	int num_node = entity_list.size() - start;
	char data[512] = { 0 };
	int j = 0;

	node = new graph_node_t[num_node];
	ref = new ref_t[num_node];

	for (unsigned int i = start; i < entity_list.size(); i++)
	{
		if (strcmp(entity_list[i]->type, "navpoint") == 0)
		{
			Entity *ent = entity_list[i];
			int k = 0;
			int targetname = atoi(entity_list[i]->target_name + 3);
			node[j].num_arcs = 0;

			// Manhattan distance table
			ref[k].x = (int)ent->position.x;
			ref[k].y = (int)ent->position.y;
			ref[k].z = (int)ent->position.z;

			strcpy(data, ent->target);


			char *target = strtok(data, " ");
			while (target != NULL)
			{
				int target_val = atoi(target + 3);

				//skip past characters eg: "nav0"
				node[j].arc[k].a = targetname;
				node[j].arc[k].b = target_val;
				node[j].arc[k].weight = 1.0f;
				node[j].num_arcs = ++k;
				target = strtok(NULL, " ");
				if (k == 8)
					break;
			}
			j++;
		}
	}
}

void print_graph(graph_node_t *node, int num_node)
{
	for (int i = 0; i < num_node; i++)
	{
		if (node[i].num_arcs == 0)
			continue;

		printf("node %d: nav%d\n", i, node[i].arc[0].a);
		for (int j = 0; j < node[i].num_arcs; j++)
		{
			printf("\tPath from nav%d to nav%d weight %3.3f\n",
				node[i].arc[j].a, node[i].arc[j].b, node[i].arc[j].weight);
		}
	}
}

void print_path(int *path, int path_length, graph_node_t *node)
{
	for (int i = 0; i < path_length; i++)
	{
		printf("go to node %d nav%d\n", path[i], node[path[i]].arc[0].a);
	}
}


float rand_float(float fMin, float fAdd)
{
	float fRandom = (float)(rand() % RAND_MAX) / (RAND_MAX - 1);
	return fMin + fAdd * fRandom;
}

unsigned int crc32_byte(unsigned int r)
{
	for (int j = 0; j < 8; ++j)
		r = (r & 1 ? 0 : (unsigned int)0xEDB88320L) ^ r >> 1;
	return r ^ (unsigned int)0xFF000000L;
}

void crc32(const void *data, unsigned int n_bytes, unsigned int* crc)
{
	static unsigned int table[0x100];
	if (!*table)
	{
		for (unsigned int i = 0; i < 0x100; ++i)
		{
			table[i] = crc32_byte(i);
		}
	}
	for (unsigned int i = 0; i < n_bytes; ++i)
	{
		*crc = table[(char)*crc ^ ((char*)data)[i]] ^ *crc >> 8;
	}
}

int spiral(float distance, vec3 &scale, float step, vec3 *point)
{
	int i;


	for(i = 0; i * step < distance; i++)
	{
		float sin_val = (float)fsin(i * step);
		float cos_val = (float)fcos(i * step);
		point[i].x = scale.x * (cos_val - sin_val);
		point[i].y = -scale.y * (sin_val + cos_val);
		point[i].z = scale.z * i * step;
	}

	return i;
}

int gen_spiral(Graphics &gfx, unsigned int &ibo, unsigned int &vbo)
{
	static vertex_t vert[512];
	vec3 point[512];
	int index[512];
	vec3 scale(5.0f, 5.0f, 20.0f);
	vec3 offset(0.0f, 0.0f, -12.0f);

	int num_point = spiral(100.0f, scale, 0.25f, point);

	for(int i = 0; i < num_point; i++)
	{
		memset(&vert[i], 0, sizeof(vertex_t));
		vert[i].position = point[i] + offset;
		vert[i].color = 0xFF000000;
		vert[i].tangent.x = 3.5f; //life
		vert[i].tangent.y = 5.0f; //size
		vert[i].tangent.z = -1.0f; //type
		index[i] = i;
	}

	ibo = gfx.CreateIndexBuffer(index, num_point);
	vbo = gfx.CreateVertexBuffer(vert, num_point);
	return 0;
}


int lightning(float distance, vec3 &scale, float step, vec3 *point)
{
	int i;

	for (i = 0; i * step < distance; i++)
	{
		vec2 value;
		float x = (float)(0.5 * fsin((i * step) * 30));
		point[i].x = (float)(1.0 - 2.0 * abs32(sign(x) - x));
		point[i].y = point[i].x;
		point[i].z = scale.z * i * step;
	}

	return i;
}

int gen_lightning(Graphics &gfx, unsigned int &ibo, unsigned int &vbo)
{
	static vertex_t vert[512];
	vec3 point[512];
	int index[512];
	vec3 scale(29.0f, 20.0f, 5.0f);
	vec3 offset(-10.0f, -10.0f, -12.0f);

	int num_point = lightning(100.0f, scale, 0.25f, point);

	for (int i = 0; i < num_point; i++)
	{
		memset(&vert[i], 0, sizeof(vertex_t));
		vert[i].position = point[i] + offset;
		vert[i].color = 0xFF000000;
		vert[i].tangent.x = 3.5f; //life
		vert[i].tangent.y = 5.0f; //size
		vert[i].tangent.z = -1.0f; //type
		index[i] = i;
	}

	ibo = gfx.CreateIndexBuffer(index, num_point);
	vbo = gfx.CreateVertexBuffer(vert, num_point);
	return 0;
}

int trim(char *data, int length)
{
	int pos = 0;
	for (int i = 0; i < length; i++)
	{
		char c = data[i];

		if (c == '\r' || c == '\n' || c == ' ')
		{
			continue;
		}
		data[pos++] = c;
	}
	return pos;
}

void delta_compress(char *output, char *input, char *delta, int size)
{
        int i = 0;

        for (i = 0; i < size; i++)
        {
                output[i] = input[i] - delta[i];
        }
}

void delta_uncompress(char *output, char *input, char *delta, int size)
{
        int i = 0;

        for (i = 0; i < size; i++)
        {
                output[i] = input[i] + delta[i];
        }
}

void runlength_encode(uint8_t *output, rletable_t *table, int *table_size, uint8_t *input, unsigned int *size)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int length = 0;

	int input_size = *size;

        for (i = 0; i < input_size; i++)
        {
                if (input[i] == input[i + 1])
                {
                        length++;
                        continue;
                }
                else
                {
                        if (length > 0)
                        {
                                table[k].length = length;
                                table[k].pos = i - length;
                                k++;
                        }
                        length = 0;
                }

                output[j++] = input[i];
        }
	*size = j;
	*table_size = k;
}

void runlength_decode(uint8_t *output, rletable_t *table, uint8_t *input, unsigned int *size)
{
        int i = 0;
        int j = 0;
	int input_size = *size;

        for (i = 0; i < input_size; i++)
        {
                int t = 0;

                for (t = 0; ;t++)
                {
                        if (table[t].length == 0)
                                break;

                        if (j == table[t].pos)
                        {
                                int count = table[t].length;

                                while (count)
                                {
                                        output[j++] = input[i];
                                        count--;
                                }
                                break;
                        }
                }

                output[j++] = input[i];
        }

	*size = j;
}

float GetLuminance(vec3 &v)
{
	// greyscale conversion
//	return v.x * 0.2126f + v.y * 0.7152f + v.z * 0.0722f;

	return (v.x + v.y + v.z) / 3.0f;
}



vec3 ColorToVector(const pixel_t &color)
{
	vec3 vcolor;


	//high byte is alpha
//	vcolor.z = (float)(((color.a) >> 24) / 255.0f);
	vcolor.x = (float)(color.r / 255.0f);
	vcolor.y = (float)(color.g / 255.0f);
	vcolor.z = (float)(color.b / 255.0f);

	return vcolor;
}

pixel_t VectorToColor(vec3 &v)
{
	pixel_t color;

	memset(&color, 0, sizeof(pixel_t));

	//byte a = 255;
	byte r = (byte)(255.0f * v.x);
	byte g = (byte)(255.0f * v.y);
	byte b = (byte)(255.0f * v.z);


	color.r = r;
	color.g = g;
	color.b = b;
//	color.a = a;

	return color;
}


void gen_normalmap(float scale, const pixel_t *pixel, pixel_t *pixelout, int width, int height)
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{

			vec3 topleft		= ColorToVector(pixel[MAX(x - 1, 0)			+ MIN(y + 1, height - 1) * width]);
			vec3 top			= ColorToVector(pixel[x						+ MIN(y + 1, height - 1) * width]);
			vec3 topright		= ColorToVector(pixel[MIN(x + 1, width - 1) + MIN(y + 1, height - 1) * width]);
			vec3 left			= ColorToVector(pixel[MAX(x - 1, 0)			+ y * width]);
			vec3 center			= ColorToVector(pixel[x						+ y * width]);
			vec3 right			= ColorToVector(pixel[MIN(x + 1, width - 1) + y * width]);
			vec3 bottomleft		= ColorToVector(pixel[MAX(x - 1, 0)			+ MAX(y - 1, 0) * width]);
			vec3 bottom			= ColorToVector(pixel[x						+ MAX(y - 1, 0) * width]);
			vec3 bottomright	= ColorToVector(pixel[MIN(x + 1, width - 1) + MAX(y - 1, 0) * width]);

			float centerL		= GetLuminance(center);
			float leftL			= GetLuminance(left);
			float rightL		= GetLuminance(right);
			float bottomL		= GetLuminance(bottom);
			float topL			= GetLuminance(top);
			float topleftL		= GetLuminance(topleft);
			float toprightL		= GetLuminance(topright);
			float bottomleftL	= GetLuminance(bottomleft);
			float bottomrightL	= GetLuminance(bottomright);


			// finite difference method
//			The x component of the normal is luminance(i - 1, j) - luminance(i + 1, j)
//			The y component of the normal is luminance(i, j - 1) - luminance(i, j + 1)
//			And then we can find the z component as sqrt(1 - x*x - y*y)

//			vec3 normal;
//			normal.x = leftL - rightL;
//			normal.y = bottomL - topL;
//			normal.z = sqrt(1 - normal.x * normal.x - normal.y * normal.y);

			/*
			vec3 tangent1(1.0f, 0.0f, centerL - leftL);
			vec3 tangent2(1.0f, 0.0f, rightL - centerL);
			vec3 bitangent1(0.0f, 1.0f, centerL - bottomL);
			vec3 bitangent2(0.0f, 1.0f, topL - centerL);

			tangent1 = tangent1.normalize();
			tangent2 = tangent2.normalize();
			bitangent1 = bitangent1.normalize();
			bitangent2 = bitangent2.normalize();
			vec3 tangent = tangent1 + tangent2;
			vec3 bitangent = bitangent1 + bitangent2;

			vec3 normal = vec3::crossproduct(tangent, bitangent);
			normal = normal.normalize();

			*/

			vec3 normal;

			//Sobel_dx
			// 1 0 -1
			// 2 0 -2
			// 1 0 -1

			//Sobel_dy
			//  1  2  1
			//  0  0  0
			// -1 -2 -1

			double kernelx[9] = { 0, 0, 0,
								0, 1, 0,
								  0, 0, 0 };

			double kernely[9] = {  0,  0,  0,
									0,  1,  0,
								   0,  0,  0 };


			normal.x = (float)(topleftL * kernelx[0] + topL * kernelx[1] + toprightL * kernelx[2]  +
				leftL * kernelx[3] + centerL * kernelx[4] + rightL * kernelx[5] +
				bottomleftL * kernelx[6] + bottomL * kernelx[7] + bottomrightL * kernelx[8]);

			normal.y = (float)(topleftL * kernely[0] + topL * kernely[1] + toprightL * kernely[2] +
				leftL * kernely[3] + centerL * kernely[4] + rightL * kernely[5] +
				bottomleftL * kernely[6] + bottomL * kernely[7] + bottomrightL * kernely[8]);

			normal.z = normal.y;
//			normal.z = 1.0f / scale;


//			normal.x = centerL;
//			normal.y = centerL;
//			normal.z = centerL;

//			normal.x *= 0.5f;
//			normal.y *= 0.5f;
//			normal.x += 0.5f;
//			normal.y += 0.5f;

			pixel_t color = VectorToColor(normal);

			pixelout[x + y * width] = color;
//			pixelout[x + y * width] = pixel[x + y * width];
		}
	}
}
