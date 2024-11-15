//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

// This file is really a dumping ground of random C functions

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
#ifndef WIN32
#define stricmp strcasecmp
#include <netdb.h>
#endif

#define DMESG_SIZE 2048
extern char dmesg[DMESG_SIZE][1024];
extern int dmesg_index;

float newtonSqrt(float x);
int fceil(int num, int denom);


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
		if (abs32(aabb[i].w) < 0.00001f )
			continue;

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
	if (abs32(dir.x) > 0.0001f)
	{
		float t1 = (min.x - origin.x) / dir.x;
		float t2 = (max.x - origin.x) / dir.x;

		tmin = MAX(tmin, MIN(t1, t2));
		tmax = MIN(tmax, MAX(t1, t2));
	}

	// Y coordinate
	if (abs32(dir.y) > 0.0001f)
	{
		float t1 = (min.y - origin.y) / dir.y;
		float t2 = (max.y - origin.y) / dir.y;

		tmin = MAX(tmin, MIN(t1, t2));
		tmax = MIN(tmax, MAX(t1, t2));
	}

	// Z coordinate
	if (abs32(dir.z) > 0.0001f)
	{
		float t1 = (min.z - origin.z) / dir.z;
		float t2 = (max.z - origin.z) / dir.z;

		tmin = MAX(tmin, MIN(t1, t2));
		tmax = MIN(tmax, MAX(t1, t2));
	}


	if (tmax > tmin && tmax > 0.0f)
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
void tessellate_quadratic_bezier_surface(vec3 *control, vertex_t *&vertex, int *&index, int &num_vertex, int &num_index, int level)
{
	//x/y + (x % y != 0);
	//int num_row = fceil(1, level); // Doesnt this mean num_row always equals one?
	int num_row = level + 1;
	int x = 0;
	int y = 0;
	int i = 0;

	// generate all vertices
	for (y = 0; y < num_row; y++)
	{
		for (x = 0; x < num_row; x++)
		{
			quadratic_bezier_surface(control, (float)(x * level), (float)(y * level), vertex[i++].position);
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

    
	memset(str, 0, sizeof(str));
    va_start(args, format);
    vsnprintf(str, 511, format, args);
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

typedef struct
{
	char *buffer;
	int size;
} fileinfo_t;


#ifdef DEBUG_FILELOAD
vector<std::pair<char *, fileinfo_t>> file_list;
#include <thread>
#include <mutex>
std::mutex mtx;
#endif

char *get_file(char *filename, unsigned int *size)
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

#ifdef DEBUG_FILELOAD
	std::pair<char *, fileinfo_t> pair;
	std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
	lck.lock();
	pair.first = new char[512];
	strcpy(pair.first, filename);
	pair.second.buffer = buffer;
	pair.second.size = file_size;
	file_list.push_back(pair);
	lck.unlock();
#endif

	return buffer;
}

#ifdef DEBUG_FILELOAD
void delete_file(char *fileptr)
{
	for (unsigned int i = 0; i < file_list.size(); i++)
	{
		if (fileptr == file_list[i].second.buffer)
		{
			delete[] fileptr;
			break;
		}
	}
}

void print_file()
{
	for (unsigned int i = 0; i < file_list.size(); i++)
	{
		printf("File %d size %d\t\t%s\r\n", i, file_list[i].second.size, file_list[i].first);
		delete[] file_list[i].first;
	}
}
#endif

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

	data = (unsigned char *) new unsigned char [header.uncompressedSize + 1];

	//printf("%s, %d / %d bytes at offset %08X\n", filename, header.compressedSize, header.uncompressedSize, header.offset);

	if (jzReadData(zip, &header, data) != Z_OK)
	{
		printf("Couldn't read file data!");
		delete [] data;
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

	if (stricmp(filename, user->file) != 0)
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

int get_zipfile(char *zipfile, char *file, unsigned char **data, unsigned int *size)
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

void newlinelist(char *filename, char **list, unsigned int &num, char **file)
{
	if (filename == NULL || list == NULL)
	{
		printf("newlinelist given null values\n");
		return;
	}

	*file = get_file(filename, NULL);
	if (*file == NULL)
	{
		printf("Unable to open %s\n", filename);
		return;
	}

	num = 0;

	char *line = strtok(*file, "\n");
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

int load_texture_pk3(Graphics &gfx, char *file_name, char **pk3_list, int num_pk3, bool clamp, bool bgr, int anisotropic)
{
	int width, height, components, format;
	int tex_object = -1;

	unsigned int size = 0;
	unsigned char *data = NULL;
	char pk3_name[1024];

	memset(pk3_name, 0, sizeof(pk3_name));
	sprintf(pk3_name, "%s", file_name + strlen("media/"));

	for (int i = 0; i < num_pk3; i++)
	{
		get_zipfile(pk3_list[i] + FILE_OFFSET, pk3_name, &data, &size);
		if (data != NULL)
			break;
	}


	if (data == NULL)
		data = (unsigned char *)get_file(file_name, &size);

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
					get_zipfile(pk3_list[i] + FILE_OFFSET, pk3_name, &data, &size);
					if (data != NULL)
						break;
				}
			}
		}

	}


	// tga failed tried jpg
	if (data == NULL)
	{
		if (strlen(file_name) > 4)
		{
			char *ext = strstr(file_name, ".tga");
			if (ext != NULL)
			{
				*ext = '\0';
				strcat(file_name, ".jpg");
				//printf("Trying jpeg texture [%s]\n", texture_name);
				if (data == NULL)
					data = (unsigned char *)get_file(file_name, NULL);

			}
		}

	}




	if (data == NULL)
	{
		debugf("Unable to load texture %s from pk3\n", file_name);
		return load_texture(gfx, file_name, clamp, false, anisotropic);
	}
	else
	{
		//debugf("Loaded %s from pk3\n", file_name);
	}



#ifdef OPENGL
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
#ifdef _DEBUG
		printf("Unknown component: %s %d\n", file_name, components);
#endif
		stbi_image_free(bytes);
		delete [] data;
		return 0;
	}
#endif
#ifdef SOFTWARE
	unsigned char *bytes = stbi_load_from_memory(data, size, &width, &height, &components, 0);
	if (bytes == NULL)
	{
		debugf("Unable to load texture %s from pk3\n", file_name);
		delete [] data;
		return load_texture(gfx, file_name, clamp, false, anisotropic);
	}

	char *pBits = NULL;
	if (components == 3)
	{
		pBits = tga_24to32(width, height, (char *)bytes, bgr);
		components = 5;
	}


	format = 4;
//	components = 4;
#endif
#ifdef DIRECTX
	unsigned char *bytes = stbi_load_from_memory(data, size, &width, &height, &components, 0);
	if (bytes == NULL)
	{
		debugf("Unable to load texture %s from pk3\n", file_name);
		delete[] data;
		return load_texture(gfx, file_name, clamp, false, anisotropic);
	}

	char *pBits = NULL;
	if (components == 3)
	{
		pBits = tga_24to32(width, height, (char *)bytes, bgr);
		components = 5;
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
#if DIRECTX || SOFTWARE
	if (components == 5)
		tex_object = gfx.LoadTexture(width, height, 4, format, pBits, clamp, anisotropic);
	else
		tex_object = gfx.LoadTexture(width, height, components, format, bytes, clamp, anisotropic);
#endif
#ifdef OPENGL
	tex_object = gfx.LoadTexture(width, height, components, format, bytes, clamp, anisotropic);
#endif
#ifdef VULKAN
	format = -1;
	unsigned char *bytes = stbi_load_from_memory(data, size, &width, &height, &components, 0);
	tex_object = gfx.LoadTexture(width, height, components, format, bytes, clamp, anisotropic);
#endif



#ifndef DEDICATED
#ifndef VULKAN
	stbi_image_free(bytes);
	delete [] data;
#endif
#endif

#ifdef OPENGL
	if (format != GL_RGBA)
	{
		// negative means it has an alpha channel
		return -tex_object;
	}
#endif
	return tex_object;
}

// Need asset manager class so things arent doubly loaded
int load_texture(Graphics &gfx, char *file_name, bool clamp, bool bgr, int anisotropic)
{
	unsigned char *img_data = NULL;
	int width = 0;
	int height = 0;
	int components = 0;
	int format = 0;
	int tex_object = -1;

	unsigned int size = 0;
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
#ifdef OPENGL
	img_data = stbi_load_from_memory(data, size, &width, &height, &components, 0);
	delete[] data;

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
#ifdef _DEBUG
		printf("Unknown component: %s %d\n", file_name, components);
#endif
		stbi_image_free(img_data);
		return 0;
	}
#endif
#ifdef DIRECTX
	components = 4;
	img_data = stbi_load_from_memory(data, size, &width, &height, &components, 0);
	format = 4;
#endif
#ifdef SOFTWARE
	img_data = stbi_load_from_memory(data, size, &width, &height, &components, 4);
#endif
#ifdef VULKAN
	format = -1;
	img_data = stbi_load_from_memory(data, size, &width, &height, &components, 0);
#endif
#ifdef DEDICATED
	if (components == 3)
	{
		char *temp = tga_24to32(width, height, (char *)img_data, bgr);
		delete[] img_data;
		img_data = (unsigned char *)temp;
		components = 5;
	}
#endif
#ifndef DEDICATED
#ifdef DIRECTX
	if (components == 3)
	{
		char *temp = tga_24to32(width, height, (char *)img_data, bgr);
		delete[] img_data;
		img_data = (unsigned char *)temp;
		components = 5;
	}
#endif

	if (components == 3)
	{
		tex_object = gfx.LoadTexture(width, height, 4, format, img_data, clamp, anisotropic);
	}
	else if (components == 5)
	{
		tex_object = gfx.LoadTexture(width, height, 4, format, img_data, clamp, anisotropic);
//		delete [] pBits;
	}
	else
	{
		tex_object = gfx.LoadTexture(width, height, components, format, img_data, clamp, anisotropic);
	}
	stbi_image_free(img_data);
#endif
#ifdef OPENGL
	if (format != GL_RGBA)
	{
		// negative means it has an alpha channel
		return -tex_object;
	}
#endif
	return tex_object;
}

void calc_hash(char *filename, char *hash)
{
	unsigned int size = 0;

	char *data = get_file(filename, &size);
	if (data == NULL)
	{
		sprintf(hash, "missing file");
		return;
	}

	md5sum(data, size, hash);
	delete [] data;
}

