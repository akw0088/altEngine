#include "include.h"


char *get_file(char *filename, int *size);
int abs32(int val);


// globals
float main_matrix[3][3];
float view_matrix[3][3];

q1vertex_t *clip_list1[40];
q1vertex_t *clip_list2[40];
q1vertex_t pts[32];
q1vertex_t *default_vlist[32];

static fix clip_x_low, clip_x_high, clip_y_low, clip_y_high;

vec3 translate;

double chop_temp;
fix scan[768][2];

int is_cached = 0;
short surface_cache[MAX_MAP_FACES];      // cache entry for each face

float proj_scale_x = 160.0f;
float proj_scale_y = 160 * 200 / 240.0f;
float xcenter = 159.5f;
float ycenter = 99.5f;
float near_clip = 0.01f;
float near_code = 16.0f;

double clip_scale_x;
double clip_scale_y;

surf_t surface[MAX_CACHED_SURFACES];  // circular queue
int surface_head, surface_tail;          // index into surfaces
int cur_cache;                           // current storage in use

int shift, global_step, global_row, lightmap_width;
unsigned char *light_index;
unsigned char blank_light[512];



QBsp::QBsp()
{
	loaded = false;
	initialized = false;
	map_selected = false;
}

int QBsp::load(Graphics &gfx, char *filename)
{
	int size = 0;
	char *file = NULL;
	
	file = get_file(filename, &size);
	if (file == NULL)
	{
		char *pak = "media/PAK0.PAK";
		file = get_pakfile(pak, filename);
	}

	qbsp_t *qbsp = (qbsp_t *)file;

	if (qbsp == NULL)
	{
		printf("Unable to open %s\n", filename);
		return -1;
	}


	data.dentdata = (char *)(file + qbsp->entity.offset);
	data.dnodes = (q1dnode_t *)(file + qbsp->node.offset);
	data.texinfo = (q1texinfo_t *)(file + qbsp->texinfo.offset);
	data.dfaces = (q1dface_t *)(file + qbsp->face.offset);
	data.dclipnodes = (q1dclipnode_t *)(file + qbsp->clipnode.offset);
	data.dedges = (q1dedge_t *)(file + qbsp->edge.offset);
	data.dmarksurfaces = (unsigned short *)(file + qbsp->marksurf.offset);
	data.dsurfedges = (int *)(file + qbsp->surface_edge.offset);
	data.dplanes = (q1dplane_t *)(file + qbsp->plane.offset);
	data.dleafs = (q1dleaf_t *)(file + qbsp->leaf.offset);
	data.dmodels = (q1dmodel_t *)(file + qbsp->model.offset);
	data.dtexdata = (byte *)(file + qbsp->tex.offset);
	data.dvertexes = (vec3 *)(file + qbsp->vert.offset);
	data.dvisdata = (byte *)(file + qbsp->vis.offset);
	data.dlightdata = (byte *)(file + qbsp->lightmap.offset);
	

	data.num_nodes = qbsp->node.size / sizeof(q1dnode_t);
	data.num_texinfo = qbsp->texinfo.size / sizeof(q1texinfo_t);
	data.num_faces = qbsp->face.size / sizeof(q1dface_t);
	data.num_clipnodes = qbsp->clipnode.size / sizeof(q1dclipnode_t);
	data.num_edges = qbsp->edge.size / sizeof(q1dedge_t);
	data.num_marksurfaces = qbsp->marksurf.size / sizeof(unsigned short);
	data.num_surfedges = qbsp->surface_edge.size / sizeof(int);
	data.num_planes = qbsp->plane.size / sizeof(q1dplane_t);
	data.num_leafs = qbsp->leaf.size / sizeof(q1dleaf_t);
	data.num_models = qbsp->model.size / sizeof(q1dmodel_t);
	data.num_vertexes = qbsp->vert.size / sizeof(vec3);
	data.num_leaf = qbsp->leaf.size / sizeof(q1dleaf_t);

	vis_node = (char *)data.dvisdata;
	vis_face = (char *)data.dfaces;

	change_axis();

	map_vertex = new vertex_t[data.num_vertexes];
	for (unsigned int i = 0; i < data.num_vertexes; i++)
	{
		map_vertex[i].position = data.dvertexes[i];
		map_vertex[i].tangent = vec4();
	}

	int *face_start_index = new int[data.num_faces];
	int *face_count = new int[data.num_faces];

	for (int i = 0; i < data.num_faces; i++)
	{
		face_start_index[i] = 0;
		face_count[i] = 0;
	}

	
	for (int i = 0; i < MAX_MAP_FACES; ++i)
		surface_cache[i] = -1;

	for (int i = 0; i < 32; ++i)
		default_vlist[i] = &pts[i];

	memset(&scan, 0, sizeof(scan));

	render(gfx, vec3(0.0f, -1000.0f, 0.0f));
	initialized = true;

	map_vertex_vbo = gfx.CreateVertexBuffer(map_vertex, data.num_vertexes);
	map_index_vbo = gfx.CreateIndexBuffer(index_array.data(), index_array.size());
	loaded = true;

	return 0;
}

