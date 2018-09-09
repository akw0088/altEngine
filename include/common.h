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

#include "include.h"

#ifndef COMMON_H
#define COMMON_H

//random in the range [0, 1]
#define random()			((rand () & 0x7fff) / ((float)0x7fff))
//random in the range [-1, 1]
#define crandom() (2.0f * (random() - 0.5f))

// When colliding with a wall, velocity towards the wall is clipped
// This is extra clipping past what is necessary, making you stay further away from walls
#define BOUNCE		1.2f


#pragma pack(1)
typedef struct
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
} pixel_t;
#pragma pack(8)

typedef struct
{
	int sequence;
	double time;
} ping_t;


//float fabs(float val);
void gen_normalmap(float scale, const pixel_t *pixel, pixel_t *pixelout, int width, int height);
bool aabb_visible(vec3 &min, vec3 &max, matrix4 &mvp);
bool RayTriangleMT(vec3 &origin, vec3 &dir, vec3 &a, vec3 &b, vec3 &c, float &t, float &u, float &v);
bool RaySphere(vec3 &origin, vec3 &dir, vec3 &sphere, float radius, float &t);
bool RayPlane(vec3 &origin, vec3 &dir, vec3 &normal, float d, vec3 &point);
bool RayBoxSlab(vec3 &origin, vec3 &dir, vec3 &min, vec3 &max, float &distance);
void quadratic_bezier_curve(vec3 &a, vec3 &b, vec3 &c, float time, vec3 &out);
void cubic_bezier_curve(vec3 &a, vec3 &b, vec3 &c, vec3 &d, float time, vec3 &out);
void bicubic_bezier_surface(vec3 *control, float time_x, float time_y, vec3 &out);
void quadratic_bezier_surface(vec3 *control, float time_x, float time_y, vec3 &out);
void make_skybox(Graphics &gfx, unsigned int num_vertex, unsigned int &num_index, int &skybox_vertex, int &skybox_index, bool sphere);
void print_entity_meminfo(vector<Entity *> &entity_list);

char *get_file(char *filename, int *size);
int get_zipfile(char *zipfile, char *file, unsigned char **data, int *size);
int list_zipfile(char *zipfile, char *filelist);
int write_file(char *filename, const char *bytes, int size);
float sign(float x);
void newlinelist(char *filename, char **list, unsigned int &num, char **file);
void calc_hash(char *filename, char *hash);

void navdata_to_graph(ref_t *&ref, graph_node_t *&node, vector<Entity *> &entity_list, int start);
void print_graph(graph_node_t *node, int num_node);
void print_path(int *path, int path_length, graph_node_t *node);
float rand_float(float fMin, float fAdd);
int gen_spiral(Graphics &gfx, unsigned int &ibo, unsigned int &vbo);
int gen_lightning(Graphics &gfx, unsigned int &ibo, unsigned int &vbo);
void WriteObj(char *filename, vertex_t *vertex_array, unsigned int num_vertex, unsigned int *index_array, unsigned int num_index);

void delta_compress(char *output, char *input, char *delta, int size);
void delta_uncompress(char *output, char *input, char *delta, int size);
void runlength_encode(uint8_t *output, rletable_t *table, int *table_size, uint8_t *input, unsigned int *size);
void runlength_decode(uint8_t *output, rletable_t *table, uint8_t *input, unsigned int *size);
char *get_pakfile(char *pakfile, char *file);
char *get_wadfile(char *wadfile, char *lump, int *lump_size, char **file);
void lump_to_wave(char *lump_data, int size, wave_t *wave);
void write_bitmap(char *filename, int width, int height, int *data);
void ping_time_start(int sequence);
double ping_time_end(int sequence);
int clamp(int value, int min, int max);
float clamp(float value, float min, float max);
int gjk(const vec3 *shape1, const vec3 *shape2, const int iterations, const int num_vert_one, const int num_vert_two);
void ClipVelocity(vec3 &in, vec3 &normal);

float random_float();

double GetCounter(double freq);
extern double freq;
void pid_controller(const vec3 &target, const float timestep, const vec3 &pos, vec3 &thrust, const float kd);

void init_pid(pid_state_t *pid);
void update_pid(pid_state_t *pid, const vec3 &target, const vec3 &position, vec3 &output);
vec3 para_spline(vec3 *control, int num_control, float t);

int auto_complete(const char *a, const char *b);

int separating_axis_theorem(const vec3 *box_a, const vec3 *box_b);
void dns_query(Socket &sock);

struct cpuinfo
{
	char	vendor[13], serial[9];
	int	support, signature, brandid, msr, config, extsupport, ext,
		stepping, model, family, type, extmodel, extfamily;
};

struct cpuinfo2
{
	char	vendor[13], serial[9];
	int	support, signature, brandid, msr, config, extsupport, ext,
		stepping, model, family, type, extmodel, extfamily;
};


// Lerp between A and B where time is within [0,1]
inline void lerp(vec3 &a, vec3 &b, float time, vec3 &out)
{
	out = a * (1 - time) + b * time;
}

void make_torus(Graphics &gfx, int numc, int numt, float r1, float r2, float scale, vertex_t *vertex, unsigned int &num_vertex, int *index, unsigned int &num_index, bool invert, int &vbo, int &ibo);
bool get_barycentric(float x, float y,
	const vec3 &a, const vec3 &b, const vec3 &c,
	float &lam1, float &lam2, float &lam3);

#endif
