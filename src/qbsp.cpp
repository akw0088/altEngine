#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "qbsp.h"

char *get_file(char *filename, int *size);

float main_matrix[3][3], view_matrix[3][3], translate[3];
point_3d *clip_list1[40], *clip_list2[40];
point_3d pts[32], *default_vlist[32];
static fix clip_x_low, clip_x_high, clip_y_low, clip_y_high;

#define VERTEX(x) ((dvertex_t *) ((char *) dvertexes + (x)*4 + (x)*8))


double chop_temp;
#define FLOAT_TO_INT(x)  ((chop_temp = (x) + BIG_NUM), *(int*)(&chop_temp))
#define FLOAT_TO_FIX(x)  \
             ((chop_temp = (x) + BIG_NUM/65536.0), *(int*)(&chop_temp))
#define BIG_NUM     ((float) (1 << 26) * (1 << 26) * 1.5)
#define fix_cint(x)       (((x)+65535) >> 16)


short BigShort(short l)
{
	byte    b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

short LittleShort(short l)
{
	return l;
}


int BigLong(int l)
{
	byte    b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

int LittleLong(int l)
{
	return l;
}

float BigFloat(float l)
{
	union { byte b[4]; float f; } in, out;

	in.f = l;
	out.b[0] = in.b[3];
	out.b[1] = in.b[2];
	out.b[2] = in.b[1];
	out.b[3] = in.b[0];

	return out.f;
}

float LittleFloat(float l)
{
	return l;
}

int QBsp::load(char *filename)
{
	int size = 0;
	char *data = get_file(filename, &size);
	qbsp_t *qbsp = (qbsp_t *)data;

	if (qbsp == NULL)
	{
		printf("Unable to open %s\n", filename);
		return -1;
	}

	dentdata		= (char *)			(data + qbsp->entity.offset);
	dnodes			= (dnode_t *)		(data + qbsp->node.offset);
	texinfo			= (texinfo_t *)		(data + qbsp->texinfo.offset);
	dfaces			= (dface_t *)		(data + qbsp->face.offset);
	dclipnodes		= (dclipnode_t *)	(data + qbsp->clipnode.offset);
	dedges			= (dedge_t *)		(data + qbsp->edge.offset);
	dmarksurfaces	= (unsigned short *)(data + qbsp->marksurf.offset);
	dsurfedges		= (int *)			(data + qbsp->surface_edge.offset);
	dplanes			= (dplane_t *)		(data + qbsp->plane.offset);
	dmodels			= (dmodel_t *)		(data + qbsp->model.offset);
	dtexdata		= (byte *)			(data + qbsp->tex.offset);
	dvertexes		= (dvertex_t *)		(data + qbsp->vert.offset);
	dvisdata		= (byte *)			(data + qbsp->vis.offset);
	dlightdata		= (byte *)			(data + qbsp->lightmap.offset);

	numnodes		= qbsp->node.size / sizeof(dnode_t);
	numtexinfo		= qbsp->texinfo.size / sizeof(texinfo_t);
	numfaces		= qbsp->face.size / sizeof(dface_t);
	numclipnodes	= qbsp->clipnode.size / sizeof(dclipnode_t);
	numedges		= qbsp->edge.size / sizeof(dedge_t);
	nummarksurfaces = qbsp->marksurf.size / sizeof(unsigned short);
	numsurfedges	= qbsp->surface_edge.size / sizeof(int);
	numplanes		= qbsp->plane.size / sizeof(dplane_t);
	nummodels		= qbsp->model.size / sizeof(dmodel_t);
	numvertexes		= qbsp->vert.size / sizeof(dvertex_t);
	numleaf			= qbsp->leaf.size / sizeof(dleaf_t);

	vis_node = (char *)dvisdata;
	vis_face = (char *)dfaces;

	SwapBSPFile(false);
	return 0;
}

void QBsp::bsp_render_world(vector *cam_loc, dplane_t *pl)
{
	planes = pl;
	loc = cam_loc;
	bsp_render_node((int)dmodels[0].headnode[0]);
}

void QBsp::bsp_render_node(int node)
{
	if (node >= 0 && vis_node[node])
	{
		if (point_plane_test(loc, &dplanes[dnodes[node].planenum]))
		{
			bsp_render_node(dnodes[node].children[0]);
			render_node_faces(node, 1);
			bsp_render_node(dnodes[node].children[1]);
		}
		else
		{
			bsp_render_node(dnodes[node].children[1]);
			render_node_faces(node, 0);
			bsp_render_node(dnodes[node].children[0]);
		}
	}
}

int QBsp::point_plane_test(vector *loc, dplane_t *plane)
{
	return plane->normal[0] * loc->x + plane->normal[1] * loc->y
		+ plane->normal[2] * loc->z < plane->dist;
}

void QBsp::render_node_faces(int node, int side)
{
	int i, n, f;
	n = dnodes[node].numfaces;
	f = dnodes[node].firstface;
	for (i = 0; i < n; ++i)
	{
		if (is_marked(f))
		{
			if (dfaces[f].side == side)
				draw_face(f);
			unmark_face(f);
		}
		++f;
	}
}

float dot_vec_dbl(float *a, vector *b)
{
	return a[0] * b->x + a[1] * b->y + a[2] * b->z;
}

void transform_point_raw(vector *out, vector *in)
{
	vector temp;
	temp.x = in->x - translate[0];
	temp.y = in->y - translate[1];
	temp.z = in->z - translate[2];

	out->x = dot_vec_dbl(view_matrix[0], &temp);
	out->z = dot_vec_dbl(view_matrix[1], &temp);
	out->y = dot_vec_dbl(view_matrix[2], &temp);
}

float proj_scale_x = 160, proj_scale_y = 160 * 200 / 240;
float xcenter = 159.5, ycenter = 99.5;
float near_clip = 0.01, near_code = 16.0;


void project_point(point_3d *p)
{
	if (p->p.z >= near_clip) {
		double div = 1.0 / p->p.z;
		p->sx = FLOAT_TO_FIX(p->p.x * div + xcenter);
		p->sy = FLOAT_TO_FIX(-p->p.y * div + ycenter);
	}
}

#define CC_OFF_LEFT 1
#define CC_OFF_RIGHT 2
#define CC_OFF_TOP 4
#define CC_OFF_BOT 8
#define CC_BEHIND 16

double clip_scale_x, clip_scale_y;

void code_point(point_3d *p)
{
	if (p->p.z >= near_code) {
		// if point is far enough away, code in 2d from fixedpoint (faster)
		if (p->sx < clip_x_low)  p->ccodes = CC_OFF_LEFT;
		else if (p->sx > clip_x_high) p->ccodes = CC_OFF_RIGHT;
		else                          p->ccodes = 0;
		if (p->sy < clip_y_low)  p->ccodes |= CC_OFF_TOP;
		else if (p->sy > clip_y_high) p->ccodes |= CC_OFF_BOT;
	}
	else {
		p->ccodes = (p->p.z > 0) ? 0 : CC_BEHIND;
		if (p->p.x * clip_scale_x < -p->p.z) p->ccodes |= CC_OFF_LEFT;
		if (p->p.x * clip_scale_x >  p->p.z) p->ccodes |= CC_OFF_RIGHT;
		if (p->p.y * clip_scale_y >  p->p.z) p->ccodes |= CC_OFF_TOP;
		if (p->p.y * clip_scale_y < -p->p.z) p->ccodes |= CC_OFF_BOT;
	}
}

void transform_point(point_3d *p, vector *v)
{
	transform_point_raw(&p->p, v);
	project_point(p);
	code_point(p);
}

double dist2_from_viewer(vector *in, vector &cam_loc)
{
	vector temp;
	temp.x = in->x - cam_loc.x;
	temp.y = in->y - cam_loc.y;
	temp.z = in->z - cam_loc.z;

	return temp.x*temp.x + temp.y*temp.y + temp.z*temp.z;
}

#define DIST  256.0    // dist to switch first mip level
#define DIST2 (DIST*DIST)

int QBsp::compute_mip_level(int face)
{
	// dumb algorithm: grab 3d coordinate of some vertex,
	// compute dist from viewer
	int se = dfaces[face].firstedge;
	int e = dsurfedges[se];
	double dist;
	if (e < 0) e = -e;
	dist = dist2_from_viewer((vector *)&dvertexes[dedges[e].v[0]], *loc) / DIST2;
	if (dist < 1) return 0;
	if (dist < 4) return 1;
	if (dist < 16) return 2;
	return 3;
}



// iterate over vertices of face, compute u&v coords, compute min & max
void QBsp::get_face_extent(int face, int *u0, int *v0, int *u1, int *v1)
{
	float uv[32][2], u[4], v[4], umin, umax, vmin, vmax;
	int tex = dfaces[face].texinfo;
	int i, n = dfaces[face].numedges;
	int se = dfaces[face].firstedge;
	vector *loc;

	memcpy(u, texinfo[tex].vecs[0], sizeof(u));
	memcpy(v, texinfo[tex].vecs[1], sizeof(v));

	for (i = 0; i < n; ++i) {
		int j = dsurfedges[se + i];
		if (j < 0)
			loc = (vector *)VERTEX(dedges[-j].v[1]);
		else
			loc = (vector *)VERTEX(dedges[j].v[0]);
		uv[i][0] = loc->x*u[0] + loc->y*u[1] + loc->z*u[2] + u[3];
		uv[i][1] = loc->x*v[0] + loc->y*v[1] + loc->z*v[2] + v[3];
	}
	umin = umax = uv[0][0];
	vmin = vmax = uv[0][1];
	for (i = 1; i < n; ++i) {
		if (uv[i][0] < umin) umin = uv[i][0];
		else if (uv[i][0] > umax) umax = uv[i][0];
		if (uv[i][1] < vmin) vmin = uv[i][1];
		else if (uv[i][1] > vmax) vmax = uv[i][1];
	}
	*u0 = FLOAT_TO_INT(umin) & ~15;
	*v0 = FLOAT_TO_INT(vmin) & ~15;
	*u1 = FLOAT_TO_INT(ceil(umax / 16)) << 4;
	*v1 = FLOAT_TO_INT(ceil(vmax / 16)) << 4;
}

typedef struct
{
	int face;
	int mip_level;
	int valid;
	bitmap *bm;
	float u, v;
} surface_t;

#define MAX_CACHED_SURFACES  (1024)
surface_t surface[MAX_CACHED_SURFACES];  // circular queue
int surface_head, surface_tail;          // index into surfaces
int cur_cache;                           // current storage in use


int is_cached = 0;
#define   MAX_MAP_FACES      65535
short surface_cache[MAX_MAP_FACES];      // cache entry for each face

void QBsp::get_raw_tmap(bitmap *bm, int tex, int ml)
{
	dmiptexlump_t *mtl = (dmiptexlump_t *)dtexdata;
	miptex_t *mip;
	mip = (miptex_t *)(dtexdata + mtl->dataofs[tex]);
	bm->bits = (char *)mip + mip->offsets[ml];
	bm->wid = mip->width >> ml;
	bm->ht = mip->height >> ml;
}

#define ADJ_SURFACE(x)    ((x) & (MAX_CACHED_SURFACES-1))

void free_surface(int surf)
{
	if (surface[surf].valid)
		surface_cache[surface[surf].face] = -1;

	cur_cache -= surface[surf].bm->wid * surface[surf].bm->ht + sizeof(bitmap);
	free(surface[surf].bm);
}

void free_next_surface(void)
{
	if (surface_head == surface_tail) return;
	free_surface(surface_tail);
	surface_tail = ADJ_SURFACE(surface_tail + 1);
}

#define MAX_CACHE            (1024*1024)

int allocate_cached_surface(int size)
{
	int surf;

	// make sure there's a free surface entry
	if (ADJ_SURFACE(surface_head + 1) == surface_tail)
		free_next_surface();

	surf = surface_head;
	surface_head = ADJ_SURFACE(surface_head + 1);

	size += sizeof(bitmap);

	// make sure there's enough storage
	while (cur_cache + size > MAX_CACHE)
		free_next_surface();

	surface[surf].bm = (bitmap *)malloc(size);
	cur_cache += size;

	return surf;
}

int shift, global_step, global_row, lightmap_width;
unsigned char *light_index;
unsigned char blank_light[512];

// compute one lightmap square of surface
void build_block(char *out, bitmap *raw, int x, int y)
{
	char colormap[512][256]; // akwright: set me to something
	fix c, dc;
	int a, b, h, c0, c1, c2, c3, step = global_step, row = global_row - step;
	int y_max = raw->ht, x_max = raw->wid;
	char *s = raw->bits + y*raw->wid;

	c0 = (255 - light_index[0]) << 16;
	c1 = (255 - light_index[1]) << 16;
	c2 = (255 - light_index[lightmap_width]) << 16;
	c3 = (255 - light_index[lightmap_width + 1]) << 16;

	c2 = (c2 - c0) >> shift;
	c3 = (c3 - c1) >> shift;

	for (b = 0; b < step; ++b)
	{
		h = x;
		c = c0;
		dc = (c1 - c0) >> shift;
		for (a = 0; a < step; ++a)
		{
			*out++ = colormap[c >> 18][s[h]];
			c += dc;
			if (++h == x_max)
				h = 0;
		}
		out += row;
		c0 += c2;
		c1 += c3;

		if (++y == y_max)
		{
			y = 0;
			s = raw->bits;
		}
		else
			s += raw->wid;
	}
}

void QBsp::get_tmap(bitmap *bm, int face, int tex, int ml, float *u, float *v)
{
	int i, j, surf, u0, v0, u1, v1, step, x, y, x0;
	bitmap raw;

	is_cached = 1;

	surf = surface_cache[face];
	if (surf >= 0)
	{
		if (surface[surf].mip_level == ml)
		{
			*bm = *surface[surf].bm;
			*u = surface[surf].u;
			*v = surface[surf].v;
			return;
		}
		surface[surf].valid = 0;
		surface_cache[face] = -1;
	}

	get_face_extent(face, &u0, &v0, &u1, &v1);

	bm->wid = (u1 - u0) >> ml;
	bm->ht = (v1 - v0) >> ml;

	if (bm->wid > 256 || bm->ht > 256)
	{
		is_cached = 0;
		get_raw_tmap(bm, tex, ml);
		return;
	}
	get_raw_tmap(&raw, tex, ml);

	surf = surface_cache[face] = allocate_cached_surface(bm->wid * bm->ht);
	bm->bits = (char *)&surface[surf].bm[1];
	*surface[surf].bm = *bm;

	surface[surf].face = face;
	surface[surf].mip_level = ml;
	surface[surf].valid = 1;
	*u = surface[surf].u = (float)u0;
	*v = surface[surf].v = (float)v0;

	step = 16 >> ml;   // width of lightmap after mipmapping
	shift = 4 - ml;      // 1 << shift  == step

	lightmap_width = ((u1 - u0) >> 4) + 1;

	// compute texture offsets
	u0 >>= ml;
	v0 >>= ml;

	global_step = step;
	global_row = bm->wid;
	if (dfaces[face].lightofs == -1)
		light_index = blank_light;
	else
		light_index = &dlightdata[dfaces[face].lightofs];

	y = v0%raw.ht;  if (y < 0)  y += raw.ht;   // fixup for signed mod
	x0 = u0%raw.wid; if (x0 < 0) x0 += raw.wid;  // fixup for signed mod

	for (j = 0; j < bm->ht; j += step)
	{
		x = x0;
		for (i = 0; i < bm->wid; i += step, ++light_index)
		{
			build_block(bm->bits + j*bm->wid + i, &raw, x, y);
			x += step; if (x >= raw.wid) x -= raw.wid;
		}
		++light_index;
		y += step; if (y >= raw.ht) y -= raw.ht;
	}
}

char  *qmap_buf, *qmap_tex;
int    qmap_buf_row, qmap_tex_row;
int    qmap_wid, qmap_ht;
int    qmap_row_table[768];      // max height of screen
int    qmap_tex_row_table[258];  // max height of texture + 2

void qmap_set_texture(bitmap *bm)
{
	int row = bm->wid, ht = bm->ht;
	qmap_tex = bm->bits;
	qmap_wid = bm->wid - 1;

	if (qmap_tex_row != row || qmap_ht != ht - 1)
	{
		int i;
		qmap_tex_row = row;
		qmap_ht = ht - 1;
		for (i = 0; i < ht; ++i)
			qmap_tex_row_table[i + 1] = row * i;
		qmap_tex_row_table[0] = qmap_tex_row_table[1];
		qmap_tex_row_table[ht + 1] = qmap_tex_row_table[ht];
	}
}

void compute_texture_normal(vector *out, vector *u, vector *v)
{
	out->x = u->y*v->z - u->z*v->y;
	out->y = u->z*v->x - u->x*v->z;
	out->z = u->x*v->y - u->y*v->x;
}


void transform_vector(vector *out, vector *in)
{
	vector temp = *in;
	out->x = dot_vec_dbl(view_matrix[0], &temp);
	out->z = dot_vec_dbl(view_matrix[1], &temp);
	out->y = dot_vec_dbl(view_matrix[2], &temp);
}


void setup_uv_vector(vector *out, vector *in, vector *norm, float *plane)
{
	float dot = -(in->x*plane[0] + in->y*plane[1] + in->z*plane[2])
		/ (norm->x*plane[0] + norm->y*plane[1] + norm->z*plane[2]);
	if (dot != 0)
	{
		vector temp = *in;
		temp.x += norm->x*dot;
		temp.y += norm->y*dot;
		temp.z += norm->z*dot;
		transform_vector(out, &temp);
	}
	else
		transform_vector(out, in);
}

// compute location of origin of texture (should precompute)
void setup_origin_vector(vector *out, dplane_t *plane, vector *norm)
{
	vector temp;
	float d = plane->dist / (norm->x*plane->normal[0]
		+ norm->y*plane->normal[1] + norm->z*plane->normal[2]);
	temp.x = d * norm->x;
	temp.y = d * norm->y;
	temp.z = d * norm->z;

	transform_point_raw(out, &temp);
}

double qmap_tmap[9];

void qmap_set_texture_gradients(double *tmap_data)
{
	int i;
	for (i = 0; i < 9; ++i)
		qmap_tmap[i] = tmap_data[i];
}

void QBsp::compute_texture_gradients(int face, int tex, int mip, float u, float v)
{
	double tmap_data[9];
	vector P, M, N;
	vector norm;
	dplane_t *plane = &dplanes[dfaces[face].planenum];
	float rescale = (8 >> mip) / 8.0f;

	compute_texture_normal(&norm,
		(vector *)texinfo[tex].vecs[0], (vector *)texinfo[tex].vecs[1]);
	// project vectors onto face's plane, and transform
	setup_uv_vector(&M, (vector *)texinfo[tex].vecs[0], &norm, plane->normal);
	setup_uv_vector(&N, (vector *)texinfo[tex].vecs[1], &norm, plane->normal);
	setup_origin_vector(&P, plane, &norm);

	u -= texinfo[tex].vecs[0][3];  // adjust according to face's info
	v -= texinfo[tex].vecs[1][3];  // passed in values adjust for lightmap

								   // we could just subtract (u,v) every time we compute a new (u,v);
								   // instead we fold it into P:
	P.x += u * M.x + v * N.x;
	P.y += u * M.y + v * N.y;
	P.z += u * M.z + v * N.z;

	tmap_data[0] = P.x*N.y - P.y*N.x;
	tmap_data[1] = P.y*N.z - P.z*N.y;
	tmap_data[2] = P.x*N.z - P.z*N.x;
	tmap_data[3] = P.y*M.x - P.x*M.y;
	tmap_data[4] = P.z*M.y - P.y*M.z;
	tmap_data[5] = P.z*M.x - P.x*M.z;
	tmap_data[6] = N.x*M.y - N.y*M.x;
	tmap_data[7] = N.y*M.z - N.z*M.y;
	tmap_data[8] = N.x*M.z - N.z*M.x;

	// offset by center of screen--if this were folded into
	// transform translation we could avoid it
	tmap_data[0] -= tmap_data[1] * 159.5 + tmap_data[2] * 99.5;
	tmap_data[3] -= tmap_data[4] * 159.5 + tmap_data[5] * 99.5;
	tmap_data[6] -= tmap_data[7] * 159.5 + tmap_data[8] * 99.5;

	tmap_data[0] *= rescale;
	tmap_data[1] *= rescale;
	tmap_data[2] *= rescale;
	tmap_data[3] *= rescale;
	tmap_data[4] *= rescale;
	tmap_data[5] *= rescale;

	qmap_set_texture_gradients(tmap_data);
}


void draw_poly(int n, point_3d **vl)
{
	int i, j, y, ey;
	fix ymin, ymax;

	// find max and min y height
	ymin = ymax = vl[0]->sy;
	for (i = 1; i < n; ++i) {
		if (vl[i]->sy < ymin) ymin = vl[i]->sy;
		else if (vl[i]->sy > ymax) ymax = vl[i]->sy;
	}

	// scan out each edge
	j = n - 1;
	for (i = 0; i < n; ++i)
	{
		//scan_convert(vl[i], vl[j]);
		j = i;
	}

	y = fix_cint(ymin);
	ey = fix_cint(ymax);

	// iterate over all spans and draw

	while (y < ey)
	{
		/*
		int sx = fix_cint(scan[y][0]), ex = fix_cint(scan[y][1]);
		if (sx < ex)
			qmap_draw_span(y, sx, ex);
		++y;
		*/
	}
}

void transform_rotated_point(point_3d *p)
{
	project_point(p);
	code_point(p);
}


static void intersect(point_3d *out, point_3d *a, point_3d *b, float where)
{
	// intersection occurs 'where' % along the line from a to b

	out->p.x = a->p.x + (b->p.x - a->p.x) * where;
	out->p.y = a->p.y + (b->p.y - a->p.y) * where;
	out->p.z = a->p.z + (b->p.z - a->p.z) * where;

	transform_rotated_point(out);
}

static double left_loc(point_3d *a, point_3d *b)
{
	return -(a->p.z + a->p.x*clip_scale_x) / ((b->p.x - a->p.x)*clip_scale_x + b->p.z - a->p.z);
}

static double right_loc(point_3d *a, point_3d *b)
{
	return  (a->p.z - a->p.x*clip_scale_x) / ((b->p.x - a->p.x)*clip_scale_x - b->p.z + a->p.z);
}

static double top_loc(point_3d *a, point_3d *b)
{
	return  (a->p.z - a->p.y*clip_scale_y) / ((b->p.y - a->p.y)*clip_scale_y - b->p.z + a->p.z);
}

static double bottom_loc(point_3d *a, point_3d *b)
{
	return -(a->p.z + a->p.y*clip_scale_y) / ((b->p.y - a->p.y)*clip_scale_y + b->p.z - a->p.z);
}

// clip the polygon to each of the view frustrum planes
int clip_poly(int n, point_3d **vl, int codes_or, point_3d ***out_vl)
{
	int i, j, k, p = 0; // p = index into temporary point pool
	point_3d **cur;

	if (codes_or & CC_OFF_LEFT) {
		cur = clip_list1;
		k = 0;
		j = n - 1;
		for (i = 0; i < n; ++i) {
			// process edge from j..i
			// if j is inside, add it

			if (!(vl[j]->ccodes & CC_OFF_LEFT))
				cur[k++] = vl[j];

			// if it crosses, add the intersection point

			if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_LEFT) {
				intersect(&pts[p], vl[i], vl[j], left_loc(vl[i], vl[j]));
				cur[k++] = &pts[p++];
			}
			j = i;
		}
		// move output list to be input
		n = k;
		vl = cur;
	}
	if (codes_or & CC_OFF_RIGHT) {
		cur = (vl == clip_list1) ? clip_list2 : clip_list1;
		k = 0;
		j = n - 1;
		for (i = 0; i < n; ++i) {
			if (!(vl[j]->ccodes & CC_OFF_RIGHT))
				cur[k++] = vl[j];
			if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_RIGHT) {
				intersect(&pts[p], vl[i], vl[j], right_loc(vl[i], vl[j]));
				cur[k++] = &pts[p++];
			}
			j = i;
		}
		n = k;
		vl = cur;
	}
	if (codes_or & CC_OFF_TOP) {
		cur = (vl == clip_list1) ? clip_list2 : clip_list1;
		k = 0;
		j = n - 1;
		for (i = 0; i < n; ++i) {
			if (!(vl[j]->ccodes & CC_OFF_TOP))
				cur[k++] = vl[j];
			if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_TOP) {
				intersect(&pts[p], vl[i], vl[j], top_loc(vl[i], vl[j]));
				cur[k++] = &pts[p++];
			}
			j = i;
		}
		n = k;
		vl = cur;
	}
	if (codes_or & CC_OFF_BOT) {
		cur = (vl == clip_list1) ? clip_list2 : clip_list1;
		k = 0;
		j = n - 1;
		for (i = 0; i < n; ++i) {
			if (!(vl[j]->ccodes & CC_OFF_BOT))
				cur[k++] = vl[j];
			if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_BOT) {
				intersect(&pts[p], vl[i], vl[j], bottom_loc(vl[i], vl[j]));
				cur[k++] = &pts[p++];
			}
			j = i;
		}
		n = k;
		vl = cur;
	}
	for (i = 0; i < n; ++i)
		if (vl[i]->ccodes & CC_BEHIND)
			return 0;

	*out_vl = vl;
	return n;
}


