#include "include.h"
#include "raster.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef SOFTWARE

matrix4 Graphics::current_mvp;

#ifdef THREAD
#ifdef WIN32
#include <process.h>
#define RTYPE DWORD
#define RVAL 0
#else
#include <pthread.h>
#define RTYPE void *
#define RVAL 0;
#endif

typedef struct
{
	raster_t type;
	int *pixels;
	float *zbuffer;
	int width;
	int height;
	matrix4 mvp;
	int *index_array;
	vertex_t *vertex_array;
	texinfo_t *texture;
	texinfo_t *lightmap;
	int start_index;
	int start_vertex;
	int num_index;
	int num_vert;
} param_t;

#define MAX_JOB 8192
#define MAX_THREAD 16
typedef struct
{
	volatile int work;
	volatile int idle;
	param_t param[MAX_JOB];
} work_t;

work_t work1[MAX_THREAD];

RTYPE thread1(void *num)
{
	int i = (int64_t)num;
	int last_job = 0;

	while (1)
	{
		while (work1[i].work == last_job || work1[i].work == 0)
		{
			work1[i].idle = 1;
	#ifdef WIN32
			Sleep(1);
	#else
			sleep(0);
	#endif
		}
		int next = last_job + 1;
		if (next >= MAX_JOB)
			next = 1;
		work1[i].idle = 0;

		if (work1[i].param[next].type == BARYCENTRIC)
		{
			raster_triangles(
				work1[i].param[next].type,
				i,
				work1[i].param[next].pixels,
				work1[i].param[next].zbuffer,
				work1[i].param[next].width,
				work1[i].param[next].height,
				work1[i].param[next].mvp,
				work1[i].param[next].index_array,
				work1[i].param[next].vertex_array,
				work1[i].param[next].texture,
				work1[i].param[next].lightmap,
				work1[i].param[next].start_index,
				work1[i].param[next].start_vertex,
				work1[i].param[next].num_index,
				work1[i].param[next].num_vert);
		}
		else if (work1[i].param[next].type == BARYCENTRIC_STRIP)
		{
			raster_triangles_strip(
				work1[i].param[next].type,
				i,
				work1[i].param[next].pixels,
				work1[i].param[next].zbuffer,
				work1[i].param[next].width,
				work1[i].param[next].height,
				work1[i].param[next].mvp,
				work1[i].param[next].index_array,
				work1[i].param[next].vertex_array,
				work1[i].param[next].texture,
				work1[i].param[next].lightmap,
				work1[i].param[next].start_index,
				work1[i].param[next].start_vertex,
				work1[i].param[next].num_index,
				work1[i].param[next].num_vert);
		}
		last_job = next;
	}

	return RVAL;
}

#endif


void Graphics::resize(int width, int height)
{
#ifdef WIN32
	SelectObject(hdcMem, hObject);
	DeleteObject(hBitmap);
	DeleteDC(hdcMem);
#endif

#ifdef THREAD

	for (int i = 0; i < 16; i++)
	{
//		if (pixel[i])
//			delete[] pixel[i];
		pixel[i] = new int[(width * height) * sizeof(int) + 256];
		zbuff[i] = new float[(width * height) * sizeof(float) + 256];
	}
#endif
//	if (pixels)
//		delete[] pixels;
	pixels = new int[width * height * sizeof(int)];
//	if (zbuffer)
//		delete[] zbuffer;
	zbuffer = new float[width * height * sizeof(float)];
	clear();
	center.x = width >> 1;
	center.y = height >> 1;

#ifdef WIN32
	hdcMem = CreateCompatibleDC(hdc);
	hBitmap = CreateCompatibleBitmap(hdc, width, height);
	hObject = SelectObject(hdcMem, hBitmap);
#endif
#ifdef __linux__
	if (image)
	{
		//XDestroyImage(image);
	}
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
	pixels = NULL;

#ifdef THREAD
	for (int i = 0; i < 16; i++)
	{
		pixel[i] = NULL;
		zbuff[i] = NULL;
	}
#endif
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
#ifdef THREAD
        for (int i = 0; i < 16; i++)
        {
                work1[i].work = 0;
#ifdef WIN32
//              _beginthread(thread1, 0, (void *)i);
                thread_handle[i] = CreateThread(NULL, 0, thread1, (void *)i, 0, (LPDWORD)&tid[i]);
                SetThreadPriority(thread_handle[i], THREAD_PRIORITY_ABOVE_NORMAL);
#else
                pthread_create(&tid[i], NULL, thread1, (void *)i);
#endif
        }
#endif



	zbuffer = new float[width*height * sizeof(float)];
	clear();

}

