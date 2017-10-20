#include "include.h"

#ifndef COMMON_H
#define COMMON_H

//random in the range [0, 1]
#define random()			((rand () & 0x7fff) / ((float)0x7fff))
//random in the range [-1, 1]
#define crandom() (2.0f * (random() - 0.5f))


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
void tessellate_quadratic_bezier_surface(vec3 *control, vertex_t *&vertex, int *&index, int &num_vertex, int &num_index, float level);
char *get_file(char *filename, int *size);
int get_zipfile(char *zipfile, char *file, unsigned char **data, int *size);
int list_zipfile(char *zipfile, char *filelist);
int write_file(char *filename, const char *bytes, int size);
float sign(float x);
void newlinelist(char *filename, char **list, unsigned int &num);
void calc_hash(char *filename, char *hash);

void navdata_to_graph(ref_t *&ref, graph_node_t *&node, vector<Entity *> &entity_list, int start);
void print_graph(graph_node_t *node, int num_node);
void print_path(int *path, int path_length, graph_node_t *node);
float rand_float(float fMin, float fAdd);
int gen_spiral(Graphics &gfx, unsigned int &ibo, unsigned int &vbo);
int gen_lightning(Graphics &gfx, unsigned int &ibo, unsigned int &vbo);

void delta_compress(char *output, char *input, char *delta, int size);
void delta_uncompress(char *output, char *input, char *delta, int size);
void runlength_encode(uint8_t *output, rletable_t *table, int *table_size, uint8_t *input, unsigned int *size);
void runlength_decode(uint8_t *output, rletable_t *table, uint8_t *input, unsigned int *size);
char *get_pakfile(char *pakfile, char *file);
char *get_wadfile(char *wadfile, char *lump, int *lump_size);
void lump_to_wave(char *lump_data, int size, wave_t *wave);
void write_bitmap(char *filename, int width, int height, int *data);
void ping_time_start(int sequence);
double ping_time_end(int sequence);
float clamp(float value, float min, float max);

float random_float();

double GetCounter(double freq);
extern double freq;
void pid_controller(const vec3 &target, const float timestep, const vec3 &pos, vec3 &thrust, const float kd);

void init_pid(pid_state_t *pid);
void update_pid(pid_state_t *pid, const vec3 &target, const vec3 &position, vec3 &output);

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

#endif
