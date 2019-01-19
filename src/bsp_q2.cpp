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

// based on the bsp_hl code

#include "include.h"

#include "bsp_types_q2.h"

char *get_file(char *filename, unsigned int *size);
int abs32(int val);

using namespace q2;


int Q2Bsp::load(Graphics &gfx, char *filename)
{
	qheader_t *header = NULL;
	unsigned int size = 0;
	char *file = get_file(filename, &size);
	if (file == NULL)
	{
		char *pak = "media/PAK0.PAK";
		file = get_pakfile(pak, filename);
		return -1;
	}

	header = (qheader_t *)file;
	qdata_t data;

	data.Ent = (char *)(header->lump[q2::Entity].offset + file);
	data.plane = (qplane_t *)(header->lump[q2::Plane].offset + file);
	data.vertex = (vec3 *)(header->lump[Vertex].offset + file);
	data.vis = (qvis_t *)(header->lump[Visibility].offset + file);
	data.node = (qnode_t *)(header->lump[Node].offset + file);
	data.texinfo = (qtexinfo_t *)(header->lump[TexInfo].offset + file);
	data.face = (qface_t *)(header->lump[Face].offset + file);
	data.lightmap = (qlightmap_t *)(header->lump[Lightmap].offset + file);
	data.leaf = (qleaf_t *)(header->lump[Leaf].offset + file);
	data.leafface = (unsigned short *)(header->lump[LeafFace].offset + file);
	data.leafbrush = (qleafbrush_t *)(header->lump[LeafBrush].offset + file);
	data.edge = (qedge_t *)(header->lump[q2::Edge].offset + file);
	data.face_edge = (unsigned int *)(header->lump[FaceEdge].offset + file);
	data.model = (qmodel_t *)(header->lump[q2::Model].offset + file);
	data.brush = (qbrush_t *)(header->lump[Brush].offset + file);
	data.brushside = (qbrushside_t *)(header->lump[BrushSide].offset + file);
	data.pop = (qpop_t *)(header->lump[Pop].offset + file);
	data.area = (qarea_t *)(header->lump[Area].offset + file);
	data.area_portal = (qareaportal_t *)(header->lump[AreaPortal].offset + file);

	data.num_ent = header->lump[q2::Entity].length;
	data.num_plane = header->lump[q2::Plane].length / sizeof(qplane_t);
	data.num_vertex = header->lump[Vertex].length / sizeof(vec3);
	data.num_vis = header->lump[Visibility].length / sizeof(qvis_t);
	data.num_node = header->lump[Node].length / sizeof(qnode_t);
	data.num_texinfo = header->lump[TexInfo].length / sizeof(qtexinfo_t);
	data.num_face = header->lump[Face].length / sizeof(qface_t);
	data.num_lightmap = header->lump[Lightmap].length / sizeof(qlightmap_t);
	data.num_leaf = header->lump[Leaf].length / sizeof(qleaf_t);
	data.num_leafface = header->lump[LeafFace].length / sizeof(unsigned short);
	data.num_leafbrush = header->lump[LeafBrush].length / sizeof(qleafbrush_t);
	data.num_edge = header->lump[q2::Edge].length / sizeof(qedge_t);
	data.num_face_edge = header->lump[FaceEdge].length / sizeof(unsigned int);
	data.num_model = header->lump[q2::Model].length / sizeof(qmodel_t);
	data.num_brush = header->lump[Brush].length / sizeof(qbrush_t);
	data.num_brushside = header->lump[BrushSide].length / sizeof(qbrushside_t);
	data.num_pop = header->lump[Pop].length / sizeof(qpop_t);
	data.num_area = header->lump[Area].length / sizeof(qarea_t);
	data.num_area_portal = header->lump[AreaPortal].length / sizeof(qareaportal_t);




	return 0;
}















