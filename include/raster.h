#include "include.h"

#ifndef RASTER_H
#define RASTER_H


void raster_triangles(int *pixels, int *zbuffer, int width, int height, matrix4 &mvp, int *index_array, vertex_t *vertex_array, int *texture_array, int start_index, int start_vertex, int num_index, int num_verts);
void barycentric_triangle(int *pixels, int *zbuffer, int width, int height, int x1, int y1, int z1, int c1, int x2, int y2, int z2, int c2, int x3, int y3, int z3, int c3);
void span_triangle(int *pixels, int *zbuffer, int width, int height, int x1, int y1, int z1, int c1, int x2, int y2, int z2, int c2, int x3, int y3, int z3, int c3);
void clip2d_sutherland_hodgman(int width, int height, POINT *points, int &num_point);
void halfspace_triangle(int *pixels, int *zbuffer, int width, int height, const vec2 &v1, const vec2 &v2, const vec2 &v3);
void halfspace_triangle_fast(int *pixels, int *zbuffer, int width, int height, const vec2 &v1, const vec2 &v2, const vec2 &v3);
#endif
