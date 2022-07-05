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

#include "include.h"
#include <cstddef>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define ERROR_CHECK

#ifdef OPENGL
void Graphics::resize(int width, int height)
{
	Graphics::width = width;
	Graphics::height = height;

	if (!height)
		height = 1;
	glViewport(0, 0, width, height);
}

Graphics::Graphics()
{

	width = 0;
	height = 0;
	gpustat.drawcall = 0;
	gpustat.triangle = 0;
#ifdef _WIN32
	hwnd = 0;
	hdc = 0;
#endif
#ifdef __linux__
	display = NULL;
	font = NULL;
#endif
}


void Graphics::init(void *param1, void *param2)
{
#ifdef _WIN32
	hwnd = *((HWND *)param1);
	hdc = *((HDC *)param2);
#endif
#ifdef __linux__
	display = (Display *)param1;
	window = *((Window *)param2);
#endif

	memset(&gpustat, 0, sizeof(gpustat_t));
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearStencil(0);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(5.0f);
	glLineWidth(5.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
//	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	//glEnable(GL_STENCIL_TEST);
	//glStencilMask(0x00); // disable writes to stencil
	glClearStencil(0x00); // clear stencil to zero
	glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);; // always pass stencil, set to 0xFF
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // replace if 0xFF if passes


	//disable vsync
#ifdef WIN32
	wglSwapIntervalEXT(0);
#else
	//glXSwapInterval(0);
#endif


//	float bias = -3.0f;
	//glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);

#ifdef ERROR_CHECK
	error_check();
#endif
}

Graphics::~Graphics()
{
}

