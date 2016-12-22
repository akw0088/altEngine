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
	int xp = 0;
	int xn = 0;
	int yp = 0;
	int yn = 0;
	int zp = 0;
	int zn = 0;


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
		if (aabb[i].x > aabb[i].w)
		{
			xp = 1;
		}
		if (aabb[i].x < -aabb[i].w)
		{
			xn = 1;
		}
		if (aabb[i].y > aabb[i].w)
		{
			yp = 1;
		}
		if (aabb[i].y < -aabb[i].w)
		{
			yn = 1;
		}
		if (aabb[i].z > aabb[i].w)
		{
			zp = 1;
		}
		if (aabb[i].z < -aabb[i].w)
		{
			zn = 1;
		}
	}

	// all points were outside of one plane
	if ((xp + xn + yp + yn + zp + zn) == 6)
	{
		visible = false;
	}	

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

bool RaySphere(vec3 &origin, vec3 &dir, vec3 sphere, float radius, float &t)
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

	if (denom == 0.0f)
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
	if (dir.x != 0.0)
	{
		float t1 = (min.x - origin.x) / dir.x;
		float t2 = (max.x - origin.x) / dir.x;

		tmin = MAX(tmin, MIN(t1, t2));
		tmax = MIN(tmax, MAX(t1, t2));
	}

	// Y coordinate
	if (dir.y != 0.0)
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

// Lerp between A and B where time is within [0,1]
inline void lerp(vec3 &a, vec3 &b, float time, vec3 &out)
{
	out = a * (1 - time) + b * time;
}

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
void tesselate_quadratic_bezier_surface(vec3 *control, vertex_t *vertex, int *index, int &num_index, float level)
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


//TODO, make this a ring buffer instead of using malloc
int debugf(const char *format, ...)
{
    va_list args;
    char str[512] = { 0 };
    
    
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);
    printf("%s", str);
    
    
    unsigned int width = 60;
    
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
            memcpy(line, pstr, size);
            line[width] = '\0';
            Menu::console_buffer.push_back(line);
            pstr += width;
        }
	if (dmesg_index >= DMESG_SIZE)
		dmesg_index  = 0;
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

int write_file(char *filename, char *bytes, int size)
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

void newlinelist(char *filename, char **list, int &num)
{
	if (filename == NULL || list == NULL)
	{
		printf("newlinelist given null values\n");
		return;
	}

	char *file = get_file(filename, NULL);
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

int load_texture_pk3(Graphics &gfx, char *file_name, char **pk3_list, int num_pk3)
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
	if (data == NULL)
	{
//		debugf("Unable to load texture %s\n", file_name);
		return load_texture(gfx, file_name);
	}
	else
	{
		//debugf("Loaded %s from pk3\n", file_name);
	}

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
		free((void *)data);
		return 0;
	}

	tex_object = gfx.LoadTexture(width, height, components, format, bytes);
	stbi_image_free(bytes);
	free((void *)data);

#ifndef DIRECTX
	if (format != GL_RGBA)
	{
		// negative means it has an alpha channel
		return -tex_object;
	}
#endif
	return tex_object;
}

// Need asset manager class so things arent doubly loaded
int load_texture(Graphics &gfx, char *file_name)
{
	int width, height, components, format;
	int tex_object;

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
		free((void *)data);
		return 0;
	}

	tex_object = gfx.LoadTexture(width, height, components, format, bytes);
	stbi_image_free(bytes);
	free((void *)data);

#ifndef DIRECTX
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
	free((void *)data);
	if (strcmp(hash, md5match) == 0)
		return true;
	else
		return false;
}
