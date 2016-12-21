#include "include.h"

#ifndef COMMON_H
#define COMMON_H

bool aabb_visible(vec3 &min, vec3 &max, matrix4 &mvp);
bool RayTriangleMT(vec3 &origin, vec3 &dir, vec3 &a, vec3 &b, vec3 &c, float &t, float &u, float &v);
bool RaySphere(vec3 &origin, vec3 &dir, vec3 sphere, float radius, float &t);
bool RayPlane(vec3 &origin, vec3 &dir, vec3 &normal, float d, vec3 &point);
bool RayBoxSlab(vec3 &origin, vec3 &dir, vec3 &min, vec3 &max, float &distance);
void lerp(vec3 &a, vec3 &b, float time, vec3 &out);
void quadratic_bezier_curve(vec3 &a, vec3 &b, vec3 &c, float time, vec3 &out);
void cubic_bezier_curve(vec3 &a, vec3 &b, vec3 &c, vec3 &d, float time, vec3 &out);
void bicubic_bezier_surface(vec3 *control, float time_x, float time_y, vec3 &out);
void quadratic_bezier_surface(vec3 *control, float time_x, float time_y, vec3 &out);
void tesselate_quadratic_bezier_surface(vec3 *control, vertex_t *vertex, int *index, int &num_index, float level);
char *get_file(char *filename, int *size);
int get_zipfile(char *zipfile, char *file, unsigned char **data, int *size);
int write_file(char *filename, char *bytes, int size);
float sign(float x);
void newlinelist(char *filename, char **list, int &num);
bool check_hash(char *filename, char *checkhash, char *hash);

#endif