char *tga_24to32(int width, int height, char *pBits, bool bgr)
{
	int lImageSize = width * height * 4;
	char *pNewBits = new char[lImageSize * sizeof(char)];

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
		if (strcmp(entity_list[i]->entstring->type, "navpoint") == 0)
		{
			Entity *ent = entity_list[i];
			int k = 0;
			int targetname = atoi(entity_list[i]->entstring->target_name + 3);
			node[j].num_arcs = 0;

			// Manhattan distance table
			ref[k].x = (int)ent->position.x;
			ref[k].y = (int)ent->position.y;
			ref[k].z = (int)ent->position.z;

			strcpy(data, ent->entstring->target);


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


void gen_unit_box(Graphics &gfx, int &unit_index, int &unit_vertex)
{
	vec3 aabb[8];
	vertex_t vert[8];

	// just make unit box
	float scale = 10.0f;
	aabb[0] = vec3(-scale, -scale, -scale);
	aabb[1] = vec3(-scale, -scale, scale);
	aabb[2] = vec3(-scale, scale, -scale);
	aabb[3] = vec3(-scale, scale, scale);
	aabb[4] = vec3(scale, -scale, -scale);
	aabb[5] = vec3(scale, -scale, scale);
	aabb[6] = vec3(scale, scale, -scale);
	aabb[7] = vec3(scale, scale, scale);

	for (int i = 0; i < 8; i++)
		vert[i].position = aabb[i];

	int	index[24] = { 0, 1,
		0, 2,
		1, 3,
		2, 3,
		4, 5,
		4, 6,
		5, 7,
		6, 7,
		0, 4,
		1, 5,
		2, 6,
		3, 7 };

	unit_vertex = gfx.CreateVertexBuffer(vert, 8);
	unit_index = gfx.CreateIndexBuffer(index, 24);
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
	char r = (char)(255.0f * v.x);
	char g = (char)(255.0f * v.y);
	char b = (char)(255.0f * v.z);


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

void read_bitmap(char *filename, int &width, int &height, int **data)
{
	FILE *file;
	bitmap_t	bitmap;

	memset(&bitmap, 0, sizeof(bitmap_t));

	file = fopen(filename, "rb");
	if (file == NULL)
	{
		perror("Unable to write file");
		*data = NULL;
		return;
	}

	fread(&bitmap, 1, sizeof(bitmap_t), file);
	width = bitmap.dib.width;
	height = bitmap.dib.height;
	*data = new int[width * height];
	fread((void *)*data, 1, width * height * 4, file);
	fclose(file);

	if (bitmap.dib.bpp == 24)
	{
		int *old = *data;
		*data = (int *)tga_24to32(width, height, (char *)*data, false);
		delete[] old;
	}
}




void write_bitmap(char *filename, int width, int height, int *data)
{
	FILE *file;
	bitmap_t	bitmap;

	memset(&bitmap, 0, sizeof(bitmap_t));
	memcpy(bitmap.header.type, "BM", 2);
	bitmap.header.offset = sizeof(bmpheader_t);
	bitmap.dib.size = sizeof(dib_t);
	bitmap.dib.width = width;
	bitmap.dib.height = height;
	bitmap.dib.planes = 1;
	bitmap.dib.bpp = 32;
	bitmap.dib.compression = 0;
	bitmap.dib.image_size = width * height * sizeof(int);
	bitmap.header.file_size = sizeof(bmpheader_t) + sizeof(dib_t) + bitmap.dib.image_size;

	file = fopen(filename, "wb");
	if (file == NULL)
	{
		perror("Unable to write file");
		return;
	}

	fwrite(&bitmap, 1, sizeof(bitmap_t), file);
	fwrite((void *)data, 1, width * height * 4, file);
	fclose(file);
}

ping_t hist[64];
int hist_index = 0;

void ping_time_start(int sequence)
{
	ping_t start;

	double time = 0.0;

#ifdef WIN32
	time = GetCounter(freq);
#endif

	start.sequence = sequence;
	start.time = time;
	hist[hist_index++] = start;
	if (hist_index >= 64)
		hist_index = 0;
}

double ping_time_end(int sequence)
{
	double time = 0.0;

	for (int i = 0; i < 64; i++)
	{
		if (hist[i].sequence == sequence)
		{
#ifdef WIN32
			time = GetCounter(freq);
#endif
			return time - hist[i].time;
		}
	}


	return -1.0;
}

float clamp(float value, float min, float max)
{
	return MIN(max, MAX(min, value));
}

int clamp(int value, int min, int max)
{
	return MIN(max, MAX(min, value));
}



#ifndef _WIN64 
void get_cpu_info(struct cpuinfo *info)
{
#ifdef WIN32
	__asm
	{
		mov esi, info
		mov eax, 0
		CPUID
		mov DWORD PTR[esi]info.support, eax
		mov DWORD PTR([esi]info.vendor), ebx
		mov DWORD PTR([esi]info.vendor + 4), edx
		mov DWORD PTR([esi]info.vendor + 8), ecx
		mov DWORD PTR([esi]info.vendor + 12), 0
		cmp eax, 1
		js done
		mov eax, 1
		CPUID
		mov[esi]info.signature, eax
		mov[esi]info.brandid, ebx
		mov[esi]info.msr, ecx
		mov[esi]info.config, edx
		cmp[esi]info.support, 3
		js done
		mov eax, 3
		CPUID
		mov DWORD PTR([esi]info.serial), ecx
		mov DWORD PTR([esi]info.serial + 4), edx
		mov DWORD PTR([esi]info.serial + 8), 0
		done:
		mov eax, 0x80000000
			CPUID
			mov[esi]info.extsupport, eax
			cmp eax, 0x80000001
			js enda
			mov eax, 0x80000001
			CPUID
			mov[esi]info.ext, eax
			enda :
	};
#endif

	info->stepping = (info->signature & 0x0000000F);
	info->model = (info->signature & 0x000000F0) >> 4;
	info->family = (info->signature & 0x00000F00) >> 8;
	info->type = (info->signature & 0x00003000) >> 12;
	info->extmodel = (info->signature & 0x000F0000) >> 16;
	info->extfamily = (info->signature & 0x0FF00000) >> 24;
}
#else
void get_cpu_info(struct cpuinfo *info)
{
}
#endif

void show_hw_info()
{
#ifdef WIN32
	__m128 input = { -997.0f };
	input = _mm_rcp_ps(input);
	int platform = (input.m128_u32[0] >> 8) & 0xf;
	switch (platform)
	{
	case 0x0:
		debugf("CPU:             Intel\n");
		break;
	case 0x7:
		debugf("CPU:             AMD Bulldozer\n");
		break;
	case 0x8:
		debugf("CPU:             AMD K8\n");
		break;
	}

	debugf("Frequency:     %.1f ghz\n", freq / 1000.0f);
#endif

	struct cpuinfo	info = { {0} };
	get_cpu_info(&info);
	printf("Vendor:\t\t%s\nStepping:\t%i\nModel:\t\t%i\nFamily:\t\t%i\nType:\t\t%i\nExtModel:\t%i\nExtFamily:\t%i\n", info.vendor, info.stepping,
		info.model, info.family, info.type, info.extmodel, info.extfamily);

#ifdef WIN32
	ULONGLONG mem_kb = 0;
	GetPhysicallyInstalledSystemMemory(&mem_kb);
	debugf("RAM:             %.2f GB\n", (float)(mem_kb / (1024 * 1024)));
#endif

#ifdef OPENGL
	debugf("GL Version:    %s\n", glGetString(GL_VERSION));
	debugf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	debugf("Vendor:         %s\n", glGetString(GL_VENDOR));
	debugf("GPU:             %s\n", glGetString(GL_RENDERER));
	char *vendor = (char *)glGetString(GL_VENDOR);
	if (vendor == NULL)
	{
		debugf("Vendor is NULL, does you GPU support OpenGL 4.4?\n");
	}


	if (vendor && (strstr(vendor, "nvidia") || strstr(vendor, "NVIDIA")))
	{
		#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
		int total_mem_kb = 0;
		glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &total_mem_kb);
		debugf("GPU RAM:        %.2f GB\n", (float)(total_mem_kb / (1024 * 1024)));
	}
	else if (vendor && strstr(vendor, "Intel"))
	{
		debugf("GPU RAM:      Using system ram\n");
	}
	else if (vendor)
	{
#ifdef WIN32
		unsigned int  n = wglGetGPUIDsAMD(0, 0);
		unsigned int *ids = new UINT[n];
		size_t total_mem_mb = 0;
		wglGetGPUIDsAMD(n, ids);
		wglGetGPUInfoAMD(ids[0], WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(size_t), &total_mem_mb);
		debugf("GPU RAM:      %.2f GB\n", (float)(total_mem_mb / 1024));
#endif
	}
#endif
}



/*
This is based on a pid controller project from a control systems class
The assignment was to have a rocket with X/Y thrusters hit targets of two types
	one type you had to wait 0.2 seconds a small distance away before going to next target
	the other type you had to just get near it before going to next target

This code (translated from matlab) performed phenomenally
My code was more time domain typical approach type stuff,
but the performance of the PID approach my partner Noah Maze
came up with blew me away

I kept a copy of the project from 2011, six years later I finally get around to messing with it :p

// some background
// https://www.youtube.com/watch?v=XfAt6hNV8XM
*/
void pid_controller(const vec3 &target, const float timestep, const vec3 &pos, vec3 &thrust, const float kd)
{
	vec3 delta = target - pos;

//	int kd; // gain

	// Lead Compensator params
	int a = 5;
	int p = 10 * a;

	static vec3 old_delta;
	static vec3 old_thrust;
	static int start = 0;


	if (start == 0)
	{
		//initialize old data
		start = 1;

		old_delta = target - pos;
		old_thrust = vec3(0.0f, 0.0f, 0.0f);
	}

		// If you have to wait
		//kd = 820;
		//If you just have to hit it.
//		kd = 636;

	// Really more of a pd controller as there is no summation of error terms
	// doing difference / derivatives over position and velocity

	// X-axis thrust
	thrust.x = ((kd * (delta.x - old_delta.x) / timestep) +
		kd * a * delta.x - (thrust.x - old_thrust.x) / timestep) / p;

	// Y-axis thrust
	thrust.y = ((kd * (delta.y - old_delta.y) / timestep) +
		kd * a * (delta.y) - (thrust.y - old_thrust.y) / timestep) / p;

	// Z-axis thrust
	thrust.z = ((kd * (delta.z - old_delta.z) / timestep) +
		kd * a * delta.z - (thrust.z - old_thrust.z) / timestep) / p;

	old_delta = delta;
	old_thrust = thrust;

//	printf("target %3.3f %3.3f %3.3f\n", target.x, target.y, target.z);
//	printf("pos    %3.3f %3.3f %3.3f\n", pos.x, pos.y, pos.z);
//	printf("thrust %3.3f %3.3f %3.3f\n", thrust.x, thrust.y, thrust.z);
}

void init_pid(pid_state_t *pid)
{
	pid->dState = vec3(0.0f, 0.0f, 0.0f);
	pid->iState = vec3(0.0f, 0.0f, 0.0f);
	pid->iMin = 0.0f;
	pid->iMax = 800.0f;

	pid->pGain = 100.0f;
	pid->iGain = 0.0f; // really just used to eliminate oscillations
	pid->dGain = 250.0f;
}

// Going to limit to only position, not velocity or acceleration 
// Error is delta position from target
void update_pid(pid_state_t *pid, const vec3 &target, const vec3 &position, vec3 &output)
{
	vec3 pTerm, dTerm, iTerm;
	vec3 error = target - position;
	float max_force = 1000.0f;

	pTerm = error * pid->pGain;
	pid->iState += error;
	if (pid->iState.magnitude() > pid->iMax)
	{
		pid->iState = pid->iState.normalize() * pid->iMax;
	}
	else if (pid->iState.magnitude() < pid->iMin)
	{
		pid->iState = pid->iState.normalize() * pid->iMin;
	}

	iTerm = pid->iState * pid->iGain;

	dTerm = (pid->dState - position) * pid->dGain;
	pid->dState = position;

	if (pTerm.magnitude() > max_force)
		pTerm = pTerm.normalize() * max_force;
	if (iTerm.magnitude() > max_force)
		iTerm = iTerm.normalize() * max_force;
	if (dTerm.magnitude() > max_force)
		dTerm = dTerm.normalize() * max_force;
	
//	printf("pterm %04.2f iterm %04.2f dterm %04.2f\n", pTerm, iTerm, dTerm);

	output += pTerm + iTerm + dTerm;
}

void bezier_curve(float t, vec3 &p, const vec3 &p0, const vec3 &p1, const vec3 &p2, const vec3 &p3)
{
	if (t < 0)
	{
		p =  p0;
		return;
	}
	if (t > 1)
	{
		p = p3;
		return;
	}

	p = p0 * ((1 - t) * (1 - t) * (1 - t)) +
		p2 * (3.0f * ((1 - t) * (1 - t)) * t * t) +
		p3 * (t * t * t);
}


// just a single 0 < t < 1.0 value for multiple bezier curves, need to specify control points p1, p2  and start / end points p0, p3
// can also include a lookat point I suppose, but later
void polyline(vec3 *point, int num_point, float t, vec3 &p)
{
	float time = t * num_point;
	int set = (int)time;

	if (t < 0.0f)
	{
		p = point[0];
		return;
	}

	if (t > 1.0f)
	{
		p = point[num_point - 1];
		return;
	}

	if (set >= num_point - 1)
	{
		set = num_point - 4;
	}

	if (set < 0)
	{
		set = 0;
	}

	bezier_curve(time - set, p, point[set], point[set+1], point[set + 2], point[set + 3]);
}

// Random number generator
static unsigned int seed = 0x13371337;

float random_float()
{
	float res;
	unsigned int tmp;

	seed *= 16807;

	tmp = seed ^ (seed >> 4) ^ (seed << 15);

	*((unsigned int *)&res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}

int auto_complete(const char *a, const char *b)
{
	int min_length = MIN(strlen(a), strlen(b));

	for (int i = 0; i < min_length; i++)
	{
		if (a[i] != b[i])
			return i;
	}

	return strlen(a);
}

int getFarthestInDir(const vec3 *shape, const vec3 &v, const int num_vert)
{
	float max_dot = shape[0] * v;
	int index = 0;

	for(int i = 0; i < num_vert; i++)
	{
		float dotted = shape[i] * v;
		if (dotted > max_dot)
		{
			max_dot = dotted;
			index = i;
		}
	}

	return index;
}

//furthest point in a shape (minkowski difference) along given direction v
void support(const vec3 *shape1, const vec3 *shape2,
	const vec3 &v, vec3 &point, const int num_vert_one, const int num_vert_two)
{
	int index1;
	int index2;
	vec3 nv = -v;

	index1 = getFarthestInDir(shape1, v, num_vert_one);
	index2 = getFarthestInDir(shape2, nv, num_vert_two);
	point = shape1[index1] - shape2[index2];
}


void pick_line(const vec3 &v, const vec3 *shape1, const vec3 *shape2,
	vec3 &a, vec3 &b, const int num_vert_one, const int num_vert_two)
{
	const vec3 nv = -v;
	support(shape2, shape1, v, b, num_vert_two, num_vert_one);
	support(shape2, shape1, nv, a, num_vert_two, num_vert_two);
}

void pick_triangle(vec3 &a, vec3 &b, vec3 &c,
	int &flag, const vec3 *shape1, const vec3 *shape2,
	const int iteration_allowed, const int num_vert_one, const int num_vert_two)
{
	flag  = 0;

	vec3 ab  = b - a;
	vec3 ao = -a;
	vec3 temp = vec3::crossproduct(ab, ao);

	vec3 v = vec3::crossproduct( temp, ab);
	if (v.magnitude() < 0.00001f)
		return;

	c = b;
	b = a;
	support(shape2, shape1, v, a, num_vert_two, num_vert_one);

	for(int i = 0; i < iteration_allowed; i++)
	{
		ab = b - a;
		ao = -a;
		vec3 ac = c - a;

		vec3 abc = vec3::crossproduct(ab, ac);

		vec3 abp = vec3::crossproduct(ab, abc);
		vec3 acp = vec3::crossproduct(abc, ac);

		if (abp * ao > 0)
		{
			c = b;
			b = a;
			v = abp;
		}
		else if (acp * ao > 0)
		{
			b = a;
			v = acp;
		}
		else
		{
			flag = 1;
			break;
		}
		support(shape2, shape1, v, a, num_vert_two, num_vert_one);
	}
}


void pick_tetrahedron(vec3 &a, vec3 &b, vec3 &c,
	int &flag, const vec3 *shape1, const vec3 *shape2,
	const int iteration_allowed, const int num_vert_one, const int num_vert_two)
{
	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 abc = vec3::crossproduct(ab, ac);
	vec3 ao = -a;
	vec3 v;
	vec3 d;

	if (abc * ao > 0)
	{
		d = c;
		c = b;
		b = a;

		v = abc;
		support(shape2, shape1, v, a, num_vert_two, num_vert_one);
	}
	else
	{
		d = b;
		b = a;
		v = -abc;
		support(shape2, shape1, v, a, num_vert_two, num_vert_one);
	}

	vec3 ad = d - a;
	for( int i = 0; i < iteration_allowed; i++)
	{
		ab = b - a;
		ao = -a;
		ac = c - a;

		abc = vec3::crossproduct(ab, ac);
		if ( abc * ao > 0)
		{
		}
		else
		{
			vec3 acd = vec3::crossproduct(ac, ad);

			if (acd * ao > 0)
			{
				b = c;
				c = d;
				ab = ac;
				ac = ad;
				abc = acd;
			}
			else
			{
				vec3 adb = vec3::crossproduct(ad, ab);
				if (adb * ao > 0)
				{
					c = b;
					b = d;
					ac = ab;
					ab = ad;
					abc = adb;
				}
				else
				{
					flag = 1;
					break;
				}
			}
		}

		if (abc * ao > 0)
		{
			d = c;
			c = b;
			b = a;
			v = abc;
			support(shape2, shape1, v, a, num_vert_two, num_vert_one);
		}
		else
		{
			d = b;
			b = a;
			v = -abc;
			support(shape2, shape1, v, a, num_vert_two, num_vert_one);
		}
	}
}


int gjk(const vec3 *shape1, const vec3 *shape2, const int iterations, const int num_vert_one, const int num_vert_two)
{
	vec3 v(0.8f, 0.5f, 1.0f);
	vec3 a, b, c;
	int flag = 0;
	int flag2 = 0;

	pick_line(v, shape2, shape1, a, b, num_vert_two, num_vert_one);
	pick_triangle(a, b, c, flag, shape2, shape1, iterations, num_vert_two, num_vert_one);

	if (flag)
	{
		pick_tetrahedron(a, b, c, flag2, shape2, shape1, iterations, num_vert_two, num_vert_one);
	}
	return flag2;
}

void GetInterval(const vec3 *object, const vec3 &axis, float &minv, float &maxv)
{
	maxv = axis * object[0];
	minv = maxv;

	for (int i = 1; i < 8; i++)
	{
		float value = axis * object[i];

		minv = MIN(minv, value);
		maxv = MAX(maxv, value);
	}
}

bool TestIntersection(const vec3 *object_a, const vec3 *object_b, const vec3 *normal_a, const vec3 *normal_b,
	const vec3 *edge_a, const vec3 *edge_b)
{
	float min1, max1, min2, max2;

	// Only testing cubes, 3 planes for faces, 9 faces for edges
	int num_normal_a = 3;
	int num_normal_b = 3;
	int num_edge_a = 9;
	int num_edge_b = 9;

	for (int i = 0; i < num_normal_a; i++)
	{
		GetInterval(object_a, normal_a[i], min1, max1);
		GetInterval(object_b, normal_a[i], min2, max2);
		if (max1 < min2 || max2 < min1)
			return false;
	}
	for (int i = 0; i < num_normal_b; i++)
	{
		GetInterval(object_a, normal_b[i], min1, max1);
		GetInterval(object_b, normal_b[i], min2, max2);
		if (max1 < min2 || max2 < min1)
			return false;
	}
	for (int i = 0; i < num_edge_a; i++)
	{
		for (int j = 0; j < num_edge_b; j++)
		{
			vec3 axis = vec3::crossproduct(edge_a[i], edge_b[j]);
			GetInterval(object_a, axis, min1, max1);
			GetInterval(object_b, axis, min2, max2);
			if (max1 < min2 || max2 < min1)
				return false;
		}
	}
	return true;
}

// box in binary order
//0: 0 0 0 
//1: 0 0 1 
//2: 0 1 0 
//3: 0 1 1 
//4: 1 0 0 
//5: 1 0 1
//6: 1 1 0
//7: 1 1 1

// Triangles faces 
// zero based (pairs of triangles, skip one each time)
//012 132, 715 731, 567 546, 026 064, 041 145, 236 376
int separating_axis_theorem(const vec3 *box_a, const vec3 *box_b)
{
	vec3 ba;
	vec3 ca;
	vec3 normal_a[3];
	vec3 normal_b[3];
	vec3 edge_a[9];
	vec3 edge_b[9];

	vec3 ra;
	vec3 ua;
	vec3 fa;

	vec3 rb;
	vec3 ub;
	vec3 fb;

	float min = FLT_MAX;
	int ai = -1;
	int bj = -1;

	// Find the closest vertex between the two boxes
	//  Since we can only collide on one side at a time,
	// we can throw away 3 of 6 face planes and
	// 6 of 9 edges per box
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			vec3 diff = box_a[i] - box_b[j];
			float dist = diff.magnitude();
			if (dist < min)
			{
				min = dist;
				ai = i;
				bj = j;
			}
		}
	}

	float diag_a = (box_a[0] - box_a[7]).magnitude();
	float diag_b = (box_b[0] - box_b[7]).magnitude();

	float diag = MAX(diag_a, diag_b);
	if (min > diag)
	{
		// too far apart to be colliding, early exit
		return 0;
	}

	// Only keep normal planes that involve vertex 0
	if (ai == 0)
	{
		// Find normals for faces on cube A
		//012
		ba = box_a[1] - box_a[0];
		ca = box_a[2] - box_a[0];
		normal_a[0] = vec3::crossproduct(ba, ca).normalize();

		//026
		ba = box_a[2] - box_a[0];
		ca = box_a[6] - box_a[0];
		normal_a[1] = vec3::crossproduct(ba, ca).normalize();

		//041
		ba = box_a[4] - box_a[0];
		ca = box_a[1] - box_a[0];
		normal_a[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 0 (right up forward)
		ra = box_a[4] - box_a[0];
		ua = box_a[2] - box_a[0];
		fa = box_a[1] - box_a[0];
	}
	else if (ai == 1)
	{
		// Find normals for faces on cube A
		//012
		ba = box_a[1] - box_a[0];
		ca = box_a[2] - box_a[0];
		normal_a[0] = vec3::crossproduct(ba, ca).normalize();

		//041
		ba = box_a[4] - box_a[0];
		ca = box_a[1] - box_a[0];
		normal_a[1] = vec3::crossproduct(ba, ca).normalize();

		//715
		ba = box_a[1] - box_a[7];
		ca = box_a[5] - box_a[7];
		normal_a[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 1 (right up forward)
		ra = box_a[5] - box_a[1];
		ua = box_a[3] - box_a[1];
		fa = box_a[0] - box_a[1];
	}
	else if (ai == 2)
	{
		//012
		ba = box_a[1] - box_a[0];
		ca = box_a[2] - box_a[0];
		normal_a[0] = vec3::crossproduct(ba, ca).normalize();

		//026
		ba = box_a[2] - box_a[0];
		ca = box_a[6] - box_a[0];
		normal_a[1] = vec3::crossproduct(ba, ca).normalize();

		//236
		ba = box_a[3] - box_a[2];
		ca = box_a[6] - box_a[2];
		normal_a[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 2 (right up forward)
		ra = box_a[6] - box_a[2];
		ua = box_a[3] - box_a[2];
		fa = box_a[0] - box_a[2];
	}
	else if (ai == 3)
	{
		//132
		ba = box_a[3] - box_a[1];
		ca = box_a[2] - box_a[1];
		normal_a[0] = vec3::crossproduct(ba, ca).normalize();

		//731
		ba = box_a[7] - box_a[1];
		ca = box_a[3] - box_a[1];
		normal_a[1] = vec3::crossproduct(ba, ca).normalize();

		//376
		ba = box_a[7] - box_a[3];
		ca = box_a[6] - box_a[3];
		normal_a[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 3 (right up forward)
		ra = box_a[7] - box_a[3];
		ua = box_a[1] - box_a[3];
		fa = box_a[2] - box_a[3];
	}
	else if (ai == 4)
	{
		//546
		ba = box_a[4] - box_a[5];
		ca = box_a[6] - box_a[5];
		normal_a[0] = vec3::crossproduct(ba, ca).normalize();

		//064
		ba = box_a[6] - box_a[0];
		ca = box_a[4] - box_a[0];
		normal_a[1] = vec3::crossproduct(ba, ca).normalize();


		//145
		ba = box_a[4] - box_a[1];
		ca = box_a[5] - box_a[1];
		normal_a[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 4 (right up forward)
		ra = box_a[0] - box_a[4];
		ua = box_a[6] - box_a[4];
		fa = box_a[5] - box_a[4];
	}
	else if (ai == 5)
	{

		//715
		ba = box_a[1] - box_a[7];
		ca = box_a[5] - box_a[7];
		normal_a[0] = vec3::crossproduct(ba, ca).normalize();

		//567
		ba = box_a[6] - box_a[5];
		ca = box_a[7] - box_a[5];
		normal_a[1] = vec3::crossproduct(ba, ca).normalize();

		//376
		ba = box_a[7] - box_a[3];
		ca = box_a[6] - box_a[3];
		normal_a[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 5 (right up forward)
		ra = box_a[1] - box_a[5];
		ua = box_a[7] - box_a[5];
		fa = box_a[4] - box_a[5];
	}


	// Only keep normal planes that involve vertex 0
	if (bj == 0)
	{
		// Find normals for faces on cube A
		//012
		ba = box_b[1] - box_b[0];
		ca = box_b[2] - box_b[0];
		normal_b[0] = vec3::crossproduct(ba, ca).normalize();

		//026
		ba = box_b[2] - box_b[0];
		ca = box_b[6] - box_b[0];
		normal_b[1] = vec3::crossproduct(ba, ca).normalize();

		//041
		ba = box_b[4] - box_b[0];
		ca = box_b[1] - box_b[0];
		normal_b[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 0 (right up forward)
		rb = box_a[4] - box_a[0];
		ub = box_a[2] - box_a[0];
		fb = box_a[1] - box_a[0];
	}
	else if (bj == 1)
	{
		// Find normals for faces on cube A
		//012
		ba = box_b[1] - box_b[0];
		ca = box_b[2] - box_b[0];
		normal_b[0] = vec3::crossproduct(ba, ca).normalize();

		//041
		ba = box_b[4] - box_b[0];
		ca = box_b[1] - box_b[0];
		normal_b[1] = vec3::crossproduct(ba, ca).normalize();

		//715
		ba = box_b[1] - box_b[7];
		ca = box_b[5] - box_b[7];
		normal_b[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 1 (right up forward)
		rb = box_a[5] - box_a[1];
		ub = box_a[3] - box_a[1];
		fb = box_a[0] - box_a[1];
	}
	else if (bj == 2)
	{
		//012
		ba = box_b[1] - box_b[0];
		ca = box_b[2] - box_b[0];
		normal_b[0] = vec3::crossproduct(ba, ca).normalize();

		//026
		ba = box_b[2] - box_b[0];
		ca = box_b[6] - box_b[0];
		normal_b[1] = vec3::crossproduct(ba, ca).normalize();

		//236
		ba = box_b[3] - box_b[2];
		ca = box_b[6] - box_b[2];
		normal_b[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 2 (right up forward)
		rb = box_a[6] - box_a[2];
		ub = box_a[3] - box_a[2];
		fb = box_a[0] - box_a[2];
	}
	else if (bj == 3)
	{
		//132
		ba = box_b[3] - box_b[1];
		ca = box_b[2] - box_b[1];
		normal_b[0] = vec3::crossproduct(ba, ca).normalize();

		//731
		ba = box_b[7] - box_b[1];
		ca = box_b[3] - box_b[1];
		normal_b[1] = vec3::crossproduct(ba, ca).normalize();

		//376
		ba = box_a[7] - box_a[3];
		ca = box_a[6] - box_a[3];
		normal_b[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 3 (right up forward)
		rb = box_a[7] - box_a[3];
		ub = box_a[1] - box_a[3];
		fb = box_a[2] - box_a[3];
	}
	else if (bj == 4)
	{
		//546
		ba = box_b[4] - box_b[5];
		ca = box_b[6] - box_b[5];
		normal_b[0] = vec3::crossproduct(ba, ca).normalize();

		//064
		ba = box_b[6] - box_b[0];
		ca = box_b[4] - box_b[0];
		normal_b[1] = vec3::crossproduct(ba, ca).normalize();


		//145
		ba = box_b[4] - box_b[1];
		ca = box_b[5] - box_b[1];
		normal_b[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 4 (right up forward)
		rb = box_a[0] - box_a[4];
		ub = box_a[6] - box_a[4];
		fb = box_a[5] - box_a[4];
	}
	else if (bj == 5)
	{

		//715
		ba = box_b[1] - box_b[7];
		ca = box_b[5] - box_b[7];
		normal_b[0] = vec3::crossproduct(ba, ca).normalize();

		//567
		ba = box_b[6] - box_b[5];
		ca = box_b[7] - box_b[5];
		normal_b[1] = vec3::crossproduct(ba, ca).normalize();

		//376
		ba = box_b[7] - box_b[3];
		ca = box_b[6] - box_b[3];
		normal_b[2] = vec3::crossproduct(ba, ca).normalize();

		// Find Edges involving vertex 5 (right up forward)
		rb = box_a[1] - box_a[5];
		ub = box_a[7] - box_a[5];
		fb = box_a[4] - box_a[5];
	}


	// Test 3 planes of A, 3 planes of B (face planes of nearest vertices)
	// Test 9 planes generated by performing edges of A cross edges of B
	// Test face planes and "edge planes" which are edgeA cross EdgeB

	ra = ra.normalize();
	ua = ua.normalize();
	fa = fa.normalize();

	rb = ra.normalize();
	ub = ua.normalize();
	fb = fa.normalize();

	edge_a[0] = vec3::crossproduct(ra, rb);
	edge_a[1] = vec3::crossproduct(ra, ub);
	edge_a[2] = vec3::crossproduct(ra, fb);

	edge_b[0] = vec3::crossproduct(rb, ra);
	edge_b[1] = vec3::crossproduct(ub, ra);
	edge_b[2] = vec3::crossproduct(fb, ra);

	edge_a[3] = vec3::crossproduct(ua, rb);
	edge_a[4] = vec3::crossproduct(ua, ub);
	edge_a[5] = vec3::crossproduct(ua, fb);

	edge_b[3] = vec3::crossproduct(rb, ua);
	edge_b[4] = vec3::crossproduct(ub, ua);
	edge_b[5] = vec3::crossproduct(fb, ua);

	edge_a[6] = vec3::crossproduct(fa, rb);
	edge_a[7] = vec3::crossproduct(fa, ub);
	edge_a[8] = vec3::crossproduct(fa, fb);

	edge_b[6] = vec3::crossproduct(rb, fa);
	edge_b[7] = vec3::crossproduct(ub, fa);
	edge_b[8] = vec3::crossproduct(fb, fa);


	return TestIntersection(box_a, box_b, normal_a, normal_b, edge_a, edge_b);
}


void CreateSphere(int sides, float radius, vertex_t *&vertex, unsigned int *&index, unsigned int &num_vertex, unsigned int &num_index, bool invert, vec3 &offset)
{
	float theta1 = 0, theta2 = 0, theta3 = 0;
	float xcoord = 0;
	float ycoord = 0;
	float ex = 0, px = 0, cx = xcoord;
	float ey = 0, py = 0, cy = ycoord;
	float ez = 0, pz = 0, cz = 0, r = radius;
	int k = 0;


	vertex = new vertex_t[sides * (sides + 1)];
	num_vertex = sides * (sides + 1);

	for (int j = 0; j < sides / 2; j++)
	{
		theta1 = j * (2 * MY_PI) / sides - MY_PI / 2;
		theta2 = (j + 1) * (2 * MY_PI) / sides - MY_PI / 2;

		for (int i = 0; i <= sides; i++)
		{
			theta3 = i * (2 * MY_PI) / sides;

			ey = (float)(fcos(theta1) * fcos(theta3));
			ex = (float)(fsin(theta1));
			ez = (float)(fcos(theta1) * fsin(theta3));
			px = cx + r * ex;
			py = cy + r * ey;
			pz = cz + r * ez;

			vertex[k].normal = vec3(ex, ey, ez);
			vertex[k].texCoord0.x = i / (float)sides;
			vertex[k].texCoord0.y = 2 * j / (float)sides;
			vertex[k].position = vec3(px, py, pz) + offset;
			k++;

			ey = (float)(fcos(theta2) * fcos(theta3));
			ex = (float)(fsin(theta2));
			ez = (float)(fcos(theta2) * fsin(theta3));
			px = cx + r * ex;
			py = cy + r * ey;
			pz = cz + r * ez;

			vertex[k].normal = vec3(ex, ey, ez);
			vertex[k].texCoord0.x = i / (float)sides;
			vertex[k].texCoord0.y = 2 * (j + 1) / (float)sides;
			vertex[k].position = vec3(px, py, pz) + offset;
			k++;
		}
	}
	index = new unsigned int[k * 3];
	num_vertex = k;

	unsigned int j = 0;
	for (unsigned int i = 0; i < num_vertex; i += 2)
	{
		// read quad strip, generate two triangles
		if (i == 0)
		{
			if (invert == false)
			{
				index[j + 0] = i + 2;
				index[j + 1] = i + 1;
				index[j + 2] = i + 0;

				index[j + 3] = i + 1;
				index[j + 4] = i + 2;
				index[j + 5] = i + 3;
			}
			else
			{
				index[j + 2] = i + 2;
				index[j + 1] = i + 1;
				index[j + 0] = i + 0;

				index[j + 5] = i + 1;
				index[j + 4] = i + 2;
				index[j + 3] = i + 3;
			}
			j += 6;
			i += 2;
		}
		else
		{
			if (invert == false)
			{
				index[j + 0] = i + 0;
				index[j + 1] = i - 1;
				index[j + 2] = i - 2;

				index[j + 3] = i - 1;
				index[j + 4] = i + 0;
				index[j + 5] = i + 1;
			}
			else
			{
				index[j + 2] = i + 0;
				index[j + 1] = i - 1;
				index[j + 0] = i - 2;

				index[j + 5] = i - 1;
				index[j + 4] = i + 0;
				index[j + 3] = i + 1;
			}
			j += 6;
		}
	}
	num_index = j;
}

void ClipVelocity(vec3 &in, vec3 &normal)
{
	float	backoff;
	vec3	change;
	float	overbounce = BOUNCE;

	backoff = in * normal;
	change = (normal * backoff) * overbounce;
	in -= change;
}

void make_skybox(Graphics &gfx, unsigned int num_vertex, unsigned int &num_index, int &skybox_vertex, int &skybox_index, bool sphere)
{
	if (sphere == false)
	{
		vec3 verts[36] =
		{
		// Front face
		vec3(-200.0f, 200.0f, 200.0f), //3
		vec3(200.0f, -200.0f, 200.0f), //2
		vec3(200.0f, 200.0f, 200.0f),  //1

		vec3(-200.0f, -200.0f, 200.0f), //4
		vec3(200.0f, -200.0f, 200.0f),  //2
		vec3(-200.0f, 200.0f, 200.0f), //3

		// Back face
		vec3(200.0f, 200.0f, -200.0f),		//3
		vec3(-200.0f, -200.0f, -200.0f),	//2
		vec3(-200.0f, 200.0f, -200.0f),		//1

		vec3(200.0f, -200.0f, -200.0f),		//4
		vec3(-200.0f, -200.0f, -200.0f),	//2
		vec3(200.0f, 200.0f, -200.0f),		//3

		// Left face
		vec3(-200.0f, 200.0f, -200.0f),		//3
		vec3(-200.0f, -200.0f, 200.0f),		//2
		vec3(-200.0f, 200.0f, 200.0f),		//1

		vec3(-200.0f, -200.0f, -200.0f),	//4
		vec3(-200.0f, -200.0f, 200.0f),		//2
		vec3(-200.0f, 200.0f, -200.0f),		//3

		// Right face
		vec3(200.0f, 200.0f, 200.0f),		//3
		vec3(200.0f, -200.0f, -200.0f),		//2
		vec3(200.0f, 200.0f, -200.0f),		//1

		vec3(200.0f, -200.0f, 200.0f),		//4
		vec3(200.0f, -200.0f, -200.0f),		//2
		vec3(200.0f, 200.0f, 200.0f),		//3

		// Top face
		vec3(-200.0f, 200.0f, 200.0f),		//3
		vec3(200.0f, 200.0f, -200.0f),		//2
		vec3(-200.0f, 200.0f, -200.0f),		//1

		vec3(200.0f, 200.0f, 200.0f),		//4
		vec3(200.0f, 200.0f, -200.0f),		//2
		vec3(-200.0f, 200.0f, 200.0f),		//3

		// Bottom face
		vec3(200.0f, -200.0f, 200.0f),		//3
		vec3(-200.0f, -200.0f, -200.0f),	//2
		vec3(200.0f, -200.0f, -200.0f),		//1

		vec3(-200.0f, -200.0f, 200.0f),		//4
		vec3(-200.0f, -200.0f, -200.0f),	//2
		vec3(200.0f, -200.0f, 200.0f)		//3
		};

		vec2 texcoords[36] =
		{
		// Front face
		vec2(0.0f, 0.0f),			//2
		vec2(0.0f, -1.0f),			//1
		vec2(-1.0f, -1.0f),			//3

		vec2(0.0f, 0.0f),			//2
		vec2(-1.0f, -1.0f),			//3
		vec2(-1.0f, 0.0f),			//4


		// Back face
		vec2(0.0f, 0.0f),			//2
		vec2(0.0f, 1.0f),			//1
		vec2(1.0f, 1.0f),			//3

		vec2(0.0f, 0.0f),			//2
		vec2(1.0f, 1.0f),			//3
		vec2(1.0f, 0.0f),			//4

		// Left face
		vec2(0.0f, -1.0f),			//1
		vec2(0.0f, 0.0f),			//2
		vec2(-1.0f, -1.0f),			//3

		vec2(0.0f, 0.0f),			//2
		vec2(-1.0f, 0.0f),			//4
		vec2(-1.0f, -1.0f),			//3

		// Right face
		vec2(0.0f, 1.0f),			//1
		vec2(0.0f, 0.0f),			//2
		vec2(1.0f, 1.0f),			//3

		vec2(0.0f, 0.0f),			//2
		vec2(1.0f, 0.0f),			//4
		vec2(1.0f, 1.0f),			//3

		// Top face
		vec2(0.0f, 1.0f),			//1
		vec2(0.0f, 0.0f),			//2
		vec2(1.0f, 1.0f),			//3

		vec2(0.0f, 0.0f),			//2
		vec2(1.0f, 0.0f),			//4
		vec2(1.0f, 1.0f),			//3

		// Bottom face
		vec2(0.0f, 1.0f),			//1
		vec2(0.0f, 0.0f),			//2
		vec2(1.0f, 1.0f),			//3

		vec2(0.0f, 0.0f),			//2
		vec2(1.0f, 0.0f),			//4
		vec2(1.0f, 1.0f),			//3
		};

		/*
		vec3 normals[6] =
		{
		vec3(0.0f, 0.0f, -1.0f),
		vec3(0.0f, 0.0f, 1.0f),
		vec3(1.0f, 0.0f, 0.0f),
		vec3(-1.0f, 0.0f, 0.0f),
		vec3(0.0f, -1.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f)
		};*/

		vertex_t vertex_array[36];
		int index_array[36];
		for (int i = 0; i < 36; i++)
		{
			vertex_array[i].position = verts[i] * 1e10;
			vertex_array[i].texCoord0 = texcoords[i];
			vertex_array[i].normal = vec3();
			index_array[i] = i;
			skybox_vertex = gfx.CreateVertexBuffer(vertex_array, num_vertex);
			skybox_index = gfx.CreateIndexBuffer(index_array, num_index);
		}
	}
	else
	{
		vertex_t *vertex_array;
		unsigned int *index_array;
		vec3 offset(0.0f, -500.0f, 0.0f);

		CreateSphere(36, 1000000.0f, vertex_array, index_array, num_vertex, num_index, false, offset);
		skybox_vertex = gfx.CreateVertexBuffer(vertex_array, num_vertex);
		skybox_index = gfx.CreateIndexBuffer(index_array, num_index);
		delete[] vertex_array;
		delete[] index_array;
	}

}

void WriteObj(char *filename, vertex_t *vertex_array, unsigned int num_vertex, unsigned int *index_array, unsigned int num_index)
{
	FILE *fp = fopen(filename, "w");
	if (fp == NULL)
	{
		debugf("Unable to export %s\n", filename);
		return;
	}

	fprintf(fp, "# model exported by altEngine\n\n");
	fprintf(fp, "mtllib export.mtl\n\n");

	for (unsigned int i = 0; i < num_vertex; i++)
	{
		fprintf(fp, "v %f %f %f\n", vertex_array[i].position.x, vertex_array[i].position.y, vertex_array[i].position.z);
	}
	fprintf(fp, "\n");
	for (unsigned int i = 0; i < num_vertex; i++)
	{
		fprintf(fp, "vt %f %f %f\n", vertex_array[i].texCoord0.x, vertex_array[i].texCoord0.y, 0.0f);
	}
	fprintf(fp, "\n");

	for (unsigned int i = 0; i < num_vertex; i++)
	{
		fprintf(fp, "vn %f %f %f\n", vertex_array[i].normal.x, vertex_array[i].normal.y, vertex_array[i].normal.z);
	}
	fprintf(fp, "\n");

	for (unsigned int i = 0; i < num_index;)
	{
		fprintf(fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
			index_array[i + 0] + 1, index_array[i + 0] + 1, index_array[i + 0] + 1,
			index_array[i + 1] + 1, index_array[i + 1] + 1, index_array[i + 1] + 1,
			index_array[i + 2] + 1, index_array[i + 2] + 1, index_array[i + 2] + 1
		);
		i += 3;
	}
	fprintf(fp, "\n");

	fprintf(fp, "# %d triangles in total\n", num_index / 3);
	fprintf(fp, "\n\n");
	fclose(fp);
}



void print_entity_meminfo(vector<Entity *> &entity_list)
{
#if 0
	for (int i = 0; i < entity_list.size(); i++)
	{
		debugf("entity_list[%d] = %X\n", i, entity_list[i]);
		debugf("   rigid\t = %X\n", (void *)entity_list[i]->rigid);
		debugf("   model\t = %X\n", (void *)entity_list[i]->model);
		debugf("   light\t = %X\n", (void *)entity_list[i]->light);
		debugf("   speaker\t = %X\n", (void *)entity_list[i]->speaker);
		debugf("   trigger\t = %X\n", (void *)entity_list[i]->trigger);
		debugf("   projectile\t = %X\n", (void *)entity_list[i]->projectile);
		debugf("   player\t = %X\n", (void *)entity_list[i]->player);
		debugf("   construct\t = %X\n", (void *)entity_list[i]->construct);
		debugf("   portal_camera = %X\n", (void *)entity_list[i]->portal_camera);
		debugf("   brushinfo\t = %X\n", (void *)entity_list[i]->brushinfo);
		debugf("   entstring\t = %X\n\n\n", (void *)entity_list[i]->entstring);
	}
#endif
}

void dns_query(Socket &sock)
{
	unsigned char buffer[LINE_SIZE];

	typedef struct
	{
		unsigned short id;	// arbitrary echoed back
		unsigned char flag;	// bits 0001NNNN
		unsigned short qd_count; //1
		unsigned short an_count; //0
		unsigned short ns_count; //0
		unsigned short ar_count; //0
	} dns_header_t;

	typedef struct
	{
		//	unsigned char qname[128];	//pascal string, variable length
		unsigned short qtype;  // 1 - A Record
		unsigned short qclass; // 1 - IN class
	} question_t;

	typedef struct
	{
		unsigned short offset;	//pascal string, variable length
		unsigned short qtype;  // 1 - A Record
		unsigned short qclass; // 1 - IN class
		unsigned short ttl;
		unsigned short rdlength;
		unsigned char data[128];
	} dns_answer_t;


	unsigned char data[] = {
		0xAA, 0xAA,			// id
		0x01, 0x00,			// flag
		0x00, 0x01,			// qd count
		0x00, 0x00,			// an count
		0x00, 0x00,			// ns count
		0x00, 0x00,			// ar count
		0x07,				// string length
		0x64, 0x65, 0x73, 0x6b, 0x74, 0x6f, 0x70, //desktop
		0x0b,				// string length
		0x61, 0x77, 0x72, 0x69, 0x67, 0x68, 0x74, 0x32, 0x30, 0x30, 0x39, //awright2009
		0x03,				// string length
		0x63, 0x6f, 0x6d,	//com
		0x0,				// null string terminator
		0x00, 0x01,			// qtype
		0x00, 0x01			// qclass
	};

	char ip[] = "8.8.8.8:53";
	int ret = sock.sendto((char *)data, sizeof(data), ip);
	if (ret < 0)
	{
		printf("dns request send failed\n");
		return;
	}

	//Sleep(100);
	memset(buffer, 0, sizeof(buffer));
	sock.recvfrom((char *)buffer, sizeof(buffer), ip, strlen(ip));

	dns_header_t *header = (dns_header_t *)&buffer[0];
	dns_answer_t *answer = (dns_answer_t *)&buffer[sizeof(dns_header_t)];


	if (header->id == 0xAAAA)
	{
		printf("%s\n", answer->data + 1);
	}
}

//================================================================================================
// Matlab 'conv'
// given input of length signal_len, and kernel_len, will output (signal_len + kernel_len - 1) values
//================================================================================================
void convolve(const float *signal, unsigned int signal_len,
              const float *kernel, unsigned int kernel_len,
              float *result, int *result_len)
{
  unsigned int n;

  *result_len = signal_len + kernel_len - 1;

  for (n = 0; n < signal_len + kernel_len - 1; n++)
  {
    unsigned int kmin = 0, kmax = 0, k = 0;

    result[n] = 0;

    kmin = (n >= kernel_len - 1) ? n - (kernel_len - 1) : 0;
    kmax = (n < signal_len - 1) ? n : signal_len - 1;

    for (k = kmin; k <= kmax; k++)
    {
      result[n] += signal[k] * kernel[n - k];
    }
  }
}


//================================================================================================
// Matlab conv with 'same' parameter
// does convolution, but keeps same length as input signal_len (throwing away values at begining and end)
//================================================================================================
void convolve_same(const float *signal, unsigned int signal_len,
              const float *kernel, unsigned int kernel_len,
              float *result, int *result_len)
{
        convolve(signal, signal_len, kernel,kernel_len, result, result_len);

        for(unsigned int i = 0; i < signal_len; i++)
        {
                result[i] = result[kernel_len / 2 + i];
        }
        *result_len = signal_len;
}


unsigned int get_url(char *host, char *path, char *response, unsigned int size)
{
	const char request[1024] = "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n";
	struct sockaddr_in	servaddr;
	SOCKET sock;
	char buffer[1024];
	int ret;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// increase rcv buffer for large downloads
	socklen_t arglen = sizeof(int);
	unsigned int rcvbuf = 100 * 1024 * 1024; //default 8192
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, sizeof(rcvbuf));
	printf("Setting SO_RCVBUF to %d\n", rcvbuf);
	getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char *)&rcvbuf, &arglen);
	printf("SO_RCVBUF = %d\n", rcvbuf);

	
	struct hostent *hp = gethostbyname(host);

	if (hp == NULL)
	{
		printf("gethostbyname() failed\n");
		return -1;
	}

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr( inet_ntoa(*(struct in_addr*)(hp->h_addr_list[0])) );
	servaddr.sin_port = htons(80);

	// 3 way handshake
	printf("Attempting to connect to %s\n", host);
	ret = connect(sock, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
	if (ret == SOCKET_ERROR)
	{
#ifdef _WIN32
		ret = WSAGetLastError();

		switch (ret)
		{
		case WSAETIMEDOUT:
			printf("Fatal Error: Connection timed out.\n");
			break;
		case WSAECONNREFUSED:
			printf("Fatal Error: Connection refused\n");
			break;
		case WSAEHOSTUNREACH:
			printf("Fatal Error: Router sent ICMP packet (destination unreachable)\n");
			break;
		default:
			printf("Fatal Error: %d\n", ret);
			break;
		}
#else
		ret = errno;

                switch(ret)
                {
		case ENETUNREACH:
			printf("Fatal Error: The network is unreachable from this host at this time.\n(Bad IP address)\n");
			break;
                case ETIMEDOUT:
                        printf("Fatal Error: Connecting timed out.\n");
                        break;
                case ECONNREFUSED:
                        printf("Fatal Error: Connection refused\n");
                        break;
                case EHOSTUNREACH:
                        printf("Fatal Error: router sent ICMP packet (destination unreachable)\n");
                        break;
                default:
                        printf("Fatal Error: %d\n", ret);
                        break;
                }
#endif
		return -1;
	}
	printf("TCP handshake completed\n");

	memset(buffer, 0, 1024);
	memset(response, 0, size);

	sprintf(buffer, request, path, host);
	send(sock, buffer, strlen(buffer), 0);


	unsigned int num_read = 0;
#ifdef WIN32
	unsigned int start = GetTickCount();
#else
	unsigned int start = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	start = tv.tv_usec * 1000;
#endif
	printf("\n");
	while (1)
	{
		int ret = recv(sock, &response[num_read], size - num_read, 0);
#ifdef WIN32
		unsigned int end = GetTickCount();
#else
		unsigned int end;
		
		gettimeofday(&tv, NULL);
		end = tv.tv_usec * 1000;
#endif
		float mbs = (num_read / (1024 * 1024)) / ((end - start) / 1000.0f);
		printf("Downloaded %d MB rate %f mb/s %f mbps\r", num_read / (1024 * 1024), mbs, mbs * 8);
		if (ret > 0)
		{
			num_read += ret;
		}
		else if (ret == 0)
		{
#ifdef WIN32
		unsigned int end = GetTickCount();
#else
		unsigned int end;
		gettimeofday(&tv, NULL);
		end = tv.tv_usec * 1000;
#endif
			printf("\nDownload complete %d bytes %f total time %f average mb/s %f mbit/s\n",
				num_read, (end - start) / 1000.0f,
				(num_read / (1024 * 1024)) / ((end - start) / 1000.0f),
				((num_read * 8) / 1000000.0f) / ((end - start) / 1000.0f));
			break;
		}
		else
		{
#ifdef WIN32
				ret = WSAGetLastError();
#else
				perror("recv failed");
#endif

			break;
		}
	}
	closesocket(sock);
	return num_read;
}



int http_upload(char *file)
{
	char buffer[4096];
	int sock;
	struct sockaddr_in servaddr;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
	{
		perror("socket failed");
		return -1;
	}


	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(80);

	bind(sock, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
	listen(sock, 3);

	unsigned int filesize = 0;
	char *data = get_file(file, &filesize);
	unsigned int num_sent = 0;

	char header[] = "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type : text/html\r\nConnection : Closed\r\n\r\n";

	printf("Listening for connections on port 80\r\n");
	while (1)
	{
		int csock = accept(sock, NULL, NULL);
		printf("Accepted connection\r\n");
		memset(buffer, 0, 4096);
		recv(csock, buffer, 4096, 0);
		printf("Request:\r\n%s\r\n", buffer);

		sprintf(buffer, header, strlen(header) + filesize - 2);
		send(csock, buffer, strlen(buffer), 0);
		while (num_sent < filesize)
		{
			num_sent += send(csock, &data[num_sent], filesize - num_sent, 0);
		}
		closesocket(csock);
		num_sent = 0;
	}
}

int check_content_length(unsigned char *data, int size, char **filep)
{
	char *file = strstr((char *)data, "\r\n\r\n");
	char *content_length = strstr((char *)data, "Content-Length:");
	int data_size = 0;
	if (file)
	{
		file += 4;
		*filep = file;

		if (content_length)
		{
			int expected_size = atoi(content_length + strlen("Content-Length: "));
			data_size = size - (file - (char *)data);
			if (data_size != expected_size)
			{
				printf("content length mismatch got %d bytes, expected %d bytes\r\n", data_size, expected_size);
				return data_size;
			}
		}
	}

	return data_size;
}

int file_download(char *ip_str, unsigned short int port, char *response, int size, int *download_size, char *file_name)
{
	struct sockaddr_in	servaddr;
	SOCKET sock;
	int ret;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip_str);
	servaddr.sin_port = htons(port);

	// 3 way handshake
	printf("Attempting to connect to %s\n", ip_str);
	ret = connect(sock, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
	if (ret == SOCKET_ERROR)
	{
#ifdef _WIN32
		ret = WSAGetLastError();

		switch (ret)
		{
		case WSAETIMEDOUT:
			printf("Fatal Error: Connection timed out.\n");
			break;
		case WSAECONNREFUSED:
			printf("Fatal Error: Connection refused\n");
			break;
		case WSAEHOSTUNREACH:
			printf("Fatal Error: Router sent ICMP packet (destination unreachable)\n");
			break;
		default:
			printf("Fatal Error: %d\n", ret);
			break;
		}
#else
		ret = errno;

        switch(ret)
        {
		case ENETUNREACH:
			printf("Fatal Error: The network is unreachable from this host at this time.\n(Bad IP address)\n");
			break;
        case ETIMEDOUT:
                printf("Fatal Error: Connecting timed out.\n");
                break;
        case ECONNREFUSED:
                printf("Fatal Error: Connection refused\n");
                break;
        case EHOSTUNREACH:
                printf("Fatal Error: router sent ICMP packet (destination unreachable)\n");
                break;
        default:
                printf("Fatal Error: %d\n", ret);
                break;
        }
#endif
		return -1;
	}
	printf("TCP handshake completed\n");

	memset(response, 0, size);

	int expected_size = 0;
	recv(sock, (char *)&expected_size, 4, 0);
	recv(sock, (char *)file_name, 128, 0);

	while (*download_size < expected_size)
	{
		*download_size += recv(sock, &response[*download_size], expected_size - *download_size, 0);
	}
	closesocket(sock);
	return 0;
}

int file_upload(char *file, unsigned short port)
{
	int			connfd;
	unsigned int		size = sizeof(struct sockaddr_in);
	struct sockaddr_in	servaddr, client;
	time_t			ticks;
	int listenfd;

#ifdef _WIN32
	WSADATA		WSAData;

	WSAStartup(MAKEWORD(2, 0), &WSAData);
#endif

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1)
	{
		perror("socket error");
		return 0;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	if ((::bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) == -1)
	{
		perror("bind error");
		return 0;
	}

	printf("Server listening on: %s:%d\n", inet_ntoa(servaddr.sin_addr), htons(servaddr.sin_port));

	if (listen(listenfd, 3) == -1)
	{
		perror("listen error");
		return 0;
	}

	for (;;)
	{
		char response[1024] = { 0 };

		printf("listening for connections...\n");
		connfd = accept(listenfd, (struct sockaddr *)&client, (socklen_t *)&size);
		if (connfd == INVALID_SOCKET)
			continue;

		ticks = time(NULL);
		snprintf(response, sizeof(response), "%.24s\r\n", ctime(&ticks));
		printf("Client: %s - %s", inet_ntoa(client.sin_addr), response);

		unsigned int file_size = 0;
		char *data = get_file(file, &file_size);
		char file_name[128] = { 0 };

		memcpy(file_name, file, MIN(127, strlen(file)));
		send(connfd, (char *)&file_size, sizeof(int), 0);
		send(connfd, (char *)&file_name, 128, 0);
		send(connfd, data, file_size, 0);
		closesocket(connfd);
	}
	return 0;
}

void *open_lib(char *file, char *function, unsigned int **fpointer)
{
#ifdef WIN32
	HMODULE hModule = LoadLibrary(file);
	*fpointer = (unsigned int *)GetProcAddress(hModule, function);

	return hModule;
#else
	void *handle = dlopen(file, RTLD_LAZY);
	if (!handle)
	{
		printf("dlopen failed %s\r\n", dlerror());
		return NULL;
	}

	*fpointer = (unsigned int *)dlsym(handle, function);
	char *error = dlerror();
	if (error != NULL)
	{
		printf("dlsym failed %s", error);
		return NULL;
	}
	return handle;
#endif
}

void close_lib(void *handle)
{
#ifdef WIN32
	FreeLibrary((HMODULE)handle);
#else
	dlclose(handle);
#endif
}


float CosineInterpolate(float y1, float y2, float mu)
{
	float mu2;

	mu2 = (1 - cosf(mu * (float)M_PI)) / 2;
	return(y1 * (1 - mu2) + y2 * mu2);
}

float CubicInterpolate(float y0, float y1, float y2, float y3, float mu)
{
	float a0, a1, a2, a3, mu2;

	mu2 = mu * mu;
	a0 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
	a1 = y0 - 2.5f * y1 + 2 * y2 - 0.5f * y3;
	a2 = -0.5f * y0 + 0.5f * y2;
	a3 = y1;

	return(a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3);
}

float HermiteInterpolate(float y0, float y1, float y2, float y3,
	float mu, float tension, float bias)
{
	float m0, m1, mu2, mu3;
	float a0, a1, a2, a3;

	mu2 = mu * mu;
	mu3 = mu2 * mu;
	m0 = (y1 - y0)*(1 + bias)*(1 - tension) / 2;
	m0 += (y2 - y1)*(1 - bias)*(1 - tension) / 2;
	m1 = (y2 - y1)*(1 + bias)*(1 - tension) / 2;
	m1 += (y3 - y2)*(1 - bias)*(1 - tension) / 2;
	a0 = 2 * mu3 - 3 * mu2 + 1;
	a1 = mu3 - 2 * mu2 + mu;
	a2 = mu3 - mu2;
	a3 = -2 * mu3 + 3 * mu2;

	return(a0*y1 + a1*m0 + a2*m1 + a3*y2);
}


vec3 HermiteInterp(vec3 &a, vec3 &b, vec3 &c, vec3 &d, float t)
{
	vec3 result;

	result.x = HermiteInterpolate(a.x, b.x, c.x, d.x, t, 0.0f, 0.0f);
	result.y = HermiteInterpolate(a.y, b.y, c.y, d.y, t, 0.0f, 0.0f);
	result.z = HermiteInterpolate(a.z, b.z, c.z, d.z, t, 0.0f, 0.0f);
	return result;
}



int generate_spline(vec3 *output, vec3 *control, int num_control, int num_interp)
{
	unsigned int k = 0;

	// plot beginning
	for (int i = 0; i < num_interp; i++)
	{
		float t = (float)i / num_interp;

		output[k++] = HermiteInterp(control[0], control[0], control[1], control[2], t);
	}

	// plot middle
	for (int j = 0; j < num_control - 3; j++)
	{
		for (int i = 0; i < num_interp; i++)
		{
			float t = (float)i / num_interp;

			output[k++] = HermiteInterp(control[j], control[j + 1], control[j + 2], control[j + 3], t);
		}
	}

	//plot end
	for (int i = 0; i < num_interp; i++)
	{
		float t = (float)i / num_interp;

		output[k++] = HermiteInterp(control[num_control - 3], control[num_control - 2], control[num_control - 1], control[num_control - 1], t);
	}

	return k;
}

vec3 para_spline(vec3 *control, int num_control, float t)
{
	int seg = (int)(t * (num_control - 1));

	t = clamp(t, 0.0f, 1.0f);
	float nt = (t * (num_control - 1)) - ((int)(t * (num_control - 1)) % (num_control - 1));


	if (seg == 0)
	{
		return HermiteInterp(control[0], control[0], control[1], control[2], nt);
	}
	else if (seg < num_control - 2)
	{
		return HermiteInterp(control[seg - 1], control[seg], control[seg + 1], control[seg + 2], nt);
	}
	else
	{
		return HermiteInterp(control[num_control - 3], control[num_control - 2], control[num_control - 1], control[num_control - 1], nt);
	}
}

void make_torus(Graphics &gfx, int numc, int numt, float r1, float r2, float scale, vertex_t *vertex, unsigned int &num_vertex, int *index, unsigned int &num_index, bool invert, int &vbo, int &ibo)
{
	float s, t, x, y, z;
	float twopi = 2 * (float)M_PI;

	for (int i = 0; i < numc; i++)
	{
		for (int j = 0; j <= numt; j++)
		{
			for (int k = 1; k >= 0; k--)
			{
				s = (i + k) % numc + 0.5f;
				t = (float)(j % numt);


				x = (r1 + r2 * cos(s  * twopi / numc)) * cos(t * twopi / numt);
				y = (r1 + r2 * cos(s * twopi / numc)) * sin(t * twopi / numt);
				z = r2 * sin(s * twopi / numc);

				float u = (i + k) / (float)numc;
				float v = t / (float)numt;
				vertex[num_vertex].position = vec3(x * scale, y * scale, z * scale);
				vertex[num_vertex].texCoord0 = vec2(u, v);
				vertex[num_vertex].texCoord1 = vec2(u, v);
				num_vertex++;
			}
		}
	}


	unsigned int j = 0;
	for (unsigned int i = 0; i < num_vertex; i += 2)
	{
		// read quad strip, generate two triangles
		if (i == 0)
		{
			if (invert == false)
			{
				index[j + 0] = i + 2;
				index[j + 1] = i + 1;
				index[j + 2] = i + 0;

				index[j + 3] = i + 1;
				index[j + 4] = i + 2;
				index[j + 5] = i + 3;
			}
			else
			{
				index[j + 2] = i + 2;
				index[j + 1] = i + 1;
				index[j + 0] = i + 0;

				index[j + 5] = i + 1;
				index[j + 4] = i + 2;
				index[j + 3] = i + 3;
			}
			j += 6;
			i += 2;
		}
		else
		{
			if (invert == false)
			{
				index[j + 0] = i + 0;
				index[j + 1] = i - 1;
				index[j + 2] = i - 2;

				index[j + 3] = i - 1;
				index[j + 4] = i + 0;
				index[j + 5] = i + 1;
			}
			else
			{
				index[j + 2] = i + 0;
				index[j + 1] = i - 1;
				index[j + 0] = i - 2;

				index[j + 5] = i - 1;
				index[j + 4] = i + 0;
				index[j + 3] = i + 1;
			}
			j += 6;
		}
	}
	num_index = j;

	// calc normals
	for (unsigned int i = 0; i < num_index; i+=3)
	{
		vec3 a = vertex[index[i+1]].position - vertex[index[i]].position;
		vec3 b = vertex[index[i+2]].position - vertex[index[i]].position;
		vertex[index[i]].normal = vec3::crossproduct(a, b);
		vertex[index[i+1]].normal = vec3::crossproduct(a, b);
		vertex[index[i+2]].normal = vec3::crossproduct(a, b);
	}

	ibo = gfx.CreateIndexBuffer(index, num_index);
	vbo = gfx.CreateVertexBuffer(vertex, num_vertex);
}


/*
F(u,v) = (cos(u)*sin(v)*r, cos(v)*r, sin(u)*sin(v)*r)

r = radius
u = longitude [0 2pi]
v = latitude [0 pi]
*/
vec3 func_sphere(float u, float v, float r)
{
	vec3 result;

	result.x = cos(u) * sin(v) * r;
	result.y = cos(v) * r;
	result.z = sin(u) * sin(v) * r;

	return result;
}



void make_sphere(float u_res, float v_res, float radius, int smooth_normals, vertex_t **vertex_array_in, int &num_vert, int **index_array_in)
{
	float u0 = 0.0;
	float v0 = 0.0;
	float uf = 2.0 * M_PI;
	float vf = M_PI;

	float u_step = (uf - u0) / u_res;
	float v_step = (vf - v0) / v_res;

	num_vert = 0;

	// work through everything once to get a vertex count
	for (int i = 0; i < u_res; i++)
	{
		for (int j = 0; j < v_res; j++)
		{
			// two triangles
			num_vert += 6;
		}
	}

	int num_vert_start = num_vert;
	*vertex_array_in = new vertex_t[num_vert];
	*index_array_in = new int[num_vert];

	vertex_t *vertex_array = *vertex_array_in;
	int *index_array = *index_array_in;

	num_vert = 0;

	for (int i = 0; i < u_res; i++)
	{
		for (int j = 0; j < v_res; j++)
		{
			float u = i * u_step + u0;
			float v = j * v_step + v0;

			float u_next = (i + 1 == u_res) ? uf : (i + 1) * u_step + u0;
			float v_next = (j + 1 == v_res) ? vf : (j + 1) * v_step + v0;

			vec3 p0 = func_sphere(u, v, radius);
			vec3 p1 = func_sphere(u, v_next, radius);
			vec3 p2 = func_sphere(u_next, v, radius);
			vec3 p3 = func_sphere(u_next, v_next, radius);

			vertex_t vert;

			if (smooth_normals == 0)
			{
				vec3 s = p2 - p0;
				vec3 t = p1 - p0;

				vert.normal = vec3::crossproduct(s, t);
			}

			// should probably add texture coordinates


			// triangle 1
			vert.position = p0;
			vert.texCoord0.x = u;
			vert.texCoord0.y = v;
			if (smooth_normals)
				vert.normal = vert.position.normalize();

			index_array[num_vert] = num_vert;
			vertex_array[num_vert++] = vert;

			vert.position = p1;
			vert.texCoord0.x = u;
			vert.texCoord0.y = v_next;
			if (smooth_normals)
				vert.normal = vert.position.normalize();
			index_array[num_vert] = num_vert;
			vertex_array[num_vert++] = vert;

			vert.position = p2;
			vert.texCoord0.x = u_next;
			vert.texCoord0.y = v;
			if (smooth_normals)
				vert.normal = vert.position.normalize();
			index_array[num_vert] = num_vert;
			vertex_array[num_vert++] = vert;


			// triangle 2
			vert.position = p3;
			vert.texCoord0.x = u_next;
			vert.texCoord0.y = v_next;
			if (smooth_normals)
				vert.normal = vert.position.normalize();
			index_array[num_vert] = num_vert;
			vertex_array[num_vert++] = vert;

			vert.position = p2;
			vert.texCoord0.x = u_next;
			vert.texCoord0.y = v;
			if (smooth_normals)
				vert.normal = vert.position.normalize();
			index_array[num_vert] = num_vert;
			vertex_array[num_vert++] = vert;

			vert.position = p1;
			vert.texCoord0.x = u;
			vert.texCoord0.y = v_next;
			if (smooth_normals)
				vert.normal = vert.position.normalize();
			index_array[num_vert] = num_vert;
			vertex_array[num_vert++] = vert;

		}
	}
}


vec3 func_ellipsoid(float u, float v, float r, float a, float b, float c)
{
	vec3 result;

	result.x = a * sin(u) * sin(v);
	result.y = b * cos(u);
	result.z = c * sin(u) * cos(v);

	return result;
}

void make_ellipsoid(float u_res, float v_res, float radius, int smooth_normals, vertex_t **vertex_array_in, int &num_vert, int **index_array_in)
{
	float u0 = 0.0;
	float v0 = 0.0;
	float uf = 2.0 * M_PI;
	float vf = M_PI;

	float u_step = (uf - u0) / u_res;
	float v_step = (vf - v0) / v_res;

	num_vert = 0;

	// work through everything once to get a vertex count
	for (int i = 0; i < u_res; i++)
	{
		for (int j = 0; j < v_res; j++)
		{
			// two triangles
			num_vert += 6;
		}
	}

	int num_vert_start = num_vert;
	*vertex_array_in = new vertex_t[num_vert];
	*index_array_in = new int[num_vert];

	vertex_t *vertex_array = *vertex_array_in;
	int *index_array = *index_array_in;

	num_vert = 0;

	float tex_scale_x = 1.0;
	float tex_scale_y = 0.5;
	float tex_pos_x = 0;
	float tex_pos_y = 0;


	float a = 5.0;
	float b = 3.0;
	float c = 5.0;

	for (int i = 0; i < u_res; i++)
	{
		for (int j = 0; j < v_res; j++)
		{
			int flip = 0;


			float u = i * u_step + u0;
			float v = j * v_step + v0;

			float u_next = (i + 1 == u_res) ? uf : (i + 1) * u_step + u0;
			float v_next = (j + 1 == v_res) ? vf : (j + 1) * v_step + v0;

			vec3 p0 = func_ellipsoid(u, v, radius, a, b, c);
			vec3 p1 = func_ellipsoid(u, v_next, radius, a, b, c);
			vec3 p2 = func_ellipsoid(u_next, v, radius, a, b, c);
			vec3 p3 = func_ellipsoid(u_next, v_next, radius, a, b, c);

			vertex_t vert;

			if (smooth_normals == 0)
			{
				vec3 s = p2 - p0;
				vec3 t = p1 - p0;

				vert.normal = vec3::crossproduct(s, t);
			}

			if (vert.normal * p0 < 0)
				flip = 1;



			if (flip == 0)
			{
				// triangle 1
				vert.position = p0;
				vert.texCoord0.x = u * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v * tex_scale_y + tex_pos_y;

				if (smooth_normals)
					vert.normal = vert.position.normalize();

				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;

				vert.position = p1;
				vert.texCoord0.x = u      * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v_next * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();
				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;

				vert.position = p2;
				vert.texCoord0.x = u_next * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v      * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();
				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;


				// triangle 2
				vert.position = p3;
				vert.texCoord0.x = u_next * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v_next * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();
				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;

				vert.position = p2;
				vert.texCoord0.x = u_next * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v	  * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();
				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;

				vert.position = p1;
				vert.texCoord0.x = u      * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v_next * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();
				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;
			}
			else
			{
				// triangle 1
				vert.position = p0;
				vert.texCoord0.x = u * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();

				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;

				vert.position = p2;
				vert.texCoord0.x = u_next * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v	  * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();
				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;

				vert.position = p1;
				vert.texCoord0.x = u      * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v_next * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();
				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;


				// triangle 2
				vert.position = p3;
				vert.texCoord0.x = u_next * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v_next * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();
				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;

				vert.position = p1;
				vert.texCoord0.x = u      * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v_next * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();
				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;

				vert.position = p2;
				vert.texCoord0.x = u_next * tex_scale_x + tex_pos_x;
				vert.texCoord0.y = v	  * tex_scale_y + tex_pos_y;
				if (smooth_normals)
					vert.normal = vert.position.normalize();
				index_array[num_vert] = num_vert;
				vertex_array[num_vert++] = vert;


			}

		}
	}

}



vec3 get_point(float lam1, float lam2, float lam3, vec3 &a,  vec3 &b, vec3 &c)
{
	vec3 v;

	v.x = lam1 * a.x + lam2 * b.x + lam3 * c.x;
	v.y = lam1 * a.y + lam2 * b.y + lam3 * c.y;
	v.z = lam1 * a.z + lam2 * b.z + lam3 * c.z;

	return v;
}

bool get_barycentric(float x, float y,
    const vec3 &a, const vec3 &b, const vec3 &c,
    float &lam1, float &lam2, float &lam3)
{
	float den = (b.y - c.y)*(a.x - c.x) + (c.x - b.x)*(a.y - c.y);

	if (-den <= 0)
		return false;

	lam1 = (b.y - c.y)*(x - c.x) + (c.x - b.x)*(y - c.y) / den;
	lam2 = (c.y - a.y)*(x - c.x) + (a.x - c.x)*(y - c.y) / den;
	lam3 = 1.0f - lam1 - lam2;

	return true;
}

typedef enum
{
	HC_OUT,
	HC_IN,
	HC_SPANNING,
	HC_ON
} hclass;

int split(const vertex_t *poly, int num_vert, const unsigned int *index_array, int num_index,
	const plane_t &plane, vertex_t *in, vertex_t *out, int &in_c, int &out_c)
{
	static vertex_t outpts[8192];
	static vertex_t inpts[8192];
	vertex_t		ptA, ptB;
	float			sideA, sideB;
	hclass			poly_class = HC_ON; //	assume plane and polygon coincident to start


	out_c = 0;
	in_c = 0;
	//	start with the last point
	ptA = poly[index_array[num_index - 1]];

	// determine which side we are on
	sideA = ptA.position * plane.normal + plane.d;

	for (int i = 0; i < num_index; i++)
	{
		ptB = poly[index_array[i]];

		sideB = ptB.position * plane.normal + plane.d;	// determine which side we are on

		//	if the current point is on the positive side
		if (sideB > 0.000001f)	
		{
			if (poly_class == HC_ON)
			{
				poly_class = HC_OUT;
			}
			else if (poly_class != HC_OUT)
			{
				poly_class = HC_SPANNING;
			}

			//	if the previous point was on the opposite side of the plane
			if (sideA < -0.000001f)
			{
				vec3	v = ptB.position - ptA.position;

				//	add the new point to the partitions
				vec3 p = ptA.position + (v * (-(ptA.position  * plane.normal + plane.d) / (v  * plane.normal + plane.d)));
				outpts[out_c] = ptA;
				outpts[out_c++].position = p;
				inpts[in_c] = ptA;
				inpts[in_c++].position = p;
				poly_class = HC_SPANNING;
			}

			//	add the current point to the positive partition
			outpts[out_c++] = ptB;
		}
		else if (sideB < -0.000001f)
		{
			if (poly_class == HC_ON)
				poly_class = HC_IN;
			else if (poly_class != HC_IN)
				poly_class = HC_SPANNING;


			//	if the previous point was on the opposite side of the plane
			if (sideA > 0.000001f)
			{
				vec3	v = ptB.position - ptA.position;


				//	add the newly computed point_3d to the partitions
				vec3 p = ptA.position + (v * (-(ptA.position  * plane.normal + plane.d) / (v  * plane.normal + plane.d)));
				outpts[out_c] = ptA;
				outpts[out_c++].position = p;
				inpts[in_c] = ptA;
				inpts[in_c++].position = p;
				poly_class = HC_SPANNING;
			}
			//	add the current point_3d to the negative partition
			inpts[in_c++] = ptB;
		}
		else
		{
			//	the current point is on the plane
			outpts[out_c++] = ptB;
			inpts[in_c++] = ptB;
		}
		ptA = ptB;
		sideA = sideB;
	}

	switch (poly_class)
	{
	case HC_OUT:
		//	if the polygon is entirely positive
		for (int i = 0; i < num_index; i++)
			out[index_array[i]] = poly[index_array[i]];
		break;
	case HC_IN:
		//	if the polygon is entirely negative
		for (int i = 0; i < num_index; i++)
			in[index_array[i]] = poly[index_array[i]];
		break;
	case HC_SPANNING:
		//	split polygon
		for(int i = 0; i < out_c; i++)
			out[i] = outpts[i];
		for (int i = 0; i < in_c; i++)
			in[i] = inpts[i];
		break;
	case HC_ON:
		break;
	}
	return poly_class;
}

// 16 bit one's complement
unsigned short checksum(void *b, int len)
{
	unsigned short *buf = (unsigned short *)b;
	unsigned int sum = 0;
	unsigned short result;

	for (sum = 0; len > 1; len -= 2)
	{
		sum += *buf++;
	}
	if (len == 1)
	{
		sum += *(unsigned char*)buf;
	}
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

int ping(char *ip_str)
{
	echo_t request;
	icmp_response_t *response = NULL;
	char buffer[4096];
	int sock;
	int attempt = 1;
	struct sockaddr_in addr;
	struct hostent *hname;

	// convert host to ip
	hname = gethostbyname(ip_str);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = hname->h_addrtype;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = *(long*)hname->h_addr;

	// get raw socket
	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock < 0)
	{
		perror("socket failed");
		return -1;
	}

	while (1)
	{
		unsigned int size = sizeof(addr);

		if (attempt > 3)
		{
			return -1;
		}

		printf("Pinging ip %s\n", ip_str);

		memset(&request, 0, sizeof(echo_t));
		request.header.type = ICMP_ECHO;
		request.header.un.echo.id = attempt;
		request.header.un.echo.sequence = attempt;

		for (int i = 0; i < 64; i++)
		{
			request.data[i] = attempt + '0';
		}
		attempt++;

		request.header.checksum = checksum(&request, sizeof(echo_t));
		if (sendto(sock, (char *)&request, sizeof(echo_t), 0, (struct sockaddr*)&addr, sizeof(addr)) <= 0)
		{
			perror("sendto failed");
#ifdef WIN32
			int ret = WSAGetLastError();

			switch (ret)
			{
			case WSAETIMEDOUT:
				printf("Fatal Error: Connection timed out.\n");
				break;
			case WSAECONNREFUSED:
				printf("Fatal Error: Connection refused\n");
				break;
			case WSAEHOSTUNREACH:
				printf("Fatal Error: Router sent ICMP packet (destination unreachable)\n");
				break;
			default:
				printf("Fatal Error: %d\n", ret);
				break;
			}
#endif

			return -2;
		}

		struct timeval timeout;
		fd_set read_set;

		FD_ZERO(&read_set);
		FD_SET(sock, &read_set);

		timeout.tv_sec = 0;
		timeout.tv_usec = 200000;

		int ret = select(sock + 1, &read_set, NULL, NULL, &timeout);
		if (ret < 0)
		{
			perror("select() failed");
			attempt--;
			continue;
		}
		else if (ret == 0)
		{
			printf("timed out\r\n");
		}

		if (FD_ISSET(sock, &read_set))
		{
			memset(buffer, 0, 4096);
			response = (icmp_response_t *)&buffer;
			ret = recvfrom(sock, (char *)response, 4096, 0, (struct sockaddr*)&addr, (socklen_t *)&size);
			if (ret > 0)
			{
				printf("Response received from echo: %c\r\n", response->data[0]);
				return 0;
			}
		}
	}
}


// old udp printf function
int uprintf(const char *ip_str, const char *format, ...)
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in sa;
	char buf[512];
	va_list vlist;

	va_start(vlist, format);
	vsprintf(buf, format, vlist);
	va_end(vlist);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(1234);
	sa.sin_addr.s_addr = inet_addr(ip_str);
	sendto(fd, buf, strlen(buf) + 1, 0, (struct sockaddr*)&sa, sizeof(sa));

	return 0;
}

void make_tetrahedron(Graphics &gfx, int &ibo, int &vbo)
{
	vec3 p1(0.0f, -1.0f, 2.0f);
	vec3 p2(1.73205081f, -1.0f, -1.0f);
	vec3 p3(-1.73205081f, -1.0f, -1.0f);
	vec3 p4(0.0f, 2.0f, 0.0f);

	vertex_t vertex_array[12];


	memset(vertex_array, 0, sizeof(vec3) * 12);
	vertex_array[0].position = p1;
	vertex_array[1].position = p2;
	vertex_array[2].position = p3;
	vec3 normal = vec3::crossproduct(p2 - p1, p3 - p1);
	vertex_array[0].normal = normal;
	vertex_array[2].normal = normal;
	vertex_array[3].normal = normal;

	// need to verify texcoords
	vertex_array[0].texCoord0 = vec2(0.0f, 0.0f);
	vertex_array[1].texCoord0 = vec2(1.0f, 1.0f);
	vertex_array[2].texCoord0 = vec2(1.0f, 0.0f);



	vertex_array[3].position = p1;
	vertex_array[4].position = p3;
	vertex_array[5].position = p4;
	normal = vec3::crossproduct(p3 - p1, p4 - p1);
	vertex_array[3].normal = normal;
	vertex_array[4].normal = normal;
	vertex_array[5].normal = normal;

	// need to verify texcoords
	vertex_array[3].texCoord0 = vec2(0.0f, 0.0f);
	vertex_array[4].texCoord0 = vec2(1.0f, 1.0f);
	vertex_array[5].texCoord0 = vec2(1.0f, 0.0f);


	vertex_array[6].position = p1;
	vertex_array[7].position = p4;
	vertex_array[8].position = p2;
	normal = vec3::crossproduct(p4 - p1, p2 - p1);
	vertex_array[6].normal = normal;
	vertex_array[7].normal = normal;
	vertex_array[8].normal = normal;

	// need to verify texcoords
	vertex_array[6].texCoord0 = vec2(0.0f, 0.0f);
	vertex_array[7].texCoord0 = vec2(1.0f, 1.0f);
	vertex_array[8].texCoord0 = vec2(1.0f, 0.0f);


	vertex_array[9].position = p2;
	vertex_array[10].position = p4;
	vertex_array[11].position = p3;
	normal = vec3::crossproduct(p4 - p2, p3 - p2);
	vertex_array[9].normal = normal;
	vertex_array[10].normal = normal;
	vertex_array[11].normal = normal;

	// need to verify texcoords
	vertex_array[9].texCoord0 = vec2(0.0f, 0.0f);
	vertex_array[10].texCoord0 = vec2(1.0f, 1.0f);
	vertex_array[11].texCoord0 = vec2(1.0f, 0.0f);



	int index_array[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

	vbo = gfx.CreateIndexBuffer(index_array, 12);
	ibo = gfx.CreateVertexBuffer(vertex_array, 12);

}

int intersect_two_points_plane(const plane_t &p, const vertex_t &a, const vertex_t &b, vertex_t &result)
{
	// plane equation
	// ax + by + cz + d = 0

	// plug in parametric line
	// a*(x0 + vx * t) + b*(y0 + vy * t) + c*(z0 + vz * t) + d = 0

	// solve for t

	// a*x0 + a*vx*t + b*y0 + b*vy*t + c*z0 + c*vz*t + d = 0
	// a*x0 + b*y0 + c*z0 + d + a*vx*t + b*vy*t + c*vz*t = 0
	// a*x0 + b*y0 + c*z0 + d + t*(a*vx + b*vy + c*vz) = 0
	// t*(a*vx + b*vy + c*vz) = -(a*x0 + b*y0 + c*z0 + d)
	// t = -(a*x0 + b*y0 + c*z0 + d) / (a*vx + b*vy + c*vz)


	vec3 origin = a.position;
	vec3 dir = b.position - a.position;

	float denom = (p.normal * dir);
	if (denom == 0.0f)
		return -1;

	float t = -(origin * p.normal - p.d) / denom;
	if (t < 0.0 || t > 1.0)
		return -1;

	result.position = origin + dir * t;
	result.texCoord0 = a.texCoord0 * (1.0f - t) + b.texCoord0 * t;
	result.texCoord1 = a.texCoord1 * (1.0f - t) + b.texCoord1 * t;
	return 0;
}


int intersect_two_points_plane_vec3(const plane_t &p, const vec3 &a, const vec3 &b, vec3 &result)
{
	// plane equation
	// ax + by + cz + d = 0

	// plug in parametric line
	// a*(x0 + vx * t) + b*(y0 + vy * t) + c*(z0 + vz * t) + d = 0

	// solve for t

	// a*x0 + a*vx*t + b*y0 + b*vy*t + c*z0 + c*vz*t + d = 0
	// a*x0 + b*y0 + c*z0 + d + a*vx*t + b*vy*t + c*vz*t = 0
	// a*x0 + b*y0 + c*z0 + d + t*(a*vx + b*vy + c*vz) = 0
	// t*(a*vx + b*vy + c*vz) = -(a*x0 + b*y0 + c*z0 + d)
	// t = -(a*x0 + b*y0 + c*z0 + d) / (a*vx + b*vy + c*vz)


	vec3 origin = a;
	vec3 dir = b - a;

	float denom = (p.normal * dir);
	if (denom == 0.0f)
		return -1;

	float t = -(origin * p.normal - p.d) / denom;
	if (t < 0.0 || t > 1.0)
		return -1;

	result = origin + dir * t;
	return 0;
}



int intersect_two_points_plane2(const plane_t &plane, const vertex_t &a, const vertex_t &b, vertex_t &result)
{
	float da = ((a.position * plane.normal) - plane.d) / plane.normal.magnitude();   // distance plane -> point a
	float db = ((b.position * plane.normal) - plane.d) / plane.normal.magnitude();   // distance plane -> point b

	float s = da / (da - db);   // intersection factor (between 0 and 1)

	if (s < 0 || s > 1)
	{
		return -1;
	}

	result.position = a.position + (b.position - a.position) * s;

	// update tex coords
	result.texCoord0 = a.texCoord0 + (b.texCoord0 - a.texCoord0) * s;
	result.texCoord1 = a.texCoord1 + (b.texCoord1 - a.texCoord1) * s;
	return 0;
}

int intersect_triangle_plane(const plane_t &p, const vertex_t &a, const vertex_t &b, const 	vertex_t &c, vertex_t *result)
{
	// classify points that are out of plane
	inside_t inside;
	float epsilon = 0.0001f;

	// initialize result to given triangle
	result[0] = a;
	result[1] = b;
	result[2] = c;


	float input_area = Signed2DTriArea(a.position, b.position, c.position);


	inside.dword = 0;
	inside.bit.a_in = DistPointPlane(a.position, p.normal, p.d) >= 0;
	inside.bit.b_in = DistPointPlane(b.position, p.normal, p.d) >= 0;
	inside.bit.c_in = DistPointPlane(c.position, p.normal, p.d) >= 0;

	// all points outside plane, early exit
	if (inside.dword == 0)
		return ALL_OUT;

	// all points inside plane, early exit
	if (inside.dword == 7)
		return ALL_IN;

	// easy case, one triangle, two points move in
	if (inside.bit.a_in == 1 && inside.bit.b_in == 0 && inside.bit.c_in == 0)
	{
		vertex_t ab, ac;

		int ret = intersect_two_points_plane(p, a, b, ab);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, a, c, ac);
		if (ret != 0)
		{
			//printf("Error: didnt intersect despite being classified as exiting\r\n");
			// just render normal
			return ALL_OUT;
		}

		result[0] = a;
		result[1] = ab;
		result[2] = ac;

		float output_area = Signed2DTriArea(a.position, ab.position, ac.position);

		if (!((input_area > 0 && output_area > 0) || (input_area <= 0 && output_area <= 0)))
		{
			result[0] = a;
			result[2] = ab;
			result[1] = ac;
			return CLIPPED_EASY;
		}


		output_area = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		if (fabs(output_area) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area, input_area);
			return ALL_OUT;
		}

		return CLIPPED_EASY;
	}
	else if (inside.bit.a_in == 0 && inside.bit.b_in == 1 && inside.bit.c_in == 0)
	{
		vertex_t ba, bc;

		int ret = intersect_two_points_plane(p, b, a, ba);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, b, c, bc);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		result[0] = ba;
		result[1] = b;
		result[2] = bc;


		float output_area = Signed2DTriArea(ba.position, b.position, bc.position);


		if (!((input_area > 0 && output_area > 0) || (input_area <= 0 && output_area <= 0)))
		{
			result[0] = ba;
			result[2] = b;
			result[1] = bc;
			return CLIPPED_EASY;
		}

		output_area = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		if (fabs(output_area) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area, input_area);
			return ALL_OUT;
		}



		return CLIPPED_EASY;

	}
	else if (inside.bit.a_in == 0 && inside.bit.b_in == 0 && inside.bit.c_in == 1)
	{
		vertex_t ca, cb;

		int ret = intersect_two_points_plane(p, c, a, ca);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, c, b, cb);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		result[0] = ca;
		result[1] = cb;
		result[2] = c;


		float output_area = Signed2DTriArea(ca.position, cb.position, c.position);

		if (!((input_area > 0 && output_area > 0) || (input_area <= 0 && output_area <= 0)))
		{
			result[0] = ca;
			result[2] = cb;
			result[1] = c;
			return CLIPPED_EASY;
		}


		output_area = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		if (fabs(output_area) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area, input_area);
			return ALL_OUT;
		}


		return CLIPPED_EASY;
	}

	// hard case, tip chopped off, two triangles
	else if (inside.bit.a_in == 0 && inside.bit.b_in == 1 && inside.bit.c_in == 1)
	{
		vertex_t ab;
		vertex_t ca;

		int ret = intersect_two_points_plane(p, a, b, ab);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, a, c, ca);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		result[0] = ab;
		result[1] = b;
		result[2] = c;

		result[3] = ca;
		result[4] = ab;
		result[5] = c;


		float output_area1 = Signed2DTriArea(ab.position, b.position, c.position);
		float output_area2 = Signed2DTriArea(ca.position, ab.position, c.position);



		if (!((input_area > 0 && output_area1 > 0) || (input_area <= 0 && output_area1 <= 0)))
		{
			result[0] = ab;
			result[2] = b;
			result[1] = c;
		}

		if (!((input_area > 0 && output_area2 > 0) || (input_area <= 0 && output_area2 <= 0)))
		{
			result[3] = ca;
			result[5] = ab;
			result[4] = c;
		}


		output_area1 = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		output_area2 = Signed2DTriArea(result[3].position, result[4].position, result[5].position);

		if (fabs(output_area1 + output_area2) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area1 + output_area2, input_area);
			return ALL_OUT;
		}


		return CLIPPED_HARD;
	}
	else if (inside.bit.a_in == 1 && inside.bit.b_in == 0 && inside.bit.c_in == 1)
	{
		vertex_t ab;
		vertex_t cb;


		int ret = intersect_two_points_plane(p, a, b, ab);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, b, c, cb);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}
		result[0] = a;
		result[1] = ab;
		result[2] = c;

		result[3] = cb;
		result[4] = ab;
		result[5] = c;


		float output_area1 = Signed2DTriArea(a.position, ab.position, c.position);
		float output_area2 = Signed2DTriArea(cb.position, ab.position, c.position);


		if (!((input_area > 0 && output_area1 > 0) || (input_area <= 0 && output_area1 <= 0)))
		{
			result[0] = a;
			result[2] = ab;
			result[1] = c;
		}

		if (!((input_area > 0 && output_area2 > 0) || (input_area <= 0 && output_area2 <= 0)))
		{
			result[3] = cb;
			result[5] = ab;
			result[4] = c;
		}

		output_area1 = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		output_area2 = Signed2DTriArea(result[3].position, result[4].position, result[5].position);


		if (fabs(output_area1 + output_area2) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area1 + output_area2, input_area);
			return ALL_OUT;
		}


		return CLIPPED_HARD;
	}
	else if (inside.bit.a_in == 1 && inside.bit.b_in == 1 && inside.bit.c_in == 0)
	{
		vertex_t ac;
		vertex_t bc;

		int ret = intersect_two_points_plane(p, a, c, ac);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}

		ret = intersect_two_points_plane(p, b, c, bc);
		if (ret != 0)
		{
			printf("Error: didnt intersect despite being classified as exiting Line %d\r\n", __LINE__);
			// just render normal
			return ALL_OUT;
		}
		result[0] = a;
		result[1] = b;
		result[2] = ac;

		result[3] = bc;
		result[4] = ac;
		result[5] = b;

		float output_area1 = Signed2DTriArea(a.position, b.position, ac.position);
		float output_area2 = Signed2DTriArea(bc.position, ac.position, b.position);



		if (!((input_area > 0 && output_area1 > 0) || (input_area <= 0 && output_area1 <= 0)))
		{
			result[0] = a;
			result[2] = b;
			result[1] = ac;
		}

		if (!((input_area > 0 && output_area2 > 0) || (input_area <= 0 && output_area2 <= 0)))
		{
			result[3] = bc;
			result[5] = ac;
			result[4] = b;
		}

		output_area1 = Signed2DTriArea(result[0].position, result[1].position, result[2].position);
		output_area2 = Signed2DTriArea(result[3].position, result[4].position, result[5].position);

		if (fabs(output_area1 + output_area2) > fabs(input_area) + epsilon)
		{
			// clipped triangles should be smaller?
			printf("Clipping error, area increased, discarding triangle Line %d\r\n %f > %f\r\n", __LINE__, output_area1 + output_area2, input_area);
			return ALL_OUT;
		}

		return CLIPPED_HARD;
	}

	printf("Error: intersect_triangle_plane() shouldn't get here\r\n");
	return ALL_OUT;
}