void QBsp::bsp_render_world(Graphics &gfx, vec3 &pos, q1dplane_t *pl)
{
	planes = pl;
	bsp_render_node(gfx, (int)data.dmodels[0].headnode[0], pos);
}

void QBsp::bsp_render_node(Graphics &gfx, int node, vec3 &pos)
{
	if (node >= 0 && vis_node[node])
	{
		if (point_plane_test(pos, &data.dplanes[data.dnodes[node].planenum]))
		{
			bsp_render_node(gfx, data.dnodes[node].front, pos);
			render_node_faces(gfx, node, 1);
			bsp_render_node(gfx, data.dnodes[node].back, pos);
		}
		else
		{
			bsp_render_node(gfx, data.dnodes[node].back, pos);
			render_node_faces(gfx, node, 0);
			bsp_render_node(gfx, data.dnodes[node].front, pos);
		}
	}
}

int QBsp::point_plane_test(vec3 &loc, q1dplane_t *plane)
{
	return plane->normal * loc < plane->dist;
}

void QBsp::render_node_faces(Graphics &gfx, int node, int side)
{
	int i, n, f;
	n = data.dnodes[node].numfaces;
	f = data.dnodes[node].firstface;
	for (i = 0; i < n; ++i)
	{
		if (is_marked(f))
		{
			if (data.dfaces[f].side == side)
			{
				if (initialized)
					draw_face(gfx, f);
				else
					build_face(gfx, f);
			}
			unmark_face(f);
		}
		++f;
	}
}

float dot_vec_dbl(float *a, vec3 *b)
{
	return a[0] * b->x + a[1] * b->y + a[2] * b->z;
}

void transform_point_raw(vec3 &out, vec3 &in)
{
	vec3 temp = in - translate;

	out.x = dot_vec_dbl(view_matrix[0], &temp);
	out.z = dot_vec_dbl(view_matrix[1], &temp);
	out.y = dot_vec_dbl(view_matrix[2], &temp);
}

void project_point(q1vertex_t &p)
{
	if (p.p.z >= near_clip)
	{
		double div = 1.0 / p.p.z;
		p.sx = FLOAT_TO_FIX(p.p.x * div + xcenter);
		p.sy = FLOAT_TO_FIX(-p.p.y * div + ycenter);
	}
}

void code_point(q1vertex_t &p)
{
	if (p.p.z >= near_code)
	{
		// if point is far enough away, code in 2d from fixedpoint (faster)
		if (p.sx < clip_x_low)
			p.ccodes = CC_OFF_LEFT;
		else if (p.sx > clip_x_high)
			p.ccodes = CC_OFF_RIGHT;
		else
			p.ccodes = 0;
		if (p.sy < clip_y_low)
			p.ccodes |= CC_OFF_TOP;
		else if (p.sy > clip_y_high)
			p.ccodes |= CC_OFF_BOT;
	}
	else
	{
		p.ccodes = (p.p.z > 0) ? 0 : CC_BEHIND;
		if (p.p.x * clip_scale_x < -p.p.z)
			p.ccodes |= CC_OFF_LEFT;
		if (p.p.x * clip_scale_x >  p.p.z)
			p.ccodes |= CC_OFF_RIGHT;
		if (p.p.y * clip_scale_y >  p.p.z)
			p.ccodes |= CC_OFF_TOP;
		if (p.p.y * clip_scale_y < -p.p.z)
			p.ccodes |= CC_OFF_BOT;
	}
}

void transform_point(q1vertex_t &p, vec3 &v)
{
	transform_point_raw(p.p, v);
	project_point(p);
	code_point(p);
}

double dist2_from_viewer(vec3 *in, vec3 &cam_loc)
{
	vec3 temp;
	temp.x = in->x - cam_loc.x;
	temp.y = in->y - cam_loc.y;
	temp.z = in->z - cam_loc.z;

	return temp.x*temp.x + temp.y*temp.y + temp.z*temp.z;
}