void QBsp::draw_face(int face)
{
	int n = dfaces[face].numedges;
	int se = dfaces[face].firstedge;
	int i, codes_or = 0, codes_and = 0xff;
	point_3d **vlist;

	for (i = 0; i < n; ++i)
	{
		int edge = dsurfedges[se + i];
		if (edge < 0)
			transform_point(&pts[i], (vector *)VERTEX(dedges[-edge].v[1]));
		else
			transform_point(&pts[i], (vector *)VERTEX(dedges[edge].v[0]));
		codes_or |= pts[i].ccodes;
		codes_and &= pts[i].ccodes;
	}

	if (codes_and)
		return;  // abort if poly outside frustrum

	if (codes_or)
	{
		// poly crosses frustrum, so clip it
		n = clip_poly(n, default_vlist, codes_or, &vlist);
		vlist = default_vlist;
	}
	else
		vlist = default_vlist;

	if (n)
	{
		bitmap bm;
		float u, v;
		int tex = dfaces[face].texinfo;
		int mip = compute_mip_level(face);
		get_tmap(&bm, face, texinfo[tex].miptex, mip, &u, &v);
		qmap_set_texture(&bm);
		compute_texture_gradients(face, tex, mip, u, v);
		draw_poly(n, vlist);
	}
}


void QBsp::SwapBSPFile(bool todisk)
{
	int            i, j, c;
	dmodel_t      *d;
	dmiptexlump_t   *mtl;


	// models   
	for (i = 0; i<nummodels; i++)
	{
		d = &dmodels[i];

		for (j = 0; j < MAX_MAP_HULLS; j++)
			d->headnode[j] = LittleLong(d->headnode[j]);

		d->visleafs = LittleLong(d->visleafs);
		d->firstface = LittleLong(d->firstface);
		d->numfaces = LittleLong(d->numfaces);

		for (j = 0; j < 3; j++)
		{
			d->mins[j] = LittleFloat(d->mins[j]);
			d->maxs[j] = LittleFloat(d->maxs[j]);
			d->origin[j] = LittleFloat(d->origin[j]);
		}
	}

	//
	// vertexes
	//
	for (i = 0; i < numvertexes; i++)
	{
		for (j = 0; j < 3; j++)
			dvertexes[i].point[j] = LittleFloat(dvertexes[i].point[j]);
	}

	//
	// planes
	//   
	for (i = 0; i < numplanes; i++)
	{
		for (j = 0; j < 3; j++)
			dplanes[i].normal[j] = LittleFloat(dplanes[i].normal[j]);
		dplanes[i].dist = LittleFloat(dplanes[i].dist);
		dplanes[i].type = LittleLong(dplanes[i].type);
	}

	//
	// texinfos
	//   
	for (i = 0; i<numtexinfo; i++)
	{
		for (j = 0; j < 8; j++)
			texinfo[i].vecs[0][j] = LittleFloat(texinfo[i].vecs[0][j]);
		texinfo[i].miptex = LittleLong(texinfo[i].miptex);
		texinfo[i].flags = LittleLong(texinfo[i].flags);
	}

	//
	// faces
	//
	for (i = 0; i < numfaces; i++)
	{
		dfaces[i].texinfo = LittleShort(dfaces[i].texinfo);
		dfaces[i].planenum = LittleShort(dfaces[i].planenum);
		dfaces[i].side = LittleShort(dfaces[i].side);
		dfaces[i].lightofs = LittleLong(dfaces[i].lightofs);
		dfaces[i].firstedge = LittleLong(dfaces[i].firstedge);
		dfaces[i].numedges = LittleShort(dfaces[i].numedges);
	}

	//
	// nodes
	//
	for (i = 0; i < numnodes; i++)
	{
		dnodes[i].planenum = LittleLong(dnodes[i].planenum);
		for (j = 0; j < 3; j++)
		{
			dnodes[i].mins[j] = LittleShort(dnodes[i].mins[j]);
			dnodes[i].maxs[j] = LittleShort(dnodes[i].maxs[j]);
		}
		dnodes[i].children[0] = LittleShort(dnodes[i].children[0]);
		dnodes[i].children[1] = LittleShort(dnodes[i].children[1]);
		dnodes[i].firstface = LittleShort(dnodes[i].firstface);
		dnodes[i].numfaces = LittleShort(dnodes[i].numfaces);
	}

	//
	// leafs
	//
	for (i = 0; i < numleafs; i++)
	{
		dleafs[i].contents = LittleLong(dleafs[i].contents);
		for (j = 0; j < 3; j++)
		{
			dleafs[i].mins[j] = LittleShort(dleafs[i].mins[j]);
			dleafs[i].maxs[j] = LittleShort(dleafs[i].maxs[j]);
		}

		dleafs[i].firstmarksurface = LittleShort(dleafs[i].firstmarksurface);
		dleafs[i].nummarksurfaces = LittleShort(dleafs[i].nummarksurfaces);
		dleafs[i].visofs = LittleLong(dleafs[i].visofs);
	}

	//
	// clipnodes
	//
	for (i = 0; i < numclipnodes; i++)
	{
		dclipnodes[i].planenum = LittleLong(dclipnodes[i].planenum);
		dclipnodes[i].children[0] = LittleShort(dclipnodes[i].children[0]);
		dclipnodes[i].children[1] = LittleShort(dclipnodes[i].children[1]);
	}

	//
	// miptex
	//
	if (texdatasize)
	{
		mtl = (dmiptexlump_t *)dtexdata;
		if (todisk)
			c = mtl->nummiptex;
		else
			c = LittleLong(mtl->nummiptex);
		mtl->nummiptex = LittleLong(mtl->nummiptex);
		for (i = 0; i < c; i++)
			mtl->dataofs[i] = LittleLong(mtl->dataofs[i]);
	}

	//
	// marksurfaces
	//
	for (i = 0; i < nummarksurfaces; i++)
		dmarksurfaces[i] = LittleShort(dmarksurfaces[i]);

	//
	// surfedges
	//
	for (i = 0; i < numsurfedges; i++)
		dsurfedges[i] = LittleLong(dsurfedges[i]);

	//
	// edges
	//
	for (i = 0; i < numedges; i++)
	{
		dedges[i].v[0] = LittleShort(dedges[i].v[0]);
		dedges[i].v[1] = LittleShort(dedges[i].v[1]);
	}
}