void get_frustum(matrix4 &projection, plane_t *frustum)
{
	float A, B, C, D;

	//left = col1 + col4
	A = projection.m[0] + projection.m[3];
	B = projection.m[4] + projection.m[7];
	C = projection.m[8] + projection.m[11];
	D = projection.m[12] + projection.m[15];

	frustum[0].normal.x = A;
	frustum[0].normal.y = B;
	frustum[0].normal.z = -C;
	frustum[0].d = D;

	//right = -col1 + col4
	A = -projection.m[0] + projection.m[3];
	B = -projection.m[4] + projection.m[7];
	C = -projection.m[8] + projection.m[11];
	D = -projection.m[12] + projection.m[15];

	frustum[1].normal.x = A;
	frustum[1].normal.y = B;
	frustum[1].normal.z = -C;
	frustum[1].d = D;



	//bottom = col2 + col4
	A = projection.m[1] + projection.m[3];
	B = projection.m[5] + projection.m[7];
	C = projection.m[9] + projection.m[11];
	D = projection.m[13] + projection.m[15];

	frustum[2].normal.x = A;
	frustum[2].normal.y = B;
	frustum[2].normal.z = -C;
	frustum[2].d = D;


	//top = -col2 + col4
	A = -projection.m[1] + projection.m[3];
	B = -projection.m[5] + projection.m[7];
	C = -projection.m[9] + projection.m[11];
	D = -projection.m[13] + projection.m[15];

	frustum[3].normal.x = A;
	frustum[3].normal.y = B;
	frustum[3].normal.z = -C;
	frustum[3].d = D;


	//near = col3 + col4
	A = projection.m[2] + projection.m[3];
	B = projection.m[6] + projection.m[7];
	C = projection.m[10] + projection.m[11];
	D = projection.m[14] + projection.m[15];

	frustum[4].normal.x = A;
	frustum[4].normal.y = B;
	frustum[4].normal.z = -C;
	frustum[4].d = D;


	//far = -col3 + col4
	A = -projection.m[2] + projection.m[3];
	B = -projection.m[6] + projection.m[7];
	C = -projection.m[10] + projection.m[11];
	D = -projection.m[14] + projection.m[15];

	frustum[5].normal.x = A;
	frustum[5].normal.y = B;
	frustum[5].normal.z = -C;
	frustum[5].d = D;
}