int QBsp::compute_mip_level(int face, vec3 &loc)
{
	// dumb algorithm: grab 3d coordinate of some vertex,
	// compute dist from viewer
	int se = data.dfaces[face].firstedge;
	int e = data.dsurfedges[se];
	double dist;

	if (e < 0)
		e = -e;

	dist = dist2_from_viewer((vec3 *)&data.dvertexes[data.dedges[e].v[0]], loc) / DIST2;

	if (dist < 1)
		return 0;
	if (dist < 4)
		return 1;
	if (dist < 16)
		return 2;
	return 3;
}



// iterate over vertices of face, compute u&v coords, compute min & max
void QBsp::get_face_extent(int face, int *u0, int *v0, int *u1, int *v1)
{
	float uv[32][2], u[4], v[4], umin, umax, vmin, vmax;
	int tex = data.dfaces[face].texinfo;
	int i, n = data.dfaces[face].numedges;
	int se = data.dfaces[face].firstedge;
	vec3 *loc;

	memcpy(u, &data.texinfo[tex].vecs[0], sizeof(u));
	memcpy(v, &data.texinfo[tex].vecs[1], sizeof(v));

	for (i = 0; i < n; ++i)
	{
		int j = data.dsurfedges[se + i];
		if (j < 0)
			loc = (vec3 *)VERTEX(data.dedges[-j].v[1]);
		else
			loc = (vec3 *)VERTEX(data.dedges[j].v[0]);
		uv[i][0] = loc->x*u[0] + loc->y*u[1] + loc->z*u[2] + u[3];
		uv[i][1] = loc->x*v[0] + loc->y*v[1] + loc->z*v[2] + v[3];
	}
	umin = umax = uv[0][0];
	vmin = vmax = uv[0][1];
	for (i = 1; i < n; ++i)
	{
		if (uv[i][0] < umin)
			umin = uv[i][0];
		else if (uv[i][0] > umax)
			umax = uv[i][0];
		if (uv[i][1] < vmin)
			vmin = uv[i][1];
		else if (uv[i][1] > vmax)
			vmax = uv[i][1];
	}
	*u0 = FLOAT_TO_INT(umin) & ~15;
	*v0 = FLOAT_TO_INT(vmin) & ~15;
	*u1 = FLOAT_TO_INT(ceil(umax / 16)) << 4;
	*v1 = FLOAT_TO_INT(ceil(vmax / 16)) << 4;
}

void QBsp::get_raw_tmap(bitmap *bm, int tex, int ml)
{
	q1dmiptexlump_t *mtl = (q1dmiptexlump_t *)data.dtexdata;
	miptex_t *mip;
	mip = (miptex_t *)(data.dtexdata + mtl->dataofs[tex]);
	bm->bits = (char *)mip + mip->offsets[ml];
	bm->wid = mip->width >> ml;
	bm->ht = mip->height >> ml;
}

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
	if (data.dfaces[face].lightofs == -1)
		light_index = blank_light;
	else
		light_index = &data.dlightdata[data.dfaces[face].lightofs];

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

void qmap_set_output(char *where, int row)
{
	qmap_buf = where;
	if (qmap_buf_row != row) {
		int i;
		qmap_buf_row = row;
		for (i = 0; i < 768; ++i)
			qmap_row_table[i] = i * row;
	}
}

void compute_texture_normal(vec3 &out, vec3 &u, vec3 &v)
{
	out.x = u.y * v.z - u.z * v.y;
	out.y = u.z * v.x - u.x * v.z;
	out.z = u.x * v.y - u.y * v.x;
}


void transform_vector(vec3 &out, vec3 &in)
{
	vec3 temp = in;
	out.x = dot_vec_dbl(view_matrix[0], &temp);
	out.z = dot_vec_dbl(view_matrix[1], &temp);
	out.y = dot_vec_dbl(view_matrix[2], &temp);
}


void setup_uv_vector(vec3 &out, vec3 &in, vec3 &norm, vec3 &plane)
{
	float dot = -(in * plane) / (norm * plane);
	if (dot != 0)
	{
		vec3 temp = in;
		temp += norm * dot;
		transform_vector(out, temp);
	}
	else
		transform_vector(out, in);
}