void rotate_c2w(vector *dest, vector *src)
{
	dest->x = src->x * main_matrix[0][0]
		+ src->y * main_matrix[1][0]
		+ src->z * main_matrix[2][0];
	dest->y = src->x * main_matrix[0][1]
		+ src->y * main_matrix[1][1]
		+ src->z * main_matrix[2][1];
	dest->z = src->x * main_matrix[0][2]
		+ src->y * main_matrix[1][2]
		+ src->z * main_matrix[2][2];
}

void compute_plane(dplane_t *plane, float x, float y, float z, vector &cam_loc)
{
	vector temp, temp2;
	temp2.x = x; temp2.y = z; temp2.z = y;
	rotate_c2w(&temp, &temp2);
	plane->normal[0] = temp.x;
	plane->normal[1] = temp.y;
	plane->normal[2] = temp.z;
	plane->dist = temp.x*cam_loc.x + temp.y*cam_loc.y + temp.z*cam_loc.z;
}

void compute_view_frustrum(dplane_t *planes, vector &cam_loc)
{
	compute_plane(planes + 0, -1, 0, 1, cam_loc);
	compute_plane(planes + 1, 1, 0, 1, cam_loc);
	compute_plane(planes + 2, 0, 1, 1, cam_loc);
	compute_plane(planes + 3, 0, -1, 1, cam_loc);
}

