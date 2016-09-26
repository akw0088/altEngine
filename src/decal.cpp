#include "decal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//Not sure if this really deserves it's own class, but why not...
Decal::Decal(Entity *entity)
{
	Decal::entity = entity;
	decal_tex = -1;
}

void Decal::init(Graphics &gfx)
{
//	decal_tex = gfx.LoadTexture(128, 128, 3, GL_RGB, (void *)"media/bullethole.tga");
}

void Decal::render(Graphics &gfx, Global &global)
{
/*
	if (decal_tex == -1)
		init(gfx);

	global.Select();
	global.Params(mvp, decal_tex);
	gfx.SelectIndexBuffer(Model::quad_index);
	gfx.SelectVertexBuffer(Model::quad_vertex);
	gfx.SelectTexture(0, decal_tex);
	gfx.DrawArray(PRIM_TRIANGLES, 0, 0, 6, 4);
	gfx.DeselectTexture(0);
*/
}