void Graphics::swap()
{
#ifdef THREAD
	if (pixels == NULL)
		return;

	// ensure all threads are finished
	for (int i = 0; i < 16; i++)
	{
		if (work1[i].idle != 1 )
		{
			i = 0;
		}
	}

	// copy each thread tile into frame buffer
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (x < width / 4 && y < height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[0][x + y * width];
			else if (x < 2 * width / 4 && y < height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[1][x + y * width];
			else if (x < 3 * width / 4 && y < height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[2][x + y * width];
			else if (x < width && y < height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[3][x + y * width];

			else if (x < width / 4 && y <  2 * height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[4][x + y * width];
			else if (x < 2 * width / 4 && y <  2 * height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[5][x + y * width];
			else if (x < 3 * width / 4 && y <  2 * height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[6][x + y * width];
			else if (x < width && y <  2 * height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[7][x + y * width];

			else if (x < width / 4 && y <  3 * height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[8][x + y * width];
			else if (x < 2 * width / 4 && y <  3 * height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[9][x + y * width];
			else if (x < 3 * width / 4 && y <  3 * height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[10][x + y * width];
			else if (x < width && y <  3 * height / 4)
				pixels[x + ((height - 1 - y) * width)] = pixel[11][x + y * width];

			else if (x < width / 4 && y <  height)
				pixels[x + ((height - 1 - y) * width)] = pixel[12][x + y * width];
			else if (x < 2 * width / 4 && y <  height)
				pixels[x + ((height - 1 - y) * width)] = pixel[13][x + y * width];
			else if (x < 3 * width / 4 && y <  height)
				pixels[x + ((height - 1 - y) * width)] = pixel[14][x + y * width];
			else if (x < width && y <  height)
				pixels[x + ((height - 1 - y) * width)] = pixel[15][x + y * width];
		}
	}
#endif
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
#ifdef THREAD

	if (pixel[0])
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (x < width / 4 && y < height / 4)
				{
					pixel[0][x + y * width] = 0xAAAAAAAA;
					zbuff[0][x + y * width] = 1.0f;
				}
				else if (x < 2 * width / 4 && y < height / 4)
				{
					pixel[1][x + y * width] = 0xAAAAAAAA;
					zbuff[1][x + y * width] = 1.0f;
				}
				else if (x < 3 * width / 4 && y < height / 4)
				{
					pixel[2][x + y * width] = 0xAAAAAAAA;
					zbuff[2][x + y * width] = 1.0f;
				}
				else if (x < width && y < height / 4)
				{
					pixel[3][x + y * width] = 0xAAAAAAAA;
					zbuff[3][x + y * width] = 1.0f;
				}

				else if (x < width / 4 && y < 2 * height / 4)
				{
					pixel[4][x + y * width] = 0xAAAAAAAA;
					zbuff[4][x + y * width] = 1.0f;
				}
				else if (x < 2 * width / 4 && y < 2 * height / 4)
				{
					pixel[5][x + y * width] = 0xAAAAAAAA;
					zbuff[5][x + y * width] = 1.0f;
				}
				else if (x < 3 * width / 4 && y < 2 * height / 4)
				{
					pixel[6][x + y * width] = 0xAAAAAAAA;
					zbuff[6][x + y * width] = 1.0f;
				}
				else if (x < width && y < 2 * height / 4)
				{
					pixel[7][x + y * width] = 0xAAAAAAAA;
					zbuff[7][x + y * width] = 1.0f;
				}

				else if (x < width / 4 && y < 3 * height / 4)
				{
					pixel[8][x + y * width] = 0xAAAAAAAA;
					zbuff[8][x + y * width] = 1.0f;
				}
				else if (x < 2 * width / 4 && y < 3 * height / 4)
				{
					pixel[9][x + y * width] = 0xAAAAAAAA;
					zbuff[9][x + y * width] = 1.0f;
				}
				else if (x < 3 * width / 4 && y < 3 * height / 4)
				{
					pixel[10][x + y * width] = 0xAAAAAAAA;
					zbuff[10][x + y * width] = 1.0f;
				}
				else if (x < width && y < 3 * height / 4)
				{
					pixel[11][x + y * width] = 0xAAAAAAAA;
					zbuff[11][x + y * width] = 1.0f;
				}

				else if (x < width / 4 && y < height)
				{
					pixel[12][x + y * width] = 0xAAAAAAAA;
					zbuff[12][x + y * width] = 1.0f;
				}
				else if (x < 2 * width / 4 && y < height)
				{
					pixel[13][x + y * width] = 0xAAAAAAAA;
					zbuff[13][x + y * width] = 1.0f;
				}
				else if (x < 3 * width / 4 && y < height)
				{
					pixel[14][x + y * width] = 0xAAAAAAAA;
					zbuff[14][x + y * width] = 1.0f;
				}
				else if (x < width && y < height)
				{
					pixel[15][x + y * width] = 0xAAAAAAAA;
					zbuff[15][x + y * width] = 1.0f;
				}
			}
		}
	}
#else
	if (pixels)
	{
		memset(pixels, 0xAA, width * height * sizeof(int));
	}
	if (zbuffer)
	{
		for (int i = 0; i < width * height; i++)
		{
			zbuffer[i] = 1.0f;
		}
	}
#endif
}

void Graphics::cleardepth()
{
#ifndef THREAD
	if (zbuffer)
	{
		for (int i = 0; i < width * height; i++)
		{
			zbuffer[i] = 1.0f;
		}
	}
#endif
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
	if (lightmap_tex == -1)
		lightmap_tex = 0;

#ifdef THREAD
	for (int i = 0; i < MAX_THREAD; i++)
	{
		int next = work1[i].work + 1;
		if (next >= MAX_JOB)
			next = 1;

		work1[i].param[next].type = BARYCENTRIC;
		work1[i].param[next].pixels = pixel[i];
		work1[i].param[next].zbuffer = zbuff[i];
		work1[i].param[next].width = width;
		work1[i].param[next].height = height;
		work1[i].param[next].mvp = current_mvp;
		work1[i].param[next].index_array = index_array[current_ibo];
		work1[i].param[next].vertex_array = vertex_array[current_vbo];
		work1[i].param[next].texture = &texture_array[current_tex];
		work1[i].param[next].lightmap = &texture_array[lightmap_tex];
		work1[i].param[next].start_index = start_index;
		work1[i].param[next].start_vertex = start_vertex;
		work1[i].param[next].num_index = num_index;
		work1[i].param[next].num_vert = num_verts;
		work1[i].work = next;
	}
#else
	raster_triangles(BARYCENTRIC, -1, pixels, zbuffer, width, height, current_mvp, index_array[current_ibo], vertex_array[current_vbo], &texture_array[current_tex], &texture_array[lightmap_tex], start_index, start_vertex, num_index, num_verts);
#endif
	gpustat.drawcall++;
	gpustat.triangle += num_index / 3;
}

void Graphics::DrawArrayTriStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	if (current_tex == -1)
		current_tex = 0;

#ifdef THREAD
	for (int i = 0; i < MAX_THREAD; i++)
	{
		int next = work1[i].work + 1;
		if (next >= MAX_JOB)
			next = 1;

		work1[i].param[next].type = BARYCENTRIC_STRIP;
		work1[i].param[next].pixels = pixel[i];
		work1[i].param[next].zbuffer = zbuff[i];
		work1[i].param[next].width = width;
		work1[i].param[next].height = height;
		work1[i].param[next].mvp = current_mvp;
		work1[i].param[next].index_array = index_array[current_ibo];
		work1[i].param[next].vertex_array = vertex_array[current_vbo];
		work1[i].param[next].texture = &texture_array[current_tex];
		work1[i].param[next].lightmap = &texture_array[lightmap_tex];
		work1[i].param[next].start_index = start_index;
		work1[i].param[next].start_vertex = start_vertex;
		work1[i].param[next].num_index = num_index;
		work1[i].param[next].num_vert = num_verts;
		work1[i].work = next;
	}
#else
	raster_triangles_strip(BARYCENTRIC_STRIP, -1, pixels, zbuffer, width, height, current_mvp, index_array[current_ibo], vertex_array[current_vbo], &texture_array[current_tex], &texture_array[lightmap_tex], start_index, start_vertex, num_index, num_verts);
#endif
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
	switch (level)
	{
	case 0:
		current_tex = texObject;
		break;
	default:
		lightmap_tex = texObject;
		break;
	}
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
	tex.data = (int *)new char[width * height * components];
	tex.width = width;
	tex.height = height;
	tex.components = components;
	memcpy(&tex.data[0], bytes, width * height * components);
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


/*
if (block > 0)
{
	barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
		tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
		tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
		tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
		s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3,
		(block % BLOCKDIV)* width / BLOCKDIV, (((block) % BLOCKDIV) + 1) * width / BLOCKDIV,
		(block / BLOCKDIV) * height / BLOCKDIV, ((block + BLOCKDIV) / BLOCKDIV) * height / BLOCKDIV);
}
else
{
	barycentric_triangle(pixels, zbuffer, width, height, texture, lightmap,
		tri[j + 0].x, tri[j + 0].y, tri[j + 0].z, tri[j + 0].w, RGB(255, 0, 0),
		tri[j + 1].x, tri[j + 1].y, tri[j + 1].z, tri[j + 1].w, RGB(0, 255, 0),
		tri[j + 2].x, tri[j + 2].y, tri[j + 2].z, tri[j + 2].w, RGB(0, 0, 255),
		s1, t1, s2, t2, s3, t3, ls1, lt1, ls2, lt2, ls3, lt3, 0, width, 0, height);
}

*/