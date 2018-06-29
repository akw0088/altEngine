#include "include.h"
#include "raster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef SOFTWARE

matrix4 Graphics::current_mvp;

void Graphics::resize(int width, int height)
{
#ifdef WIN32
	SelectObject(hdcMem, hObject);
	DeleteObject(hBitmap);
	DeleteDC(hdcMem);
#endif
#ifdef __linux__
	if (image)
	{
		XDestroyImage(image);
	}
#endif

	if (pixels)
		delete[] pixels;
	pixels = new int[width * height * sizeof(int) + 512];
//	if (zbuffer)
//		delete[] zbuffer;
	zbuffer = new float[width * height * sizeof(float)];
	clear();
	center.x = width / 2;
	center.y = height / 2;

#ifdef WIN32
	hdcMem = CreateCompatibleDC(hdc);
	hBitmap = CreateCompatibleBitmap(hdc, width, height);
	hObject = SelectObject(hdcMem, hBitmap);
#endif
#ifdef __linux__
	image = XCreateImage(display, vis, depth, ZPixmap, 0, (char*)pixels, width, height, 32, 0);
#endif
	Graphics::width = width;
	Graphics::height = height;
}

Graphics::Graphics()
{
	width = 1;
	height = 1;

	num_index_array = 0;
	num_vertex_array = 0;
	zbuffer = NULL;
#ifdef __linux__
	image = NULL;
#endif
}


Graphics::~Graphics()
{
}

void Graphics::init(void *param1, void *param2)
{
#ifdef WIN32
	hwnd = *((HWND *)param1);
	hdc = *((HDC *)param2);
	hdcMem = CreateCompatibleDC(hdc);
	hBitmap = CreateCompatibleBitmap(hdc, width, height);
	hObject = SelectObject(hdcMem, hBitmap);
#endif
#ifdef __linux__
	display = (Display *)param1;
	window = *((Window *)param2);
#ifdef SOFTWARE
	screen = DefaultScreen(display);
	vis = DefaultVisual(display,screen);
	depth  = DefaultDepth(display,screen);
	gc = DefaultGC(display,screen);
#endif
#endif
	zbuffer = new float[width*height * sizeof(float)];
	clear();

}

void Graphics::swap()
{
#ifdef WIN32
	SetBitmapBits(hBitmap, width * height * sizeof(int), pixels);
	BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);
#endif
#ifdef __linux__
	if (image)
	{
		XPutImage(display, window, gc, image, 0, 0, 0, 0, width, height);
		XFlush(display);
	}
#endif

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
		for (int i = 0; i < width * height; i++)
		{
			zbuffer[i] = 1.0f;
		}
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
	if (current_tex == -1)
		current_tex = 0;
	raster_triangles(BARYCENTRIC, pixels, zbuffer, width, height, current_mvp, index_array[current_ibo], vertex_array[current_vbo], &texture_array[current_tex], start_index, start_vertex, num_index, num_verts);
	//	glDrawElementsBaseVertex(GL_TRIANGLES, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);

	gpustat.drawcall++;
	gpustat.triangle += num_index / 3;
}

void Graphics::DrawArrayTriStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	if (current_tex == -1)
		current_tex = 0;
	raster_triangles_strip(BARYCENTRIC, pixels, zbuffer, width, height, current_mvp, index_array[current_ibo], vertex_array[current_vbo], &texture_array[current_tex], start_index, start_vertex, num_index, num_verts);
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
	current_tex = texObject;
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
	texinfo_t tex;
	tex.data = new int[width * height];
	tex.width = width;
	tex.height = height;
	memcpy(tex.data, bytes, sizeof(int) * width * height);
	texture_array.push_back(tex);
	return texture_array.size() - 1;
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
