#include "include.h"

#ifndef GRAPHICS_H
#define GRAPHICS_H

//#define ERROR_CHECK

class Graphics
{
public:
	Graphics();
	void init(void *param1, void *param2);
	bool error_check();
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

	void DrawArray(primitive_t primitive, int start_index, int start_vertex, unsigned int num_index, int num_verts);
	void DrawArrayTri(int start_index, int start_vertex, unsigned int num_index, int num_verts);
	void DrawArrayTriStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts);
	void DrawArrayLineStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts);
	void DrawArrayLine(int start_index, int start_vertex, unsigned int num_index, int num_verts);
	void DrawArrayPoint(int start_index, int start_vertex, unsigned int num_index, int num_verts);

	//Need to fix dx9 support cause of these guys
	void CreateVertexArrayObject(unsigned int &vao);
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
	int CreateCubeMap();
	void SelectCubemap(int texObject);
	int LoadTexture(int width, int height, int components, int format, void *bytes, bool clamp);
	void DeleteTexture(int handle);

	void SelectShader(int program);

	int checkFramebuffer();
	int setupFramebuffer(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &depth_tex);
	void bindFramebuffer(int fbo);
	void DeleteFrameBuffer(unsigned int fbo);
	void fbAttachTexture(int texObj);
	void fbAttachDepth(int texObj);
	void GetDebugLog();


	~Graphics();

public:
	int width, height;
#ifdef _WIN32
	HWND	hwnd;
	HDC	hdc;
#endif
#ifndef WIN32
#ifndef __OBJC__
	Display	*display;
	Window	window;	
	XFontStruct	*font;
#endif
#endif

#ifdef DIRECTX
	IDirect3D9				*d3d;
	IDirect3DDevice9		*device;
	D3DPRESENT_PARAMETERS	d3dpp;
	LPDIRECT3DVERTEXDECLARATION9		vertex_decl;
	vector<IDirect3DVertexBuffer9 *>	vertex_buffers;
	vector<IDirect3DIndexBuffer9 *> index_buffers;
	vector<IDirect3DTexture9 *> texture;
	vector<IDirect3DSurface9 *> surface;
	ID3DXFont *font;
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
	virtual void prelink() = 0;
protected:
	const char	*vertex_src, *geometry_src, *fragment_src;
#ifdef DIRECTX
	Graphics *gfx;
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