void Graphics::swap()
{
	gpustat.drawcall = 0;
	gpustat.triangle = 0;
#ifdef _WIN32
	SwapBuffers(hdc);
#endif
#ifdef __linux__
	glXSwapBuffers(display, window);
#endif

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::clear()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::clear_multi()
{
	static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//static const GLfloat one = 1.0f;

//	glClearBufferfv(GL_COLOR, 0, black);
	glClearBufferfv(GL_COLOR, 1, black);
//	glClearBufferfv(GL_DEPTH, 0, &one);
}

void Graphics::cleardepth()
{
	glClear( GL_DEPTH_BUFFER_BIT );

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::clear_color(vec3 &color)
{
	glClearColor(color.x, color.y, color.z, 1.0f);
}

void Graphics::clearstencil()
{
	glClear(GL_STENCIL_BUFFER_BIT);

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::Depth(bool flag)
{
	if (flag)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::Blend(bool flag)
{
	if (flag)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::BlendFuncDstColorOne()
{
	glBlendFunc(GL_DST_COLOR, GL_ONE);
}

void Graphics::BlendFuncDstColorZero()
{
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
}

void Graphics::BlendFuncZeroOneMinusAlpha()
{
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
}

void Graphics::BlendFuncOneAlpha()
{
	glBlendFunc(GL_ONE, GL_SRC_ALPHA);
}


void Graphics::BlendFuncOneOneMinusAlpha()
{
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}


void Graphics::BlendFuncOneOne()
{
	glBlendFunc(GL_ONE, GL_ONE);

#ifdef ERROR_CHECK
	error_check();
#endif
}


void Graphics::BlendFuncZeroSrcColor()
{
	//blendfunc gl_zero gl_src_color
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
}

void Graphics::BlendFuncZeroOne()
{
	glBlendFunc(GL_ZERO, GL_ONE);

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::BlendFuncDstColorOneMinusDstAlpha()
{
	glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_DST_ALPHA);
}

void Graphics::BlendFuncDstColorSrcAlpha()
{
	glBlendFunc(GL_DST_COLOR, GL_SRC_ALPHA);
}

void Graphics::BlendFuncOneMinusSrcAlphaSrcAlpha()
{
	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
}

void Graphics::BlendFuncSrcAlphaOneMinusSrcAlpha()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void Graphics::BlendFuncOneSrcAlpha()
{
	glBlendFunc(GL_ONE, GL_SRC_ALPHA);
}

void Graphics::BlendFuncOneMinusDstColorZero()
{
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
}

void Graphics::BlendFuncDstColorSrcColor()
{
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
}

void Graphics::BlendFuncZeroSrcAlpha()
{
	glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
}

void Graphics::BlendFuncOneZero()
{
	glBlendFunc(GL_ONE, GL_ZERO);

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::destroy()
{

}

void Graphics::DrawArray(primitive_t primitive, int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	// Branches in rendering loop are slow, find faster portable method
	if (primitive == PRIM_TRIANGLES)
		glDrawElementsBaseVertex(GL_TRIANGLES, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
	else if (primitive == PRIM_TRIANGLE_STRIP)
		glDrawElementsBaseVertex(GL_TRIANGLE_STRIP, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
	else if (primitive == PRIM_LINE_STRIP)
		glDrawElementsBaseVertex(GL_LINE_STRIP, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
	else if (primitive == PRIM_LINES)
		glDrawElementsBaseVertex(GL_LINES, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
	else if (primitive == PRIM_POINTS)
		glDrawElementsBaseVertex(GL_POINTS, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::DrawArrayTri(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	gpustat.drawcall++;
	gpustat.triangle += num_index / 3;
	glDrawElementsBaseVertex(GL_TRIANGLES, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
}

void Graphics::DrawArrayTriStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	gpustat.drawcall++;
	gpustat.triangle += num_index / 2 - 1;
	glDrawElementsBaseVertex(GL_TRIANGLE_STRIP, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
}

void Graphics::DrawArrayLineStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	gpustat.drawcall++;
	glDrawElementsBaseVertex(GL_LINE_STRIP, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
}

void Graphics::DrawArrayLine(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	gpustat.drawcall++;
	glDrawElementsBaseVertex(GL_LINES, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
}

void Graphics::DrawArrayPoint(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	gpustat.drawcall++;
	glDrawElementsBaseVertex(GL_POINTS, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
}

int Graphics::CreateIndexBuffer(void *index_buffer, int num_index)
{
	unsigned int	ibo;

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_index * sizeof(int), index_buffer, GL_STATIC_DRAW);

	gpustat.buffer_size += num_index * sizeof(int);
	return ibo;

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::SelectIndexBuffer(int handle)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);

#ifdef ERROR_CHECK
	error_check();
#endif

}

void Graphics::DeleteIndexBuffer(int handle)
{
	glDeleteBuffers(1, (unsigned int *)&handle);

#ifdef ERROR_CHECK
	error_check();
#endif
}


/*
	Vertex Array objects store the following:
	Bound index buffer
	Bound vertex buffer
	glEnableVertexAttribArray() state
	glVertexAttribPointer() state
*/
void Graphics::CreateVertexArrayObject(unsigned int &vao)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// This is all cached in a vertex array object
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::SelectVertexArrayObject(unsigned int vao)
{
	glBindVertexArray(vao);

#ifdef ERROR_CHECK
		error_check();
#endif
}

void Graphics::DeleteVertexArrayObject(unsigned int vao)
{
		glDeleteVertexArrays(1, &vao);

#ifdef ERROR_CHECK
		error_check();
#endif
}

int Graphics::CreateVertexBuffer(void *vertex_buffer, int num_vertex, bool dynamic)
{
	unsigned int	vbo;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// This is cached into the current vbo
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(offsetof(vertex_t, texCoord0)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(offsetof(vertex_t, texCoord1)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(offsetof(vertex_t, normal)));
	glVertexAttribPointer(4, 1, GL_INT,   GL_FALSE,	sizeof(vertex_t), (void *)(offsetof(vertex_t, color)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,	sizeof(vertex_t), (void *)(offsetof(vertex_t, tangent)));

	if (dynamic == false)
		glBufferData(GL_ARRAY_BUFFER, num_vertex * sizeof(vertex_t), vertex_buffer, GL_STATIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, num_vertex * sizeof(vertex_t), vertex_buffer, GL_DYNAMIC_DRAW);


	gpustat.buffer_size += num_vertex * sizeof(vertex_t);
#ifdef ERROR_CHECK
	error_check();
#endif
	return vbo;
}

void Graphics::SelectVertexBuffer(int handle)
{
	glBindBuffer(GL_ARRAY_BUFFER, handle);

	// This is cached into the current vbo
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2)));
	glVertexAttribPointer(4, 1, GL_INT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2) + sizeof(vec3)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2) + sizeof(vec3) + sizeof(int)));


#ifdef ERROR_CHECK
	error_check();
#endif

}

void Graphics::DeleteVertexBuffer(int handle)
{
	glDeleteBuffers(1, (unsigned int *)&handle);

#ifdef ERROR_CHECK
	error_check();
#endif

}


void Graphics::SelectCubemap(int texObject)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, texObject);

#ifdef ERROR_CHECK
	error_check();
#endif
}


void Graphics::SelectTexture(int level, int texObject)
{
	//hack for blended surfaces
	if (texObject < 0)
		texObject = -texObject;

	glActiveTexture(GL_TEXTURE0 + level);
	glBindTexture(GL_TEXTURE_2D, texObject);

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::SelectTextureArray(int level, int texObject)
{
	//hack for blended surfaces
	if (texObject < 0)
		texObject = -texObject;

	glActiveTexture(GL_TEXTURE0 + level);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texObject);

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::DeselectTexture(int level)
{
	glActiveTexture(GL_TEXTURE0 + level);
	glBindTexture(GL_TEXTURE_2D, 0);

#ifdef ERROR_CHECK
	error_check();
#endif
}


bool Graphics::error_check()
{
	GLenum err;

	err = glGetError();
	if ( err != GL_NO_ERROR)
	{
		//1281 invalid enum
		//1282 bad value
		printf("GL_ERROR %d\n", err);
		return true;
	}
	return false;
}


int Graphics::CreateCubeMap()
{
	unsigned int texObject;

	glGenTextures(1, &texObject);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texObject);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

#ifdef ERROR_CHECK
	error_check();
#endif

	return texObject;
}

int Graphics::LoadTexture(int width, int height, int components, int format, void *bytes, bool clamp, int anisotropic)
{
	unsigned int texObject = -1;

	glGenTextures(1, &texObject);
	glBindTexture(GL_TEXTURE_2D, texObject);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	if (anisotropic)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)anisotropic);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, format, GL_UNSIGNED_BYTE, bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	gpustat.texture_size += width * height * 4;

#ifdef ERROR_CHECK
	error_check();
#endif

	return texObject;
}

int Graphics::LoadTextureArray(int width, int height, int components, int format, void **bytes, bool clamp, int num_layer)
{
	unsigned int texObject = -1;

	// using so far only for depth cube maps
	glGenTextures(1, &texObject);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texObject);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Linear seems to work too
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32, width, height, num_layer, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);


	if (bytes != NULL)
	{
		for (int i = 0; i < num_layer; i++)
		{
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
				width, height, 1, format, GL_UNSIGNED_BYTE, bytes[i]);
		}
	}


#ifdef ERROR_CHECK
	error_check();
#endif

	return texObject;
}

void Graphics::DeleteTexture(int handle)
{
	glDeleteTextures(1, (unsigned int *)&handle);

#ifdef ERROR_CHECK
	error_check();
#endif

}

void Graphics::SelectShader(int program)
{
	glUseProgram(program);

#ifdef ERROR_CHECK
	error_check();
#endif

}

void Graphics::CullFace(int mode)
{
	static int last_mode = BACKFACE;

	if (mode == last_mode)
		return;
	last_mode = mode;

	if (mode == BACKFACE)
		glCullFace(GL_BACK);
	else if (mode == FRONTFACE)
		glCullFace(GL_FRONT);
	else if (mode == NONE)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);

#ifdef ERROR_CHECK
	error_check();
#endif

}

void Graphics::Color(bool flag)
{
	if (flag)
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	else
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

#ifdef ERROR_CHECK
	error_check();
#endif

}

void Graphics::Stencil(bool flag)
{
	if (flag)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);

#ifdef ERROR_CHECK
	error_check();
#endif

}