// compute location of origin of texture (should precompute)
void setup_origin_vector(vec3 &out, q1dplane_t &plane, vec3 &norm)
{
	vec3 temp;
	float d = plane.dist / (norm * plane.normal);
	temp = norm * d;

	transform_point_raw(out, temp);
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
	vec3 P, M, N;
	vec3 norm;
	q1dplane_t plane = data.dplanes[data.dfaces[face].planenum];
	float rescale = (8 >> mip) / 8.0f;

	vec3 tu = vec3(data.texinfo[tex].vecs[0]);
	vec3 tv = vec3(data.texinfo[tex].vecs[1]);

	compute_texture_normal(norm, tu, tv);
	// project vectors onto face's plane, and transform
	setup_uv_vector(M, tu, norm, plane.normal);
	setup_uv_vector(N, tv, norm, plane.normal);
	setup_origin_vector(P, plane, norm);

	u -= data.texinfo[tex].vecs[0].w;  // adjust according to face's info
	v -= data.texinfo[tex].vecs[1].w;  // passed in values adjust for lightmap

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

void transform_rotated_point(q1vertex_t &p)
{
	project_point(p);
	code_point(p);
}


static void intersect(q1vertex_t &out, q1vertex_t *a, q1vertex_t *b, float where)
{
	// intersection occurs 'where' % along the line from a to b

	out.p.x = a->p.x + (b->p.x - a->p.x) * where;
	out.p.y = a->p.y + (b->p.y - a->p.y) * where;
	out.p.z = a->p.z + (b->p.z - a->p.z) * where;

	transform_rotated_point(out);
}

static double left_loc(q1vertex_t *a, q1vertex_t *b)
{
	return -(a->p.z + a->p.x*clip_scale_x) / ((b->p.x - a->p.x)*clip_scale_x + b->p.z - a->p.z);
}

static double right_loc(q1vertex_t *a, q1vertex_t *b)
{
	return  (a->p.z - a->p.x*clip_scale_x) / ((b->p.x - a->p.x)*clip_scale_x - b->p.z + a->p.z);
}

static double top_loc(q1vertex_t *a, q1vertex_t *b)
{
	return  (a->p.z - a->p.y*clip_scale_y) / ((b->p.y - a->p.y)*clip_scale_y - b->p.z + a->p.z);
}

static double bottom_loc(q1vertex_t *a, q1vertex_t *b)
{
	return -(a->p.z + a->p.y*clip_scale_y) / ((b->p.y - a->p.y)*clip_scale_y + b->p.z - a->p.z);
}

// clip the polygon to each of the view frustrum planes
int clip_poly(int n, q1vertex_t **vl, int codes_or, q1vertex_t ***out_vl)
{
	int i, j, k, p = 0; // p = index into temporary point pool
	q1vertex_t **cur;

	if (codes_or & CC_OFF_LEFT)
	{
		cur = clip_list1;
		k = 0;
		j = n - 1;
		for (i = 0; i < n; ++i)
		{
			// process edge from j..i
			// if j is inside, add it

			if (!(vl[j]->ccodes & CC_OFF_LEFT))
				cur[k++] = vl[j];

			// if it crosses, add the intersection point

			if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_LEFT)
			{
				intersect(pts[p], vl[i], vl[j], left_loc(vl[i], vl[j]));
				cur[k++] = &pts[p++];
			}
			j = i;
		}
		// move output list to be input
		n = k;
		vl = cur;
	}
	if (codes_or & CC_OFF_RIGHT)
	{
		cur = (vl == clip_list1) ? clip_list2 : clip_list1;
		k = 0;
		j = n - 1;
		for (i = 0; i < n; ++i) {
			if (!(vl[j]->ccodes & CC_OFF_RIGHT))
				cur[k++] = vl[j];
			if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_RIGHT)
			{
				intersect(pts[p], vl[i], vl[j], right_loc(vl[i], vl[j]));
				cur[k++] = &pts[p++];
			}
			j = i;
		}
		n = k;
		vl = cur;
	}
	if (codes_or & CC_OFF_TOP)
	{
		cur = (vl == clip_list1) ? clip_list2 : clip_list1;
		k = 0;
		j = n - 1;
		for (i = 0; i < n; ++i)
		{
			if (!(vl[j]->ccodes & CC_OFF_TOP))
				cur[k++] = vl[j];
			if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_TOP)
			{
				intersect(pts[p], vl[i], vl[j], top_loc(vl[i], vl[j]));
				cur[k++] = &pts[p++];
			}
			j = i;
		}
		n = k;
		vl = cur;
	}
	if (codes_or & CC_OFF_BOT)
	{
		cur = (vl == clip_list1) ? clip_list2 : clip_list1;
		k = 0;
		j = n - 1;
		for (i = 0; i < n; ++i)
		{
			if (!(vl[j]->ccodes & CC_OFF_BOT))
				cur[k++] = vl[j];
			if ((vl[j]->ccodes ^ vl[i]->ccodes) & CC_OFF_BOT)
			{
				intersect(pts[p], vl[i], vl[j], bottom_loc(vl[i], vl[j]));
				cur[k++] = &pts[p++];
			}
			j = i;
		}
		n = k;
		vl = cur;
	}
	for (i = 0; i < n; ++i)
	{
		if (vl[i]->ccodes & CC_BEHIND)
			return 0;
	}

	*out_vl = vl;
	return n;
}


