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
	void DrawText(const char *str, float x, float y);
	void LoadMatrix(const float *matrix);
	void MultMatrix(const float *matrix);
	void destroy();

	void VertexArray(void *vert);
	void TextureArray(void *tex);
	void NormalArray(void *normal);
	void DrawArray(char *type, void *index_array, unsigned int num_index, int num_verts);

	int  CreateIndexBuffer(void *index_buffer, int num_index);
	void SelectIndexBuffer(int handle);
	void DeleteIndexBuffer(int handle);

	int  CreateVertexBuffer(void *index_buffer, int num_index);
	void SelectVertexBuffer(int handle);
	void DeleteVertexBuffer(int handle);

	void SelectTexture(int index);
	void DeselectTexture();
	int LoadTexture(int width, int height, int components, int format, void *bytes);
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
	List<LPDIRECT3DVERTEXBUFFER9>	vertex_buffers;
	List<IDirect3DIndexBuffer9> index_buffers;
	List<IDirect3DTexture9> texture;
#else
	// storing int handle instead of pointer to some heap memory
	List<void *>texture;
#endif
};

#endif