void Graphics::StencilFunc(int op, int ref, unsigned int mask)
{
	if (op == ALWAYS)
		glStencilFunc(GL_ALWAYS, ref, mask);
	else if (op == EQUAL)
		glStencilFunc(GL_EQUAL, ref, mask);
	else if (op == NEVER)
		glStencilFunc(GL_NEVER, ref, mask);
	else if (op == GEQUAL)
		glStencilFunc(GL_GEQUAL, ref, mask);
	else if (op == GREATER)
		glStencilFunc(GL_GREATER, ref, mask);
	else if (op == LESS)
		glStencilFunc(GL_LESS, ref, mask);
	else if (op == LEQUAL)
		glStencilFunc(GL_LEQUAL, ref, mask);


#ifdef ERROR_CHECK
	error_check();
#endif
}


void Graphics::DepthFunc(int op)
{
	if (op == LEQUAL)
		glDepthFunc(GL_LEQUAL);
	else if (op == LESS)
		glDepthFunc(GL_LESS);

#ifdef ERROR_CHECK
	error_check();
#endif

}

void Graphics::StencilOp(int stencil_fail, int zfail, int zpass)
{
	glStencilOp(stencil_fail, zfail, zpass);

#ifdef ERROR_CHECK
	error_check();
#endif
}

