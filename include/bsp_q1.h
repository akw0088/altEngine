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

// need to clean up all this fixed point math and helper macros if unused
#define VERTEX(x) ((vec3 *) ((char *) data.dvertexes + (x)*4 + (x)*8))
#define fix_cint(x)			(((x)+65535) >> 16)
#define FLOAT_TO_INT(x)		((chop_temp = (x) + BIG_NUM), *(int*)(&chop_temp))
#define FLOAT_TO_FIX(x)		((chop_temp = (x) + BIG_NUM/65536.0), *(int*)(&chop_temp))
#define BIG_NUM				((float) (1 << 26) * (1 << 26) * 1.5)
#define fix_int(x)			((x) >> 16)
#define float_to_fix(x)		((fix) ((x) * 65536))
#define fix_cint(x)			(((x)+65535) >> 16)
#define fix_floor(x)		((x) & 0xffff0000)
#define fix_ceil(x)			fix_floor((x)+0xffff)
#define fix_make(a,b)		(((a) << 16) + (b))
#define MAX_MAP_FACES		65535
#define MAX_MAP_LEAFS		10001

#define CC_OFF_LEFT			1
#define CC_OFF_RIGHT		2
#define CC_OFF_TOP			4
#define CC_OFF_BOT			8
#define CC_BEHIND			16

#define DIST				256.0    // dist to switch first mip level
#define DIST2				(DIST*DIST)
#define MAX_CACHED_SURFACES (1024)
#define ADJ_SURFACE(x)    ((x) & (MAX_CACHED_SURFACES-1))
#define MAX_CACHE            (1024*1024)

#define SUBDIV_SHIFT  4
#define SUBDIV        (1 << SUBDIV_SHIFT)
#define FLOAT_POSITIVE(x)   (* (int *) (x) >= 0)






class Q1Bsp
{
public:
	Q1Bsp();
	int load(Graphics &gfx, char *filename);
	void bsp_render_node(Graphics &gfx, int node, vec3 &cam_loc);
	void render_node_faces(Graphics &gfx, int node, int side);
	void draw_face(Graphics &gfx, int face);
	void build_face(Graphics &gfx, int face);
	void change_axis();
	void render(Graphics &gfx, vec3 &cam_loc);
	int visit_visible_leaves(vec3 &cam_loc);
	int find_leaf(vec3 &loc);
	int bsp_find_visible_nodes(int node);
	void bsp_explore_node(int node);
	void mark_leaf_faces(int leaf);
	void get_tmap(q1::bitmap *bm, int face, int tex, int ml, float *u, float *v);
	void get_face_extent(int face, int *u0, int *v0, int *u1, int *v1);
	void get_raw_tmap(q1::bitmap *bm, int tex, int ml);

	int point_plane_test(vec3 &loc, q1::dplane_t *plane);
	void bsp_render_world(Graphics &gfx, vec3 &cam_loc, q1::dplane_t *pl);
	void bsp_visit_visible_leaves(vec3 &cam_loc, q1::dplane_t *pl);
	int aabb_in_frustrum(short *mins, short *maxs, q1::dplane_t *planes);

	bool loaded;

private:
	void palette_to_rgb(q1::bitmap &bm);

	bool initialized;
	bool map_selected;
	q1::q1bsp_t data;
	std::vector<unsigned int> index_array;
	int map_index_vbo;
	int map_vertex_vbo;
	int *face_start_index;
	int *face_count;
	int *face_to_tex;
	vertex_t *map_vertex;

	char *vis_node;
	char *vis_face;
	char vis_leaf[MAX_MAP_LEAFS / 8 + 1];
	q1::dplane_t *planes;
};
