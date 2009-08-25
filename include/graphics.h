#include "include.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

class Graphics
{
public:
	Graphics();
	void init(void *param1, void *param2);
	void clear();
	void swap();
	void resize(const int width, const int height);
	void drawText(const char *str, float x, float y);
	void loadMatrix(const float *matrix);
	void destroy();

	void VertexArray(void *vert, int numVerts);
	void TextureArray(void *tex, int numTexs);
	void NormalArray(void *normal, int numNormals);
	void DrawArray(char *type, void *Indexes, int numIndexes );

	void InitTextures(int numTextures);
	void SelectTexture(int index);
	void DeselectTexture();
	void LoadTexture(int index, int width, int height, int components, int format, void *bytes);
	~Graphics();

private:
#ifdef _WIN32
	HWND	hwnd;
	HDC	hdc;
#else
	Display	*display;
	Window	window;	
	XFontStruct	*font;
#endif

#ifdef DIRECTX
	IDirect3D9				*d3d;
	IDirect3DDevice9		*device;
	D3DPRESENT_PARAMETERS	d3dpp;
	LPDIRECT3DVERTEXBUFFER9	vertexBuffer = NULL;
	IDirect3DIndexBuffer9	*indexBuffer = NULL;
#else
	unsigned int *texObject;
	int numTextures;
#endif
};

#endif