void Graphics::TwoSidedStencilOp(int face, int stencil_fail, int zfail, int zpass)
{
	glStencilOpSeparate(face, stencil_fail, zfail, zpass);

#ifdef ERROR_CHECK
	error_check();
#endif
}



Shader::Shader()
{
	vertex_src = NULL;
	geometry_src = NULL;
	fragment_src = NULL;
	program_handle = 0;
	vertex_handle = 0;
	geometry_handle = 0;
	fragment_handle = 0;
}


int Shader::init(Graphics *gfx, char *vertex_file, char *geometry_file, char *fragment_file)
{
	FILE		*fLog;
	int			success;
	int			max_attrib = 0;
	int			size = 0;

	fLog = fopen("infolog.txt", "a");
	fprintf(fLog, "OpenGL Version %s\n", glGetString(GL_VERSION));
	fprintf(fLog, "OpenGL Renderer %s\n", glGetString(GL_RENDERER));
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attrib);
	fprintf(fLog, "Max vertex attribs %d\n", max_attrib);

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	fprintf(fLog, "Max texture size %dx%d\n", size, size);


	if (vertex_file)
	{
		vertex_src = get_file(vertex_file, NULL);
		if (vertex_src == NULL)
		{
			fprintf(fLog, "Unable to load vertex shader %s\n", vertex_file);
			fclose(fLog);
			return -1;
		}
		else
		{
			fprintf(fLog, "Loaded vertex shader %s\n", vertex_file);
		}


		vertex_handle = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_handle, 1, &vertex_src, NULL);
		glCompileShader(vertex_handle);
		glGetShaderiv(vertex_handle, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			int		info_size;
			char	*info_log;

			glGetShaderiv(vertex_handle, GL_INFO_LOG_LENGTH, &info_size);
			info_log = new char [info_size];

			glGetShaderInfoLog(vertex_handle, info_size, NULL, info_log);
			fprintf(fLog, "Error compiling: %s", info_log);
			fclose(fLog);
			delete [] info_log;
			return -1;
		}
		delete[] vertex_src;
		vertex_src = NULL;
	}

	if (geometry_file)
	{
		geometry_src = get_file(geometry_file, NULL);
		if (geometry_src == NULL)
		{
			fprintf(fLog, "Unable to load geometry shader %s\n", geometry_file);
			fclose(fLog);
			return -1;
		}
		else
		{
			fprintf(fLog, "Loaded geometry shader %s\n", geometry_file);
		}

		geometry_handle = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry_handle, 1, &geometry_src, NULL);
		glCompileShader(geometry_handle);
		glGetShaderiv(geometry_handle, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			int		info_size;
			char	*info_log;

			glGetShaderiv(geometry_handle, GL_INFO_LOG_LENGTH, &info_size);
			info_log = new char [info_size];

			glGetShaderInfoLog(geometry_handle, info_size, NULL, info_log);
			fprintf(fLog, "Error compiling: %s", info_log);
			fclose(fLog);
			delete [] info_log;
			return -1;
		}
		delete[] geometry_src;
		geometry_src = NULL;
	}

	if (fragment_file)
	{
		fragment_src = get_file(fragment_file, NULL);
		if (fragment_src == NULL)
		{
			fprintf(fLog, "Unable to load fragment shader %s\n", fragment_file);
			fclose(fLog);
			return -1;
		}
		else
		{
			fprintf(fLog, "Loaded fragment shader %s\n", fragment_file);
		}

		fragment_handle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_handle, 1, &fragment_src, NULL);
		glCompileShader(fragment_handle);
		glGetShaderiv(fragment_handle, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			int		info_size;
			char	*info_log;

			glGetShaderiv(fragment_handle, GL_INFO_LOG_LENGTH, &info_size);
			info_log = new char [info_size];

			glGetShaderInfoLog(fragment_handle, info_size, NULL, info_log);
			fprintf(fLog, "Error compiling: %s", info_log);
			fclose(fLog);
			delete [] info_log;
			return -1;
		}
		delete[] fragment_src;
		fragment_src = NULL;
	}

	program_handle = glCreateProgram();
	if (vertex_file)
		glAttachShader(program_handle, vertex_handle);
	if (geometry_file)
		glAttachShader(program_handle, geometry_handle);
	if (fragment_file)
		glAttachShader(program_handle, fragment_handle);
	prelink();
	glLinkProgram(program_handle);
	glGetProgramiv(program_handle, GL_LINK_STATUS, &success);
	if (!success)
	{
		int		info_size;
		char	*info_log;

		glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &info_size);
		info_log = new char [info_size];

		glGetProgramInfoLog(program_handle, info_size, NULL, info_log);
		fprintf(fLog, "Error linking: %s", info_log);
		fclose(fLog);
		delete [] info_log;
		return -1;
	}

	fprintf(fLog, "Link successful\n");
	fclose(fLog);
	return 0;
}