void QBsp::draw_face(Graphics &gfx, int face)
{
//	int tex_index = data.Face[face].texinfo;
//	int tex_data = data.TexInfo[tex_index].texdata;

//	gfx.SelectTexture(0, texdata_to_obj[tex_data]);
	//gfx.SelectTexture(8, face_lightmap_obj[tex_data]);

	if (map_selected == false)
	{
		gfx.SelectIndexBuffer(map_index_vbo);
		gfx.SelectVertexBuffer(map_vertex_vbo);
		map_selected = true;
	}
	gfx.DrawArrayTri(face_start_index[face], 0, face_count[face], face_count[face]);
}


void QBsp::build_face(Graphics &gfx, int face)
{
	int edge0;
	int edge1;
	int edge2;


	for (int i = 0; i < data.dfaces[face].numedges; ++i)
	{
		int edge_index = data.dsurfedges[data.dfaces[face].firstedge + i];

		// if edge is negative, edges are reversed (first edge is index 1, like quake2)
		bool reverse = (edge_index < 0);
		q1dedge_t edge = data.dedges[abs32(edge_index)];

		if (i == 0)
		{
			edge0 = edge.v[reverse ? 1 : 0];	// first edge, used in all triangles
		}
		else
		{
			// if first edge equals edge0, skip
			if (edge.v[reverse ? 1 : 0] == edge0)
				continue;

			// if second edge equals edge0 skip
			if (edge.v[reverse ? 0 : 1] == edge0)
				continue;

			edge1 = edge.v[reverse ? 1 : 0]; // first edge
			edge2 = edge.v[reverse ? 0 : 1]; // second edge

//			calculate_texcoords(face, edge0, edge1, edge2);
			if (face_start_index == NULL)
			{
				face_start_index = new int[data.num_faces];
				face_count = new int[data.num_faces];
				for (int i = 0; i < data.num_faces; i++)
				{
					face_start_index[i] = 0;
					face_count[i] = 0;
				}

			}


			if (face_start_index[face] == 0)
			{
				face_start_index[face] = index_array.size();
			}

			index_array.push_back(edge0);
			index_array.push_back(edge1);
			index_array.push_back(edge2);
			face_count[face] += 3;
		}
	}

}

void QBsp::change_axis()
{
	int            i, j, c;
	q1dmodel_t      *d;
	q1dmiptexlump_t   *mtl;


	for (i = 0; i < data.num_vertexes; i++)
	{
		float temp = (data.dvertexes[i].y);

		data.dvertexes[i].x = (data.dvertexes[i].x);
		data.dvertexes[i].y = (data.dvertexes[i].z);
		data.dvertexes[i].z = -temp;
	}

	for (i = 0; i < data.num_planes; i++)
	{
		float temp = (data.dplanes[i].normal.y);
		data.dplanes[i].normal.x = (data.dplanes[i].normal.x);
		data.dplanes[i].normal.y = (data.dplanes[i].normal.z);
		data.dplanes[i].normal.z = -temp;
	}

}



void rotate_c2w(vec3 *dest, vec3 *src)
{
	dest->x = src->x * main_matrix[0][0] + src->y * main_matrix[1][0] + src->z * main_matrix[2][0];
	dest->y = src->x * main_matrix[0][1] + src->y * main_matrix[1][1] + src->z * main_matrix[2][1];
	dest->z = src->x * main_matrix[0][2] + src->y * main_matrix[1][2] + src->z * main_matrix[2][2];
}