int QBsp::find_leaf(vector *loc)
{
	int n = dmodels[0].headnode[0];
	while (n >= 0)
	{
		dnode_t *node = &dnodes[n];
		n = node->children[point_plane_test(loc, &dplanes[node->planenum])];
	}
	return ~n;
}


int QBsp::visit_visible_leaves(vector *cam_loc)
{
	int n, v, i;

	memset(vis_leaf, 0, sizeof(vis_leaf));

	n = find_leaf(cam_loc);

	if (n == 0 || dleafs[n].visofs < 0)
		return 0;

	v = dleafs[n].visofs;
	for (i = 1; i < numleafs; )
	{
		if (dvisdata[v] == 0)
		{
			i += 8 * dvisdata[v + 1];    // skip some leaves
			v += 2;
		}
		else
		{
			int j;
			for (j = 0; j <8; j++, i++)
				if (dvisdata[v] & (1 << j))
					vis_leaf[i >> 3] |= (1 << (i & 7));
			++v;
		}
	}
	return 1;
}

int QBsp::bsp_find_visible_nodes(int node)
{
	if (node >= 0)
	{
		vis_node[node] = !!(bsp_find_visible_nodes(dnodes[node].children[0])
			| bsp_find_visible_nodes(dnodes[node].children[1]));
		return vis_node[node];
	}
	else
	{
		node = ~node;
		return (vis_leaf[node >> 3] & (1 << (node & 7)));
	}
}