void Shader::Select()
{
	glUseProgram(program_handle);
}

Shader::~Shader()
{
	destroy();
}

void Shader::destroy()
{
	if (vertex_src)
	{
		delete [] vertex_src;
	}
	if (geometry_src)
	{
		delete [] geometry_src;
	}
	if (fragment_src)
	{
		delete [] fragment_src;
	}


	vertex_src = NULL;
	geometry_src = NULL;
	fragment_src = NULL;

	glDeleteProgram(program_handle);
	glDeleteShader(vertex_handle);
	glDeleteShader(fragment_handle);
}



void Graphics::fbAttachTexture(int texObj)
{
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texObj, 0);
}

void Graphics::fbAttachTextureOne(int texObj)
{
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texObj, 0);
}


void Graphics::fbAttachDepth(int texObj)
{
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texObj, 0);
}


void Graphics::bindFramebuffer(int fbo, int num_attach)
{
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	if (fbo)
	{
		glDrawBuffers(num_attach, &attachments[0]);
	}
}

int Graphics::checkFramebuffer()
{
	GLenum fboStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
#ifndef MACOS
#ifndef __OBJC__
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("Render to texture failed\n");
		switch (fboStatus)
		{
		case GL_FRAMEBUFFER_UNDEFINED:
			printf("GL_FRAMEBUFFER_UNDEFINED\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			printf("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			printf("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			printf("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			printf("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			printf("GL_FRAMEBUFFER_UNSUPPORTED\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			printf("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			printf("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n");
			break;
		}
		return -1;
	}
#endif
#endif
	return 0;
}

int Graphics::CreateFramebuffer(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &depth_tex, unsigned int &normal_depth, int multisample, bool twoattach)
{
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &quad_tex);
	glBindTexture(GL_TEXTURE_2D, quad_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (multisample > 0)
	{
#ifndef __OBJC__
		glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisample, GL_RGB16F, width, height, GL_TRUE);
#endif
        //		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisample, GL_RGB16F, width, height, GL_FALSE);
	}
	else
	{
#ifndef __OBJC__
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB16F, width, height);
#endif
        //	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, quad_tex, 0);

	if (twoattach)
	{
		glGenTextures(1, &normal_depth);
		glBindTexture(GL_TEXTURE_2D, normal_depth);

		if (multisample > 0)
		{
#ifndef __OBJC__
			glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisample, GL_RGB32F, width, height, GL_TRUE);
#endif
        }
		else
		{
#ifndef __OBJC__
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
#endif
            
        }
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normal_depth, 0);
	}
	glBindTexture(GL_TEXTURE_2D, 0);


	glGenTextures(1, &depth_tex);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Linear seems to work too
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	float border_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
//	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	if (multisample > 0)
	{
//		glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisample, GL_RGB32F, width, height, GL_TRUE);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisample, GL_DEPTH_COMPONENT24, width, height, GL_TRUE);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 0);
	}



	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depth_tex, 0);
	glBindTexture(GL_TEXTURE_2D, 0);


	if (checkFramebuffer() != 0)
	{
		printf("Failed to create fbo\n");
		return -1;
	}

	fbAttachTexture(quad_tex);
	if (twoattach)
	{
		fbAttachTextureOne(normal_depth);
	}
	fbAttachDepth(depth_tex);


	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	return 0;
}