void compute_plane(q1dplane_t *plane, float x, float y, float z, vec3 &cam_loc)
{
	vec3 temp, temp2;
	temp2.x = x; temp2.y = z; temp2.z = y;
	rotate_c2w(&temp, &temp2);
	plane->normal = temp;
	plane->dist = temp.x*cam_loc.x + temp.y*cam_loc.y + temp.z*cam_loc.z;
}

void compute_view_frustrum(q1dplane_t *planes, vec3 &cam_loc)
{
	compute_plane(planes + 0, -1, 0, 1, cam_loc);
	compute_plane(planes + 1, 1, 0, 1, cam_loc);
	compute_plane(planes + 2, 0, 1, 1, cam_loc);
	compute_plane(planes + 3, 0, -1, 1, cam_loc);
}

int QBsp::find_leaf(vec3 &loc)
{
	int n = data.dmodels[0].headnode[0];
	while (n >= 0)
	{
		q1dnode_t *node = &data.dnodes[n];

		if (point_plane_test(loc, &data.dplanes[node->planenum]))
			n = node->back;
		else
			n = node->front;
	}
	return ~n;
}


int QBsp::visit_visible_leaves(vec3 &cam_loc)
{
	int n, v, i;

	memset(vis_leaf, 0, sizeof(vis_leaf));

	n = find_leaf(cam_loc);

	if (n == 0 || data.dleafs[n].visofs < 0)
		return 0;

	v = data.dleafs[n].visofs;
	for (i = 1; i < data.num_leafs; )
	{
		if (data.dvisdata[v] == 0)
		{
			i += 8 * data.dvisdata[v + 1];    // skip some leaves
			v += 2;
		}
		else
		{
			int j;
			for (j = 0; j <8; j++, i++)
				if (data.dvisdata[v] & (1 << j))
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
		vis_node[node] = !!(bsp_find_visible_nodes(data.dnodes[node].front) | bsp_find_visible_nodes(data.dnodes[node].back));
		return vis_node[node];
	}
	else
	{
		node = ~node;
		return (vis_leaf[node >> 3] & (1 << (node & 7)));
	}
}

int bbox_inside_plane(short *mins, short *maxs, q1dplane_t *plane)
{
	short pt[3];

	// use quick test from graphics gems

	if (FLOAT_POSITIVE(&plane->normal.x)) // fast test assuming IEEE
		pt[0] = maxs[0];
	else
		pt[0] = mins[0];

	if (FLOAT_POSITIVE(&plane->normal.y)) // fast test assuming IEEE
		pt[1] = maxs[1];
	else
		pt[1] = mins[1];

	if (FLOAT_POSITIVE(&plane->normal.z)) // fast test assuming IEEE
		pt[2] = maxs[2];
	else
		pt[2] = mins[2];


	return plane->normal.x * pt[0] + plane->normal.y * pt[1] + plane->normal.z * pt[2] >= plane->dist;
}


int QBsp::leaf_in_frustrum(q1dleaf_t *node, q1dplane_t *planes)
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
	int n = data.dleafs[leaf].nummarksurfaces;
	int ms = data.dleafs[leaf].firstmarksurface;
	int i;

	for (i = 0; i < n; ++i)
	{
		int s = data.dmarksurfaces[ms + i];
		if (!is_marked(s))
		{
			mark_face(s);
		}
	}
}

int node_in_frustrum(q1dnode_t *node, q1dplane_t *planes)
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
			if (leaf_in_frustrum(&data.dleafs[node], planes))
				mark_leaf_faces(node);
		return;
	}

	if (vis_node[node])
	{
		if (!node_in_frustrum(&data.dnodes[node], planes))
			vis_node[node] = 0;
		else
		{
			bsp_explore_node(data.dnodes[node].front);
			bsp_explore_node(data.dnodes[node].back);
		}
	}
}

void QBsp::bsp_visit_visible_leaves(vec3 &cam_loc, q1dplane_t *pl)
{
	planes = pl;
	bsp_find_visible_nodes((int)data.dmodels[0].headnode[0]);
	bsp_explore_node((int)data.dmodels[0].headnode[0]);
}


void QBsp::render(Graphics &gfx, vec3 &cam_loc)
{
	q1dplane_t planes[4];
	compute_view_frustrum(planes, cam_loc);

	if (!visit_visible_leaves(cam_loc))
	{
		memset(vis_leaf, 255, sizeof(vis_leaf));
	}

	bsp_visit_visible_leaves(cam_loc, planes);
	bsp_render_world(gfx, cam_loc, planes);
}