#define FLOAT_POSITIVE(x)   (* (int *) (x) >= 0)

int bbox_inside_plane(short *mins, short *maxs, dplane_t *plane)
{
	int i;
	short pt[3];

	// use quick test from graphics gems

	for (i = 0; i < 3; ++i)
		if (FLOAT_POSITIVE(&plane->normal[i])) // fast test assuming IEEE
			pt[i] = maxs[i];
		else
			pt[i] = mins[i];

	return plane->normal[0] * pt[0] + plane->normal[1] * pt[1]
		+ plane->normal[2] * pt[2] >= plane->dist;
}


int QBsp::leaf_in_frustrum(dleaf_t *node, dplane_t *planes)
{
	if (!bbox_inside_plane(node->mins, node->maxs, planes + 0)
		|| !bbox_inside_plane(node->mins, node->maxs, planes + 1)
		|| !bbox_inside_plane(node->mins, node->maxs, planes + 2)
		|| !bbox_inside_plane(node->mins, node->maxs, planes + 3))
		return 0;
	return 1;
}

void QBsp::mark_leaf_faces(int leaf)
{
	int n = dleafs[leaf].nummarksurfaces;
	int ms = dleafs[leaf].firstmarksurface;
	int i;

	for (i = 0; i < n; ++i)
	{
		int s = dmarksurfaces[ms + i];
		if (!is_marked(s))
		{
			mark_face(s);
		}
	}
}

