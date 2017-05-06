#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef DEDICATED
void Graphics::resize(int width, int height)
{
}

Graphics::Graphics()
{
}


Graphics::~Graphics()
{
}

void Graphics::init(void *param1, void *param2)
{
}

void Graphics::swap()
{
	gpustat.drawcall = 0;
	gpustat.triangle = 0;
}

void Graphics::clear()
{
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


void Graphics::BlendFunc(char *src, char *dst)
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
	return 1;
}

void Graphics::SelectIndexBuffer(int handle)
{
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

int Graphics::CreateVertexBuffer(void *vertex_buffer, int num_vertex)
{
	return 1;
}

void Graphics::SelectVertexBuffer(int handle)
{
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

int Graphics::LoadTexture(int width, int height, int components, int format, void *bytes, bool clamp)
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

void Graphics::StencilFunc(char *op, int ref, int mask)
{
}

void Graphics::DepthFunc(char *op)
{
}

void Graphics::StencilOp(char *stencil_fail, char *zfail, char *zpass)
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

void Graphics::bindFramebuffer(int fbo)
{
}

int Graphics::checkFramebuffer()
{
	return 0;
}

int Graphics::setupFramebuffer(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &depth_tex)
{
	return 0;
}

void Graphics::DeleteFrameBuffer(unsigned int fbo)
{
}

void Graphics::GetDebugLog()
{
}
#endif
