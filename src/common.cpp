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
#ifdef __linux__
#define stricmp strcasecmp
#endif

#define DMESG_SIZE 2048
extern char dmesg[DMESG_SIZE][1024];
extern int dmesg_index;

float newtonSqrt(float x);
int fceil(int num, int denom);

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

//vector<std::pair<char *, char *>> file_list;

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

	/*
	std::pair<char *, char *> pair;

	pair.first = new char[256];
	strcpy(pair.first, filename);
	pair.second = buffer;
	file_list.push_back(pair);
	*/
    return buffer;
}

/*
void delete_file(char *fileptr)
{
	for (int i = 0; i < file_list.size(); i++)
	{
		if (fileptr == file_list[i].second)
		{
			delete[] fileptr;
			break;
		}
	}
}
*/
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

	int size = 0;
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
		data = (unsigned char *)get_file(file_name, NULL);

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
#ifdef DIRECTX
	unsigned char *bytes = stbi_load_from_memory(data, size, &width, &height, &components, 0);
	if (bytes == NULL)
	{
		debugf("Unable to load texture %s from pk3\n", file_name);
		free((void *)data);
		return load_texture(gfx, file_name, clamp, false, anisotropic);
	}

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
	stbi_image_free(bytes);
	delete [] data;
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
		components =  GL_RED;
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
		tex_object = gfx.LoadTexture(width, height, 4, format, pBits, clamp, anisotropic);
	}
	else
	{
		tex_object = gfx.LoadTexture(width, height, components, format, bytes, clamp, anisotropic);
	}
	stbi_image_free(bytes);
	delete [] data;
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
	int size = 0;

	char *data = get_file(filename, &size);
	if (data == NULL)
	{
		sprintf(hash, "missing file");
		return;
	}

	md5sum(data, size, hash);
	delete [] data;
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

This code (translated from matlab) performed phenomenology
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

	for( int i = 0; i < iteration_allowed; i++)
	{
		ab = b - a;
		ao = -a;
		ac = c - a;
		vec3 ad = d - a;

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