/*
int Graphics::CreateFramebufferArray(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &array_tex, int layer, int multisample)
{
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &quad_tex);
	glBindTexture(GL_TEXTURE_2D, quad_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	if (multisample > 0)
	{
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisample, GL_RGBA8, width, height, GL_FALSE);
	}

	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, quad_tex, 0);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, array_tex, layer);

	if (checkFramebuffer() != 0)
	{
		printf("Failed to create fbo\n");
		return -1;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	return 0;
}
*/

void Graphics::DeleteFrameBuffer(int fbo, int quad, int depth)
{
	glDeleteFramebuffers(1, (unsigned int *)&fbo);

	if (quad != -1)
		glDeleteTextures(1, (unsigned int *)&quad);
	if (depth != -1)
		glDeleteTextures(1, (unsigned int *)&depth);
}

void Graphics::GetDebugLog()
{
#ifdef WIN32
	GLint maxMsgLen = 0;
	glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &maxMsgLen);

	static char buffer[4096] = { 0 };
	static GLenum	source[1000];
	static GLenum	type[1000];
	static GLenum	severity[1000];
	static GLuint	id[1000];
	static GLsizei	length[1000];


	GLuint numFound = glGetDebugMessageLog(1000, 4096, &source[0], &type[0], &id[0], &severity[0], &length[0], &buffer[0]);

	FILE *fp = fopen("error.log", "w+");
	if (fp == NULL)
	{
		printf("Unable to open error.log\n");
		return;
	}

	int buf_length = 0;
	for (unsigned int i = 0; i < numFound; i++)
	{
		fprintf(fp, "source %d type %d id %d severity %d msg [%s]\n", source[i], type[i], id[i], severity[i], &buffer[buf_length]);
		buf_length += length[i];
	}
	fclose(fp);
#endif
}

#endif