void calc_plane(plane_t &plane, const vec3 &point1, const vec3 &point2, const vec3 &point3)
{
	vec3	a, b;

	a = point2 - point1;
	b = point3 - point1;

	plane.normal = vec3::crossproduct(a, b);
	plane.normal.normalize();
	plane.d = -(plane.normal * point1);
}


void gen_frustum(Frame *camera, frustum_t *frustum)
{
	float	hNear, wNear, hFar, wFar;
	float	aspectRatio, zNear, zFar, fov;
	vec3	farCoord[4], nearCoord[4], farCenter, nearCenter;
	vec3	vRight;


	vRight = vec3::crossproduct(camera->forward, camera->up);

	aspectRatio = 1.6f;
	zNear = -1.0f;
	zFar = -2001.0f;
	fov = 55.0f;

	hNear = 2.0f * tan(fov / 2.0f) * zNear;
	wNear = hNear * aspectRatio;

	hFar = 2.0f * tan(fov / 2.0f) * zFar;
	wFar = hFar * aspectRatio;

	// far center
	farCenter = camera->pos + camera->forward * zFar;

	// near center
	nearCenter = camera->pos + camera->forward * zNear;

	// far coords
	//upper right
	farCoord[0] = farCenter + camera->up * hFar / 2.0f + vRight * wFar / 2.0f;
	//upper left
	farCoord[1] = farCenter + camera->up * hFar / 2.0f - vRight * wFar / 2.0f;
	//lower right
	farCoord[2] = farCenter - camera->up * hFar / 2.0f + vRight * wFar / 2.0f;
	//lower left
	farCoord[3] = farCenter - camera->up * hFar / 2.0f - vRight * wFar / 2.0f;

	// near coords
	//upper right
	nearCoord[0] = nearCenter + camera->up * hNear / 2.0f + vRight * wNear / 2.0f;
	//upper left
	nearCoord[1] = nearCenter + camera->up * hNear / 2.0f - vRight * wNear / 2.0f;
	//lower right
	nearCoord[2] = nearCenter - camera->up * hNear / 2.0f + vRight * wNear / 2.0f;
	//lower left
	nearCoord[3] = nearCenter - camera->up * hNear / 2.0f - vRight * wNear / 2.0f;


	frustum->pos = camera->pos;
	frustum->forward = camera->forward;

	// be sure normals point into frustum
	calc_plane(frustum->left, farCoord[0], nearCoord[0], nearCoord[2]);
	calc_plane(frustum->right, farCoord[1], nearCoord[3], nearCoord[1]);
	calc_plane(frustum->top, farCoord[0], nearCoord[1], nearCoord[0]);
	calc_plane(frustum->bottom, farCoord[2], nearCoord[2], nearCoord[3]);
	calc_plane(frustum->zNear, nearCoord[0], nearCoord[1], nearCoord[2]);
	calc_plane(frustum->zFar, farCoord[0], farCoord[2], farCoord[1]);
}


