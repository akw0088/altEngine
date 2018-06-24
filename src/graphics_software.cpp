#include "include.h"
#include "raster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef SOFTWARE

matrix4 Graphics::current_mvp;

void Graphics::resize(int width, int height)
{
	SelectObject(hdcMem, hObject);
	DeleteObject(hBitmap);
	DeleteDC(hdcMem);

	if (pixels)
		delete[] pixels;
	pixels = new int[width * height * sizeof(int)];
	if (zbuffer)
		delete[] zbuffer;
	zbuffer = new int[width * height * sizeof(int)];
	clear();
	center.x = width / 2;
	center.y = height / 2;

	hdcMem = CreateCompatibleDC(hdc);
	hBitmap = CreateCompatibleBitmap(hdc, width, height);
	hObject = SelectObject(hdcMem, hBitmap);
	Graphics::width = width;
	Graphics::height = height;
}

Graphics::Graphics()
{
	width = 1;
	height = 1;

	num_index_array = 0;
	num_vertex_array = 0;
}


Graphics::~Graphics()
{
}

void Graphics::init(void *param1, void *param2)
{
	hwnd = *((HWND *)param1);
	hdc = *((HDC *)param2);
	hdcMem = CreateCompatibleDC(hdc);
	hBitmap = CreateCompatibleBitmap(hdc, width, height);
	hObject = SelectObject(hdcMem, hBitmap);
	zbuffer = new int[width*height * sizeof(int)];
	clear();

}

void Graphics::swap()
{
	SetBitmapBits(hBitmap, width * height * sizeof(int), pixels);
	BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

	gpustat.drawcall = 0;
	gpustat.triangle = 0;
}

void Graphics::clear()
{
	if (pixels)
	{
		memset(pixels, 0xAAAAAAAA, width * height * sizeof(int));
	}
	if (zbuffer)
	{
		memset(zbuffer, 0x0, width * height * sizeof(int));
	}
}

void Graphics::cleardepth()
{
}

void Graphics::Depth(bool flag)
{
}

void Graphics::Blend(bool flag)
{
}

void Graphics::BlendFuncDstColorOne()
{
}

void Graphics::BlendFuncDstColorZero()
{
}

void Graphics::BlendFuncZeroOneMinusAlpha()
{
}

void Graphics::BlendFuncOneAlpha()
{
}


void Graphics::BlendFuncOneOneMinusAlpha()
{
}


void Graphics::BlendFuncOneOne()
{
}


void Graphics::BlendFuncZeroSrcColor()
{
}

void Graphics::BlendFuncZeroOne()
{
}

void Graphics::BlendFuncDstColorOneMinusDstAlpha()
{
}

void Graphics::BlendFuncDstColorSrcAlpha()
{
}

void Graphics::BlendFuncOneMinusSrcAlphaSrcAlpha()
{
}

void Graphics::BlendFuncSrcAlphaOneMinusSrcAlpha()
{
}


void Graphics::BlendFuncOneSrcAlpha()
{
}

void Graphics::BlendFuncOneMinusDstColorZero()
{
}

void Graphics::BlendFuncDstColorSrcColor()
{
}

void Graphics::BlendFuncZeroSrcAlpha()
{
}

void Graphics::BlendFuncOneZero()
{
}

void Graphics::destroy()
{
}

void Graphics::DrawArray(primitive_t primitive, int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
}

void Graphics::DrawArrayTri(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	raster_triangles(pixels, zbuffer, width, height, current_mvp, index_array[current_ibo], vertex_array[current_vbo], start_index, start_vertex, num_index, num_verts);
	gpustat.drawcall++;
	gpustat.triangle += num_index / 3;
}

void Graphics::DrawArrayTriStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	gpustat.drawcall++;
	gpustat.triangle += num_index / 2 - 1;
}

void Graphics::DrawArrayLineStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	gpustat.drawcall++;
}

void Graphics::DrawArrayLine(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	gpustat.drawcall++;
}

void Graphics::DrawArrayPoint(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	gpustat.drawcall++;
}

int Graphics::CreateIndexBuffer(void *index_buffer, int num_index)
{
	index_array[num_index_array] = new int[num_index];
	memcpy(index_array[num_index_array++], index_buffer, num_index * sizeof(int));
	return num_index_array - 1;
}

void Graphics::SelectIndexBuffer(int handle)
{
	current_ibo = handle;
}

void Graphics::DeleteIndexBuffer(int handle)
{
}

void Graphics::CreateVertexArrayObject(unsigned int &vao)
{
}

void Graphics::SelectVertexArrayObject(unsigned int vao)
{
}

void Graphics::DeleteVertexArrayObject(unsigned int vao)
{
}

int Graphics::CreateVertexBuffer(void *vertex_buffer, int num_vertex, bool dynamic)
{
	vertex_array[num_vertex_array] = new vertex_t[num_vertex];
	memcpy(vertex_array[num_vertex_array++], vertex_buffer, num_vertex * sizeof(vertex_t));
	return num_vertex_array - 1;
}

void Graphics::SelectVertexBuffer(int handle)
{
	current_vbo = handle;
}

void Graphics::DeleteVertexBuffer(int handle)
{
}


void Graphics::SelectCubemap(int texObject)
{
}

void Graphics::SelectTexture(int level, int texObject)
{
}

void Graphics::DeselectTexture(int level)
{
}


bool Graphics::error_check()
{
	return false;
}


int Graphics::CreateCubeMap()
{
	return 0;
}

int Graphics::LoadTexture(int width, int height, int components, int format, void *bytes, bool clamp, int anisotropic)
{
	return 1;
}

void Graphics::DeleteTexture(int handle)
{
}

void Graphics::SelectShader(int program)
{
}

void Graphics::CullFace(int mode)
{
}

void Graphics::Color(bool flag)
{
}

void Graphics::Stencil(bool flag)
{
}

void Graphics::StencilFunc(int op, int ref, unsigned int mask)
{
}

void Graphics::DepthFunc(int op)
{
}

void Graphics::StencilOp(int stencil_fail, int zfail, int zpass)
{
}

Shader::Shader()
{
}


int Shader::init(Graphics *gfx, char *vertex_file, char *geometry_file, char *fragment_file)
{
	return 0;
}

void Shader::Select()
{
}

Shader::~Shader()
{
}

void Shader::destroy()
{
}



void Graphics::fbAttachTexture(int texObj)
{
}

void Graphics::fbAttachDepth(int texObj)
{
}

void Graphics::bindFramebuffer(int fbo, int num_attach)
{
}

int Graphics::checkFramebuffer()
{
	return 0;
}

void Graphics::clear_color(vec3 &color)
{

}

void Graphics::TwoSidedStencilOp(int face, int stencil_fail, int zfail, int zpass)
{
}

int Graphics::CreateFramebuffer(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &depth_tex, unsigned int &normal_depth, int multisample, bool twoattach)
{
	return 0;
}

void Graphics::DeleteFrameBuffer(int fbo, int quad, int depth)
{
}

void Graphics::GetDebugLog()
{
}
#endif
