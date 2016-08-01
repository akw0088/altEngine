#include "include.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

class Graphics
{
public:
	Graphics();
	void init(void *param1, void *param2);
	void clear();
	void cleardepth();
	void Depth(bool flag);
	void Blend(bool flag);
	void BlendFunc(char *src, char *dst);
	void CullFace(char *face);
	void Color(bool flag);
	void DepthFunc(char *op);
	void Stencil(bool flag);
	void StencilFunc(char *op, int ref, int mask);
	void StencilOp(char *stencil_fail, char *zfail, char *zpass);
	void DrawText(const char *str, float x, float y);

	void swap();
	void resize(const int width, const int height);
	void destroy();

	void DrawArray(char *type, int start_index, int start_vertex, unsigned int num_index, int num_verts);

	//Need to fix dx9 support cause of these guys
	int  CreateVertexArrayObject();
	void SelectVertexArrayObject(unsigned int vao);
	void DeleteVertexArrayObject(unsigned int vao);

	int  CreateIndexBuffer(void *index_buffer, int num_index);
	void SelectIndexBuffer(int handle);
	void DeleteIndexBuffer(int handle);

	int  CreateVertexBuffer(void *index_buffer, int num_verts);
	void SelectVertexBuffer(int handle);
	void DeleteVertexBuffer(int handle);

	void SelectTexture(int level, int index);
	void DeselectTexture(int level);
	int LoadTexture(int width, int height, int components, int format, void *bytes);
	void DeleteTexture(int handle);

	void SelectShader(int program);


	~Graphics();

public:
	int width, height;
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
	LPDIRECT3DVERTEXDECLARATION9		vertex_decl;
	vector<IDirect3DVertexBuffer9 *>	vertex_buffers;
	vector<IDirect3DIndexBuffer9 *> index_buffers;
	vector<IDirect3DTexture9 *> texture;
	ID3DXFont *font;
#else
	unsigned int vao;
#endif
};

class Shader
{
public:
	Shader();

	int init(Graphics *gfx, char *vertex_file,  char *geometry_file, char *fragment_file);
	void Select();
	void destroy();
	~Shader();
	virtual void prelink() = NULL;
protected:
	const char	*vertex_src, *geometry_src, *fragment_src;
	Graphics *gfx;
#ifdef DIRECTX
	IDirect3DVertexShader9		*vertex_shader;
	IDirect3DPixelShader9		*pixel_shader;
//	IDirect3DGeometryShader9	*geometry_shader;
	LPD3DXCONSTANTTABLE		uniform;
#else
	int		program_handle;
	int		vertex_handle;
	int		geometry_handle;
	int		fragment_handle;
#endif
};

#endif