int in_frustum(frustum_t *frustum, vec3 &pos)
{
	int left, right, top, bottom, zNear, zFar;

	left = (int)(frustum->left.normal * pos + frustum->left.d);
	right = (int)(frustum->right.normal * pos + frustum->right.d);
	top = (int)(frustum->top.normal * pos + frustum->top.d);
	bottom = (int)(frustum->bottom.normal * pos + frustum->bottom.d);
	zNear = (int)(frustum->zNear.normal * pos + frustum->zNear.d);
	zFar = (int)(frustum->zFar.normal * pos + frustum->zFar.d);

//	printf("L%d R%d T%d B%d N%d F%d\r\n", left > 0, right > 0, top > 0, bottom > 0, zNear > 0, zFar > 0);

	if (left > 0 && right > 0 && top > 0 && bottom > 0 && zNear > 0 && zFar > 0)
		return 1;
	else
		return 0;
}



bool check_aabb_plane(const plane_t &p, vec3 *aabb, plane_t &zNear, bool near_plane)
{
	vec3 result;

	// checking plane against 12 edges of wireframe cube (binary order)

	// three from origin point (min)
	if (intersect_two_points_plane_vec3(p, aabb[0], aabb[1], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}
	if (intersect_two_points_plane_vec3(p, aabb[0], aabb[2], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}
	if (intersect_two_points_plane_vec3(p, aabb[0], aabb[4], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}

	// three from origin point (max)
	if (intersect_two_points_plane_vec3(p, aabb[7], aabb[6], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}
	if (intersect_two_points_plane_vec3(p, aabb[7], aabb[5], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}
	if (intersect_two_points_plane_vec3(p, aabb[7], aabb[3], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}

	// missing two sticks XZ above origins and two pillars going up Y axis

	//(0, 1, 0) -> (1, 1, 0)
	//(0, 1, 0) -> (0, 1, 1)

	if (intersect_two_points_plane_vec3(p, aabb[2], aabb[6], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}

	if (intersect_two_points_plane_vec3(p, aabb[2], aabb[3], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}

	if (intersect_two_points_plane_vec3(p, aabb[5], aabb[1], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}

	if (intersect_two_points_plane_vec3(p, aabb[5], aabb[4], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}

	// two pillars
	//0,0,1 -> 0, 1, 1
	//1,0,0 -> 1,1,0

	if (intersect_two_points_plane_vec3(p, aabb[1], aabb[3], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}

	if (intersect_two_points_plane_vec3(p, aabb[4], aabb[6], result) == 0)
	{
		// check if point is infront of near plane (ie: not behind you)
		if (near_plane || zNear.normal * result + zNear.d > 0)
		{
			return true;
		}
	}

	return false;
}

int in_frustum_bbox(frustum_t *frustum, vec3 &max, vec3 &min)
{
//	int left, right, top, bottom, zNear, zFar;



	// try to clip the 12 edges of the bounding box against the frustum planes
	vec3 result;

	vec3 aabb[8];
	aabb[0] = vec3(min.x, min.y, min.z);
	aabb[1] = vec3(min.x, min.y, max.z);
	aabb[2] = vec3(min.x, max.y, min.z);
	aabb[3] = vec3(min.x, max.y, max.z); 
	aabb[4] = vec3(max.x, min.y, min.z); 
	aabb[5] = vec3(max.x, min.y, max.z);
	aabb[6] = vec3(max.x, max.y, min.z);
	aabb[7] = vec3(max.x, max.y, max.z);

	// check points first
	if (in_frustum(frustum, max))
		return true;

	if (in_frustum(frustum, min))
		return true;


	if (in_frustum(frustum, aabb[1]))
		return true;

	if (in_frustum(frustum, aabb[2]))
		return true;

	if (in_frustum(frustum, aabb[3]))
		return true;

	if (in_frustum(frustum, aabb[4]))
		return true;

	if (in_frustum(frustum, aabb[5]))
		return true;

	if (in_frustum(frustum, aabb[6]))
		return true;



	// Problem, frustum planes extend behind player, dont pass a point because it's inside someplace behind you

	if (check_aabb_plane(frustum->left, aabb, frustum->zNear, false))
		return true;
	if (check_aabb_plane(frustum->right, aabb, frustum->zNear, false))
		return true;
	if (check_aabb_plane(frustum->top, aabb, frustum->zNear, false))
		return true;
	if (check_aabb_plane(frustum->bottom, aabb, frustum->zNear, false))
		return true;
	if (check_aabb_plane(frustum->zNear, aabb, frustum->zNear, true))
		return true;
	
	return false;
}





void gl_rotatef(matrix4 &rmat, float angle, float x, float y, float z)
{
	float s = sin(angle);
	float c = cos(angle);

	rmat.m[0] = x*x*(1 - c) + c;
	rmat.m[1] = x*y*(1 - c) - z * s;
	rmat.m[2] = x*z*(1 - c) + y * s;
	rmat.m[3] = 0.0f;

	rmat.m[4] = y*x*(1 - c) + z*s;
	rmat.m[5] = y*y*(1 - c) + c;
	rmat.m[6] = y*z*(1 - c) - x*s;
	rmat.m[7] = 0.0f;

	rmat.m[8] = x*z*(1 - c) - y*s;
	rmat.m[9] = y*z*(1 - c) + x*s;
	rmat.m[10] = z*z*(1 - c) + c;
	rmat.m[11] = 0.0f;

	rmat.m[12] = 0.0f;
	rmat.m[13] = 0.0f;
	rmat.m[14] = 0.0f;
	rmat.m[15] = 1.0f;
}

void gl_translatef(matrix4 &rmat, float x, float y, float z)
{
	rmat.m[0] = 1.0f;
	rmat.m[1] = 0.0f;
	rmat.m[2] = 0.0f;
	rmat.m[3] = x;

	rmat.m[4] = 0.0f;
	rmat.m[5] = 1.0f;
	rmat.m[6] = 0.0f;
	rmat.m[7] = y;

	rmat.m[8] = 0.0f;
	rmat.m[9] = 0.0f;
	rmat.m[10] = 1.0f;
	rmat.m[11] = z;

	rmat.m[12] = 0.0f;
	rmat.m[13] = 0.0f;
	rmat.m[14] = 0.0f;
	rmat.m[15] = 1.0f;
}

void make_plane(Graphics &gfx, plane_t &plane, vec3 &point, float fExtent, float fStep, int &vbo, int &ibo)
{
	float iLine, angle;
	vec3 vRot;
	vec3 vUp(0.0f, 1.0f, 0.0f);
	matrix4 rmat;
	int num_vert = 0;

	matrix4 mat;

	angle = acos(plane.normal.y / plane.normal.magnitude() );
	vRot = vec3::crossproduct(vUp, plane.normal);


	vertex_t result[4096];
	int index_array[4096];

	
	gl_rotatef(rmat, angle * 180.0f / (float)M_PI, vRot.x, vRot.y, vRot.z);
	mat = rmat;
	gl_translatef(rmat, point.x, point.y, point.z);
	mat = mat * rmat;

	// lines / quads, not triangles
	for (iLine = -fExtent; iLine <= fExtent; iLine += fStep)
	{
		result[num_vert].position.x = iLine;
		result[num_vert].position.y = plane.d;
		result[num_vert].position.z = fExtent;
		num_vert++;
		result[num_vert].position.x = iLine;
		result[num_vert].position.y = plane.d;
		result[num_vert].position.z = -fExtent;
		num_vert++;
		result[num_vert].position.x = fExtent;
		result[num_vert].position.y = plane.d;
		result[num_vert].position.z = iLine;
		num_vert++;
		result[num_vert].position.x = -fExtent;
		result[num_vert].position.y = plane.d;
		result[num_vert].position.z = iLine;
		num_vert++;
	}

	for (int i = 0; i < num_vert; i++)
	{
		index_array[i] = i;
		result[i].position = vec3(mat * vec4(result[i].position, 1.0f));
	}

	vbo = gfx.CreateVertexBuffer(result, num_vert);
	ibo = gfx.CreateIndexBuffer(index_array, num_vert);
}

// From http://www.cubic.org/docs/3dclip.htm -- nice little dos 3d renderer
void setup_frustum(frustum_t *frustum, float project_scale, float width, float height)
{
	float angle_horizontal = atan2(width / 2, project_scale) - 0.0001f;
	float angle_vertical = atan2(height / 2, project_scale) - 0.0001f;
	float sh = sin(angle_horizontal);
	float sv = sin(angle_vertical);
	float ch = cos(angle_horizontal);
	float cv = cos(angle_vertical);

	// left
	frustum->left.normal.x = ch;
	frustum->left.normal.y = 0;
	frustum->left.normal.z = sh;
	frustum->left.d = 0;

	// right
	frustum->right.normal.x = -ch;
	frustum->right.normal.y = 0;
	frustum->right.normal.z = sh;
	frustum->right.d = 0;

	// top
	frustum->top.normal.x = 0;
	frustum->top.normal.y = cv;
	frustum->top.normal.z = sv;
	frustum->top.d = 0;

	// bottom
	frustum->bottom.normal.x = 0;
	frustum->bottom.normal.y = -cv;
	frustum->bottom.normal.z = sv;
	frustum->bottom.d = 0;

	// z-near clipping plane
	frustum->zNear.normal.x = 0;
	frustum->zNear.normal.y = 0;
	frustum->zNear.normal.z = 1;
	frustum->zNear.d = -1;

	// z-near clipping plane
	frustum->zFar.normal.x = 0;
	frustum->zFar.normal.y = 0;
	frustum->zFar.normal.z = -1;
	frustum->zFar.d = -2001.0f;
}

int inline fsgn(float a)
{
	if (a < 0)
		return -1;
	return 1;
}

void cliptoplane(const plane_t &plane, vertex_t *in, int num_in, vertex_t *out, int &num_out)
{
	int i;
	float in_dot[4096];

	// Calculate dot products
	for (i = 0; i < num_in; i++)
	{
		in_dot[i] = plane.normal * in[i].position;
	}
	num_out = 0;
	// calculate to the second to last point:
	for (i = 0; i < num_in - 1; i++)
	{
		// Need no clipping
		if (in_dot[i] >= plane.d)
		{
			out[num_out++] = in[i];
		}

		if (fsgn(in_dot[i] - plane.d) != fsgn(in_dot[i + 1] - plane.d))
		{
			// need clipping
			float scale = in_dot[i] / (in_dot[i] - in_dot[i + 1]);
			vec3 a = in[i].position;
			vec3 b = in[i + 1].position;
			out[num_out].position = a + (b - a) * scale;
			num_out++;
		}
	}

	// check last point for clipping
	if (in_dot[num_in - 1] >= plane.d)
	{
		out[num_out++] = in[num_in - 1];
	}

	// check last line for clipping (warp around!)
	if (fsgn(in_dot[0] - plane.d) != fsgn(in_dot[num_in - 1] - plane.d))
	{
		// need clipping
		float scale = in_dot[0] / (in_dot[0] - in_dot[num_in - 1]);
		vec3 a = in[0].position;
		vec3 b = in[num_in - 1].position;
		out[num_out].position = a + (b - a) * scale;
		num_out++;
	}
}

void init_default_car(carinfo_t *info)
{

	info->seat[0] = vec3(0.0f, 100.0f, -800.0f); // 3rd person
	info->seat[1] = vec3(50.0f, 30.0f, 20.0f);	// driver
	info->seat[2] = vec3(-50.0f, 20.0f, 0.0f);	// passenger
	info->num_seat = 2;

	// static data
	info->cg_to_front = 1.0f;					// meters
	info->cg_to_rear = 1.0f;					// meters
	info->cg_to_ground = 1.0f;				// meters
	info->mass = 1500;						// kg	
	info->inertia = 1500;						// kg.m			
	info->width = 1.5f;						// meters
	info->length = 3.0f;						// meters, must be > wheelbase
	info->wheel_length = 0.7f;
	info->wheel_width = 0.3f;
	info->wheel_base = info->cg_to_front + info->cg_to_rear;
	info->automatic = 1;


	//gearing
	//zeroth gear is reverse, corvette c5
	info->gear_ratio[0] = -2.9f;
	info->gear_ratio[1] = 0;	info->gear_ratio[2] = 2.66f;	info->gear_ratio[3] = 1.78f;	info->gear_ratio[4] = 1.3f;	info->gear_ratio[5] = 1.0f;	info->gear_ratio[6] = 0.74f;	info->gear_ratio[7] = 0.5f;	info->num_gear = 8;
	info->diff_ratio = 3.42f;
	info->efficiency = 1.0f;
	info->wheel_radius = 0.34f;
	info->min_rpm = 900.0f;
	info->max_rpm = 6000.0f;
	info->redline_rpm = 6500.0f;

	//torque in 256rpm increments
	//will be loaded from file in future.
	info->torque_curve[0] = 190.00000f;
	info->torque_curve[1] = 190.452438f;
	info->torque_curve[2] = 195.723282f;
	info->torque_curve[3] = 292.907043f;
	info->torque_curve[4] = 381.565521f;
	info->torque_curve[5] = 387.813019f;
	info->torque_curve[6] = 394.548645f;
	info->torque_curve[7] = 404.267029f;
	info->torque_curve[8] = 412.944153f;
	info->torque_curve[9] = 417.109161f;
	info->torque_curve[10] = 422.738434f;
	info->torque_curve[11] = 433.150970f;
	info->torque_curve[12] = 442.587341f;
	info->torque_curve[13] = 449.529053f;
	info->torque_curve[14] = 457.609619f;
	info->torque_curve[15] = 468.022156f;
	info->torque_curve[16] = 474.529999f;
	info->torque_curve[17] = 474.529999f;
	info->torque_curve[18] = 467.208679f;
	info->torque_curve[19] = 449.854431f;
	info->torque_curve[20] = 432.500214f;
	info->torque_curve[21] = 415.145966f;
	info->torque_curve[22] = 397.791718f;
	info->torque_curve[23] = 380.437469f;
	info->torque_curve[24] = 363.083252f;
	info->torque_curve[25] = 345.729004f;
	info->torque_curve[26] = 328.374756f;
	info->torque_curve[27] = 311.020508f;
	info->torque_curve[28] = 293.666290f;
	info->torque_curve[29] = 276.312042f;
	info->torque_curve[30] = 258.957794f;
	info->torque_curve[31] = 241.603561f;
	info->torque_curve[32] = 224.249313f;
	info->torque_curve[33] = 206.895081f;
	info->torque_curve[34] = 189.540833f;
	info->torque_curve[35] = 172.186600f;
	info->torque_curve[36] = 154.832367f;
	info->torque_curve[37] = 137.478119f;
	info->torque_curve[38] = 120.123878f;
	info->torque_curve[39] = 102.769638f;
	info->torque_curve[40] = 85.415398f;
	info->num_torque = 41;
}



// car is on a plane, need to determine force acting "downhill"
void get_incline_accel(const vec3 &normal, const float mass, const float mu, float &accel)
{
	float theta;

	get_plane_slope(normal, theta);

	// calculate acceleration
	// sin theta is x (sliding) component of gravity, cos theta is y (normal) component of gravity
	accel = -9.8f * (sin(theta) - cos(theta) * mu);
}

inline void project_vector_onto_plane(const vec3 &normal, const vec3 &dir, vec3 &result)
{
	result = dir - normal * (dir * normal);
}

vec3 get_plane_direction_gradient(const vec3 &normal)
{
	vec3 result;

	vec3 up(0.0f, 1.0f, 0.0f);
	vec3 sideways = vec3::crossproduct(normal, up);
	sideways.normalize();

	vec3 direction = vec3::crossproduct(sideways, up);
	direction.normalize();

	// project onto plane
	project_vector_onto_plane(normal, direction, result);

	// gradient goes uphill
	if (result.y < 0)
		return -result;

	return result;
}

void refract(vec3 &normal, vec3 &vec, float n, vec3 &refract)
{
	float dot = (normal * vec);
	float k = 1 - n*n * (1 - dot * dot);

	//n = index of refraction
	if (k < 0.0)
	{
		refract = vec3(0.0f, 0.0f, 0.0f);
	}
	else
	{
		refract = (vec * n) - normal * (n * (dot)+sqrt(k));
	}
}

int enqueue(queue_t *queue, unsigned char *buffer, int size)
{
	int        i;

	if (queue->size == SIZE_QUEUE)
	{
		return 0;
	}

	for (i = 0; i < size && queue->size != SIZE_QUEUE; i++)
	{
		queue->buffer[queue->tail++] = buffer[i];
		queue->size++;
		if (queue->tail == SIZE_QUEUE)
			queue->tail = 0;
	}
	return i;
}

int enqueue_front(queue_t *queue, unsigned char *buffer, int size)
{
	int        i;

	if (queue->size == SIZE_QUEUE)
		return 0;

	for (i = 0; i < size && queue->size != SIZE_QUEUE; i++)
	{
		if (queue->head == 0)
		{
			queue->head = SIZE_QUEUE;
		}
		queue->buffer[--queue->head] = buffer[(size - 1) - i];
		queue->size++;
	}
	return i;
}


int dequeue(queue_t *queue, unsigned char *buffer, int size)
{
	int        i;

	for (i = 0; i < size && queue->size != 0; i++)
	{
		buffer[i] = queue->buffer[queue->head++];
		//queue->buffer[queue->head - 1] = ' '; // empty space for debugging
		queue->size--;
		if (queue->head == SIZE_QUEUE)
		{
			queue->head = 0;
		}
	}
	return i;
}

int dequeue_peek(queue_t *queue, unsigned char *buffer, int size)
{
	int        i;
	int		head = queue->head;
	int		qsize = queue->size;

	for (i = 0; i < size && qsize != 0; i++)
	{
		buffer[i] = queue->buffer[head++];
		//queue->buffer[queue->head - 1] = ' '; // empty space for debugging
		qsize--;
		if (head == SIZE_QUEUE)
		{
			head = 0;
		}
	}
	return i;
}

vec2 WorldToScreen(const vec4 &pos, const matrix4 &mvp, const int width, const int height)
{
	vec4 clipCoords = mvp * pos;

	vec3 NDC = vec3(clipCoords) / clipCoords.w;

	vec2 screen;

	screen.x = (NDC.x * 0.5f) + 0.5f;
	screen.y = (NDC.y * 0.5f) + 0.5f;
	screen.x *= width;
	screen.y *= height;

	return screen;
}


void make_frame(const vec3 &dir, const vec3 &almost_orthangonal, Frame &frame)
{
	frame.forward = dir;
	frame.up = almost_orthangonal;

	vec3 right = vec3::crossproduct(frame.forward, frame.up);

	frame.up = vec3::crossproduct(right, frame.forward);
}

bool gluUnProject(float x, float y, float z, matrix4 &model, matrix4 &proj, int width, int height, float &outx, float &outy, float &outz)
{
	vec2 screen;
	vec4 clip;

	screen.x = (float)(x) / width;
	screen.y = (float)(height - y) / height;


	clip.x = 2.0f * (screen.x - 0.5f);
	clip.y = 2.0f * (screen.y - 0.5f);
	clip.z = 2.0f * z - 1.0f;
	clip.w = 1.0f;

	matrix4 mvp = model * proj;
	matrix4 imvp;

	imvp = mvp.inverse();

	vec4 out = imvp * clip;
	if (out.w == 0.0)
		return false;

	outx = out.x / out.w;
	outy = out.y / out.w;
	outz = out.z / out.w;

	return true;
}


void make_cone(float radius, float height, vec3& center, int step, vertex_t* triangles, unsigned int& num_triangle)
{
	vertex_t points[8192];
	unsigned int num_point = 0;

	// first point for fan
	points[num_point].position = center;
	num_point++;

	// make the cone top
	for (int i = 0; i < 360; i += 360 / step)
	{
		float x = radius * cos(i * 3.141592 / 180) + center.x;
		float y = radius * sin(i * 3.141592 / 180) + center.y;
		float z = center.z + height;

		points[num_point].position = vec3(x, y, z);
		num_point++;
	}

	// make last point first point to complete circle
	points[num_point] = points[1];
	num_point++;



	// make the cone bottom
	// first point is the center
	points[num_point].position = vec3(center.x, center.y, center.z + height);
	num_point++;


	int bottom_first = num_point;

	for (int i = 0; i < 360; i += 360 / step)
	{
		float x = radius * cos(i * 3.141592 / 180) + center.x;
		float y = radius * sin(i * 3.141592 / 180) + center.y;
		float z = center.z + height;

		points[num_point].position = vec3(x, y, z);
		num_point++;
	}


	// make last point first point to complete circle
	points[num_point] = points[bottom_first];
	num_point++;


	// Convert triangle fan to regular triangles
	int first = 0;
	num_triangle = 0;

	for (int i = 1; i < num_point / 2 - 1; i++)
	{
		int a = first;
		int b = i % num_point;
		int c = (i + 1) % num_point;

		// add triangle using last two points
		triangles[num_triangle++] = points[a];
		triangles[num_triangle++] = points[b];
		triangles[num_triangle++] = points[c];

		// add triangles both directions to avoid weird culling
		triangles[num_triangle++] = points[c];
		triangles[num_triangle++] = points[b];
		triangles[num_triangle++] = points[a];
	}


	first = bottom_first - 1;

	for (int i = bottom_first; i < num_point; i++)
	{
		int a = first;
		int b = i % num_point;
		int c = (i + 1) % num_point;

		// add triangle using last two points
		triangles[num_triangle++] = points[a];
		triangles[num_triangle++] = points[b];
		triangles[num_triangle++] = points[c];


		// add triangles both directions to avoid weird culling
		triangles[num_triangle++] = points[c];
		triangles[num_triangle++] = points[b];
		triangles[num_triangle++] = points[a];
	}
}


// quickhull 2d -- will do 3d next, but figure should do 2d first

#define MAX_POINT 4096
typedef struct
{
	vec2 a;
	vec2 b;
} line_t;


typedef struct
{
	vec2 a;
	vec2 b;
	vec2 c;
} triangle_2d_t;

int quick_hull(vec2 *point, int num_point, vec2 *hull, int &num_hull);
int find_hull(line_t &line, vec2 *point, int num_point, vec2 *hull, int &num_hull);


float det(vec2 &p1, vec2 &p2, vec2 &p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

// essentially just the determinant (see above) 
float point_above_line(vec2 &p, line_t &line)
{
	return (line.b.x - line.a.x) * (p.y - line.a.y) - (line.b.y - line.a.y) * (p.x - line.a.x);
}

float triangle_area(float x1, float y1, float x2, float y2, float x3, float y3)
{
	return (float)fabs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
}

bool point_in_triangle(vec2 &p, triangle_2d_t &tri)
{
	float d1 = det(p, tri.a, tri.b);
	float d2 = det(p, tri.b, tri.c);
	float d3 = det(p, tri.c, tri.a);

	int all_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	int all_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(all_neg && all_pos);
}



void closest_point_to_line(vec2 &p, line_t &line, float &t, vec2 &d)
{
	vec2 ab = line.b - line.a;

	t = ((p - line.a) * ab) / (ab * ab);

	if (t < 0.0f)
	{
		t = 0.0f;
	}
	if (t > 1.0f)
	{
		t = 1.0f;
	}

	d = line.a + ab * t;
}


float distance_to_line(vec2 &p, line_t &line)
{
	return fabs((line.b.x - line.a.x) * (p.y - line.a.y) - (p.x - line.a.x) * (line.b.y - line.a.y)) \
		/ sqrt(line.b.x - line.a.x * (line.b.x - line.a.x) + (line.b.y - line.a.y) * (line.b.y - line.a.y));
}



int add_point(vec2 *hull, int &num_hull, vec2 &p)
{
	int exists = 0;

	// check if these points already exist in the hull before adding them
	for (int i = 0; i < num_hull; i++)
	{
		if (hull[i].x == p.x &&
			hull[i].y == p.y)
		{
			exists = 1;
			break;
		}
	}

	if (exists == 0)
	{
		printf("Adding point to hull (%f, %f)\n", p.x, p.y);
		hull[num_hull++] = p;
		return 1;
	}

	return 0;
}

int add_line(vec2 *hull, int &num_hull, line_t &line)
{
	int a_exists = 0;
	int b_exists = 0;
	int ret = 0;

	// check if these points already exist in the hull before adding them
	for (int i = 0; i < num_hull; i++)
	{
		if (hull[i].x == line.a.x &&
			hull[i].y == line.a.y)
		{
			a_exists = 1;
		}

		if (hull[i].x == line.b.x &&
			hull[i].y == line.b.y)
		{
			b_exists = 1;
		}

		if (a_exists && b_exists)
		{
			break;
		}
	}

	if (a_exists == 0)
	{
		printf("Adding point to hull (%f, %f)\n", line.a.x, line.a.y);
		hull[num_hull++] = line.a;
		ret++;
	}
	if (b_exists == 0)
	{
		printf("Adding point to hull (%f, %f)\n", line.b.x, line.b.y);
		hull[num_hull++] = line.b;
		ret++;
	}

	return ret;
}


int quick_hull(vec2 *point, int num_point, vec2 *hull, int &num_hull)
{
	float max_point = FLT_MIN;
	float min_point = FLT_MAX;
	int max_index = -1;
	int min_index = -1;

	if (num_point < 3)
	{
		return -1;
	}

	// find max/min point
	for (int i = 0; i < num_point; i++)
	{
		if (point[i].x > max_point)
		{
			max_point = point[i].x;
			max_index = i;
		}
		if (point[i].x < min_point)
		{
			min_point = point[i].x;
			min_index = i;
		}
	}


	// if duplicates exist, find max/min y as secondary criteria
	for (int i = 0; i < num_point; i++)
	{
		if (point[i].x == max_point)
		{
			if (point[i].y > point[max_index].y)
			{
				max_index = i;
			}
		}
		if (point[i].x == min_point)
		{
			if (point[i].y < point[min_index].y)
			{
				min_index = i;
			}
		}
	}

	line_t line;

	line.a = vec2(point[min_index].x, point[min_index].y);
	line.b = vec2(point[max_index].x, point[max_index].y);

	printf("max line (%f, %f) (%f, %f)\n", line.a.x, line.a.y, line.b.x, line.b.y);
	add_line(hull, num_hull, line);

	// divide point into two sets above/below line
	vec2 point_above[4096];
	int num_above = 0;
	int j = 0;
	for (int i = 0; i < num_point; i++)
	{
		if (point_above_line(point[i], line) >= 0)
		{
			point_above[j++] = point[i];
			printf("Point above (%f, %f)\n", point[i].x, point[i].y);
		}
	}
	num_above = j;


	// divide point into two sets above/below line
	vec2 point_below[4096];
	int num_below = 0;
	j = 0;
	for (int i = 0; i < num_point; i++)
	{
		if (point_above_line(point[i], line) < 0)
		{
			point_below[j++] = point[i];
			printf("Point below (%f, %f)\n", point[i].x, point[i].y);
		}
	}
	num_below = j;


	// make recursive call
	find_hull(line, point_above, num_above, hull, num_hull);
	find_hull(line, point_below, num_below, hull, num_hull);

	return 0;
}

int find_hull(line_t &line, vec2 *point, int num_point, vec2 *hull, int &num_hull)
{
	float max_dist = -1000.0f;
	int max_index = -1;


	if (num_point == 0)
	{
		return -1;
	}

	// find furthest point to from line
	for (int i = 0; i < num_point; i++)
	{
		if (point[i].x == line.a.x && point[i].y == line.a.y)
		{
			// dont consider line points
			continue;
		}

		if (point[i].x == line.b.x && point[i].y == line.b.y)
		{
			// dont consider line points
			continue;
		}

		float dist = distance_to_line(point[i], line);

		if (dist > max_dist)
		{
			max_dist = dist;
			max_index = i;
		}
	}


	if (max_index == -1)
	{
		// no way to make triangle
		return 0;
	}

	// make a triangle with the furthest point from line
	triangle_2d_t tri;

	tri.a = line.a;
	tri.b = line.b;
	tri.c = point[max_index];

	// calculate area to be sure we dont have identical points
	float area = triangle_area(tri.a.x, tri.a.y, tri.b.x, tri.b.y, tri.c.x, tri.c.y);
	if (area == 0.0)
	{
		// no way to make triangle, this check probably isnt needed anymore, but keeping for good measure
		return 0;
	}


	printf("Triangle point (%f, %f)\n", point[max_index].x, point[max_index].y);
	add_point(hull, num_hull, point[max_index]);


	// remove points inside triangle from consideration
	for (int i = 0; i < num_point; i++)
	{
		if (point_in_triangle(point[i], tri))
		{
			if (point[i].x == tri.a.x && point[i].y == tri.a.y)
			{
				// dont drop triangle points
				continue;
			}
			else if (point[i].x == tri.b.x && point[i].y == tri.b.y)
			{
				// dont drop triangle points
				continue;
			}
			else if (point[i].x == tri.c.x && point[i].y == tri.c.y)
			{
				// dont drop triangle points
				continue;
			}


			printf("Dropping point (%f, %f)\n", point[i].x, point[i].y);
			// can remove this point from potential hull set
			// NaN in comparisons is always false, and is always not equal
			point[i].x = NAN;
			point[i].y = NAN;
		}
	}


	// make two lines from the triangles sides and repeat
	line_t line_a;
	line_t line_b;

	line_a.a = tri.a;
	line_a.b = tri.c;
	line_b.a = tri.b;
	line_b.b = tri.c;

	vec2 point_above1[MAX_POINT];
	vec2 point_below1[MAX_POINT];
	int num_above1 = 0;
	int num_below1 = 0;
	int j = 0;
	int k = 0;
	for (int i = 0; i < num_point; i++)
	{
		if (point[i].x == tri.a.x && point[i].y == tri.a.y)
		{
			// dont add triangle points
			continue;
		}
		else if (point[i].x == tri.b.x && point[i].y == tri.b.y)
		{
			// dont add triangle points
			continue;
		}
		else if (point[i].x == tri.c.x && point[i].y == tri.c.y)
		{
			// dont add triangle points
			continue;
		}


		if (point_above_line(point[i], line_a) >= 0)
		{
			point_above1[j++] = point[i];
		}

		// do seperate comparison so the NaN's go away
		if (point_above_line(point[i], line_a) < 0)
		{
			point_below1[k++] = point[i];
		}
	}
	num_above1 = j;
	num_below1 = k;


	vec2 point_above2[MAX_POINT];
	vec2 point_below2[MAX_POINT];
	int num_above2 = 0;
	int num_below2 = 0;
	j = 0;
	k = 0;
	for (int i = 0; i < num_point; i++)
	{
		if (point[i].x == tri.a.x && point[i].y == tri.a.y)
		{
			// dont add triangle points
			continue;
		}
		else if (point[i].x == tri.b.x && point[i].y == tri.b.y)
		{
			// dont add triangle points
			continue;
		}
		else if (point[i].x == tri.c.x && point[i].y == tri.c.y)
		{
			// dont add triangle points
			continue;
		}

		if (point_above_line(point[i], line_b) > 0)
		{
			point_above2[j++] = point[i];
		}

		// do seperate comparison so the NaN's go away
		if (point_above_line(point[i], line_b) < 0)
		{
			point_below2[k++] = point[i];
		}
	}
	num_above2 = j;
	num_below2 = k;

	// process each side of both lines, interior points will die due to NaN
	if (num_above1)
		find_hull(line_a, point_above1, num_above1, hull, num_hull);
	if (num_below1)
		find_hull(line_a, point_below1, num_below1, hull, num_hull);
	if (num_above2)
		find_hull(line_b, point_above2, num_above2, hull, num_hull);
	if (num_below2)
		find_hull(line_b, point_below2, num_below2, hull, num_hull);

	return 0;
}