int node_in_frustrum(dnode_t *node, dplane_t *planes)
{
	if (!bbox_inside_plane(node->mins, node->maxs, planes + 0)
		|| !bbox_inside_plane(node->mins, node->maxs, planes + 1)
		|| !bbox_inside_plane(node->mins, node->maxs, planes + 2)
		|| !bbox_inside_plane(node->mins, node->maxs, planes + 3))
		return 0;
	return 1;
}

void QBsp::bsp_explore_node(int node)
{
	if (node < 0)
	{
		node = ~node;
		if (vis_leaf[node >> 3] & (1 << (node & 7)))
			if (leaf_in_frustrum(&dleafs[node], planes))
				mark_leaf_faces(node);
		return;
	}

	if (vis_node[node])
	{
		if (!node_in_frustrum(&dnodes[node], planes))
			vis_node[node] = 0;
		else
		{
			bsp_explore_node(dnodes[node].children[0]);
			bsp_explore_node(dnodes[node].children[1]);
		}
	}
}

void QBsp::bsp_visit_visible_leaves(vector *cam_loc, dplane_t *pl)
{
	planes = pl;
	loc = cam_loc;
	bsp_find_visible_nodes((int)dmodels[0].headnode[0]);
	bsp_explore_node((int)dmodels[0].headnode[0]);
}


void QBsp::render()
{
	dplane_t planes[4];
	compute_view_frustrum(planes, cam_loc);

	if (!visit_visible_leaves(loc))
	{
		memset(vis_leaf, 255, sizeof(vis_leaf));
	}

	bsp_visit_visible_leaves(loc, planes);
	bsp_render_world(loc, planes);
}