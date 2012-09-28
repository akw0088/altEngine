#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void Graphics::resize(int width, int height)
{
	Graphics::width = width;
	Graphics::height = height;

	if (!height)
		height = 1;
#ifndef DIRECTX
	glViewport(0, 0, width, height);
#endif
}

Graphics::Graphics()
{
}



/*==============================================================
	DIRECTX ONLY SECTION
================================================================*/
#ifdef DIRECTX
Graphics::~Graphics() 
{
	font->Release();
}

void Graphics::cleardepth()
{
	device->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,255,255), 1.0f, 0);
}

void Graphics::DepthFunc(char *op)
{
}

void Graphics::Blend(bool flag)
{
	if (flag)
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	else
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void Graphics::BlendFunc(char *src, char *dst)
{
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void Graphics::Color(bool flag)
{
	if (flag)
		device->SetRenderState(D3DRS_COLORWRITEENABLE, TRUE);
	else
		device->SetRenderState(D3DRS_COLORWRITEENABLE, FALSE);	
}

void Graphics::Stencil(bool flag)
{
	if (flag)
		device->SetRenderState(D3DRS_STENCILENABLE, TRUE);
	else
		device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
}

void Graphics::StencilFunc(char *op, int ref, int mask)
{
	if (strcmp(op, "always") == 0)
		device->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
	else if (strcmp(op, "equal") == 0)
		device->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_EQUAL );

	device->SetRenderState( D3DRS_STENCILREF, ref);
	device->SetRenderState( D3DRS_STENCILMASK, mask);
}

void Graphics::StencilOp(char *stencil_fail, char *zfail, char *zpass)
{
	if (strcmp(zpass, "incr") == 0)
		device->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCR );
	else if (strcmp(zpass, "decr") == 0)
		device->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );
	else if (strcmp(zpass, "keep") == 0)
		device->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );


	device->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
	device->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
}

void Graphics::CullFace(char *face)
{
	if (strcmp(face, "back") == 0)
		device->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	else
		device->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
}

void Graphics::Depth(bool flag)
{
	if (flag)
		device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	else
		device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
}

void Graphics::init(void *param1, void *param2)
{
	hwnd = *((HWND *)param1);
	hdc = *((HDC *)param2);
	HRESULT		ret;

	memset(&d3dpp, sizeof(d3dpp), 0);
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.hDeviceWindow = hwnd;

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL)
		return;

	ret = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &device);
	if (ret != D3D_OK)
		return;

	// Render States
    device->SetRenderState(D3DRS_ZENABLE, TRUE);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	device->SetRenderState(D3DRS_LIGHTING, FALSE);
//	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// Texture Unit States
	device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	
	D3DXCreateFont( device, 8, 0, FW_THIN, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "System", &font );

	D3DVERTEXELEMENT9 decl[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, sizeof(vec3), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{0, sizeof(vec3) + sizeof(vec2), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
		{0, sizeof(vec3) + 2 * sizeof(vec2), D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 2 * sizeof(vec3) + 2 * sizeof(vec2), D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END()
	};
	device->CreateVertexDeclaration(decl, &vertex_decl);
}

void Graphics::DrawText(const char *str, float x, float y)
{
	RECT rect;

	rect.left = (int)(x * width);
	rect.top = (int)(y * height);
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;

	font->DrawText(NULL, str, -1, &rect, DT_LEFT | DT_NOCLIP, 0xFFFFFFFF );
}

void Graphics::swap()
{
	device->EndScene();
	device->Present(NULL, NULL, NULL, NULL);
}

void Graphics::clear()
{
	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(128, 128, 128), 1.0f, 0);
	device->BeginScene();
}

void Graphics::destroy()
{
	if (device)
		device->Release();
	if (d3d)
		d3d->Release();

}

int Graphics::CreateIndexBuffer(void *index_array, int num_index)
{
	IDirect3DIndexBuffer9	**d3d9_buffer = new IDirect3DIndexBuffer9 *;
	void *pIndex = NULL;

	device->CreateIndexBuffer(num_index * sizeof(int), 0, D3DFMT_INDEX32,  D3DPOOL_DEFAULT, d3d9_buffer, NULL);
	(*d3d9_buffer)->Lock(0, num_index * sizeof(int), &pIndex, 0);
	memcpy(pIndex, index_array, num_index * sizeof(int));
	(*d3d9_buffer)->Unlock();
	index_buffers.push_back(*d3d9_buffer);
	return index_buffers.size() - 1;
}

void Graphics::SelectIndexBuffer(int handle)
{
	IDirect3DIndexBuffer9	*d3d9_buffer = index_buffers[handle];

	device->SetIndices(d3d9_buffer);
}

void Graphics::DeleteIndexBuffer(int handle)
{
	IDirect3DIndexBuffer9	*d3d9_buffer = index_buffers[handle];

//	index_buffers.remove(d3d9_buffer);
	d3d9_buffer->Release();
//	delete d3d9_buffer;
}

void Graphics::DrawArray(char *type, int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	/* Branches in rendering loop are slow, find faster portable method */
	if ( strcmp(type, "triangles") == 0 )
		device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, start_vertex, 0, num_verts, start_index, num_index / 3);
	else if (strcmp(type, "triangle_strip") == 0)
		device->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, start_vertex, 0, num_verts, start_index, num_index - 2);
	else if (strcmp(type, "line_strip") == 0)
		device->DrawIndexedPrimitive(D3DPT_LINESTRIP, start_vertex, 0, num_verts, start_index, num_index - 1);
	else if (strcmp(type, "points") == 0)
		device->DrawIndexedPrimitive(D3DPT_POINTLIST, start_vertex, 0, num_verts, start_index, num_index);

}

int Graphics::CreateVertexBuffer(void *vertex_array, int num_verts)
{
	void *pVert = NULL;
	LPDIRECT3DVERTEXBUFFER9 *d3d9_buffer = new LPDIRECT3DVERTEXBUFFER9;

	device->CreateVertexBuffer(num_verts * sizeof(vertex_t), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, d3d9_buffer, NULL);
	(*d3d9_buffer)->Lock(0, num_verts * sizeof(vertex_t), &pVert, 0);
	memcpy(pVert, vertex_array, num_verts * sizeof(vertex_t));
	(*d3d9_buffer)->Unlock();

	vertex_buffers.push_back(*d3d9_buffer);
	return vertex_buffers.size() - 1;
}

void Graphics::SelectVertexBuffer(int handle)
{
	IDirect3DVertexBuffer9 *d3d9_buffer = vertex_buffers[handle];

	if (handle != 0)
		device->SetStreamSource(0, d3d9_buffer, 0, sizeof(vertex_t));
	device->SetVertexDeclaration(vertex_decl);
}

void Graphics::DeleteVertexBuffer(int handle)
{
	IDirect3DVertexBuffer9 *d3d9_buffer = vertex_buffers[handle];

//	vertex_buffers.remove(d3d9_buffer);
	d3d9_buffer->Release();
//	delete d3d9_buffer;
}

void Graphics::SelectTexture(int level, int handle)
{
	if (handle == -1)
		return;
	device->SetTexture(level, texture[handle]);
}

void Graphics::DeselectTexture(int level)
{
	device->SetTexture(level, NULL);
}

int Graphics::LoadTexture(int width, int height, int components, int format, void *bytes)
{
	IDirect3DTexture9	**d3d9_buffer = new IDirect3DTexture9 *;
	D3DLOCKED_RECT		rect;

	device->CreateTexture(width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, d3d9_buffer, NULL);
	(*d3d9_buffer)->LockRect(0, &rect, NULL, 0);
	memcpy((void *)rect.pBits, bytes, width * height * components);
	(*d3d9_buffer)->UnlockRect(0);
	(*d3d9_buffer)->GenerateMipSubLevels();
	texture.push_back(*d3d9_buffer);
	return texture.size() - 1;
}

void Graphics::DeleteTexture(int handle)
{
	if (handle == -1)
		return;

	IDirect3DTexture9	*d3d9_buffer = texture[handle];
//	texture.remove(d3d9_buffer);
	d3d9_buffer->Release();
//	delete d3d9_buffer;
}

void Graphics::SelectShader(int handle)
{
	device->SetVertexShader(NULL);
	device->SetPixelShader(NULL);
}

int Shader::init(Graphics *gfx, char *vertex_file,  char *geometry_file, char *fragment_file)
{
	Shader::gfx = gfx;
	LPD3DXBUFFER err;
	FILE *fLog = fopen("infolog.txt", "a");

	if (vertex_file)
	{
		LPD3DXBUFFER vertex_binary;
		vertex_src = (char *)getFile(vertex_file);
		if (vertex_src == NULL)
		{
			fprintf(fLog, "Unable to load vertex shader %s\n", vertex_file);
			fclose(fLog);
			return -1;
		}

		D3DXCompileShader(vertex_src, strlen(vertex_src) + 1,
			NULL, NULL, "main", "vs_3_0", D3DXSHADER_SKIPOPTIMIZATION, &vertex_binary, &err, &uniform);
		if (err)
		{
			fprintf(fLog, "Unable to load vertex shader %s\n%s\n", vertex_file, err->GetBufferPointer());
			fclose(fLog);
			return -1;
		}
		gfx->device->CreateVertexShader((DWORD *)vertex_binary->GetBufferPointer(), &vertex_shader);
		vertex_binary->Release();
		fprintf(fLog, "Loaded vertex shader %s\n", vertex_file);
	}

	if (geometry_file)
	{
		/*
		LPD3DXBUFFER geometry_binary;
		geometry_src = (char *)getFile(geometry_file);

		D3DXCompileShader(geometry_src, strlen(geometry_src) + 1,
			NULL, NULL, "main", "gs_3_0", 0, &geometry_binary, NULL, &constants);
		device->CreateGeometryShader((DWORD *)geometry_binary->GetBufferPointer(), &geometry_shader);
		geometry_binary->Release();
		*/
	}

	if (fragment_file)
	{
		LPD3DXBUFFER pixel_binary;
		fragment_src = (char *)getFile(fragment_file);
		if (fragment_src == NULL)
		{
			fprintf(fLog, "Unable to load fragment shader %s\n", fragment_file);
			fclose(fLog);
			return -1;
		}

		D3DXCompileShader(fragment_src, strlen(fragment_src) + 1,
			NULL, NULL, "main", "ps_3_0", D3DXSHADER_SKIPOPTIMIZATION, &pixel_binary, &err, &uniform);
		if (err)
		{
			fprintf(fLog, "Unable to load fragment shader %s\n%s\n", fragment_file, err->GetBufferPointer());
			fclose(fLog);
			return -1;
		}
		gfx->device->CreatePixelShader((DWORD *)pixel_binary->GetBufferPointer(), &pixel_shader);
		pixel_binary->Release();
		fprintf(fLog, "Loaded fragment shader %s\n", fragment_file);
	}

	fclose(fLog);
	return 0;
}

void Shader::Select()
{
	if (vertex_src)
		gfx->device->SetVertexShader(vertex_shader);
	if (fragment_src)
		gfx->device->SetPixelShader(pixel_shader);
}

void Shader::destroy()
{
	if (vertex_src)
		delete [] vertex_src;
	if (fragment_src)
		delete [] fragment_src;

	if (vertex_shader)
	{
		vertex_shader->Release();
		vertex_shader = NULL;
	}
	if (pixel_shader)
	{
		pixel_shader->Release();
		pixel_shader = NULL;
	}

}

Shader::~Shader()
{
	destroy();
}


#else
/*====================================================================================
	Opengl support while still easy to cheat and use functions directly for rapid dev
======================================================================================*/
Graphics::~Graphics()
{
}

void Graphics::init(void *param1, void *param2)
{
	int gl_error;

#ifdef _WIN32
	hwnd = *((HWND *)param1);
	hdc = *((HDC *)param2);
#else
	display = (Display *)param1;
	window = *((Window *)param2);
#endif

#ifdef _WIN32
//	wglUseFontBitmaps(hdc, ' ', '~', 1000);
#else
/*
	font = XLoadQueryFont(display, "-*-courier-bold-r-normal--14-*-*-*-*-*-*-*");
	if (font)
		glXUseXFont(font->fid, ' ', '~', 1000);
	else
		printf("Unable to load font!\n");
*/
#endif
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClearStencil(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);



	gl_error = glGetError();
	if (gl_error != GL_NO_ERROR)
	{
		printf("GL_ERROR %d\n", gl_error);
	}
}

// requires hand creation of font image and indexing/drawing quad for each glyph
// we are in clip coordinates (-1,1)
/*
void Graphics::DrawText(const char *str, float x, float y)
{
	glRasterPos2f(x * 2.0f - 1.0f, (1.0f - y) * 2.0f - 1.0f);
	glListBase(1000 - ' ');
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
}
*/

void Graphics::swap()
{
#ifdef _WIN32
	SwapBuffers(hdc);
#else
	glXSwapBuffers(display, window);
#endif
}

void Graphics::clear()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}

void Graphics::cleardepth()
{
	glClear( GL_DEPTH_BUFFER_BIT );
}

void Graphics::Depth(bool flag)
{
	if (flag)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
}

void Graphics::Blend(bool flag)
{
	if (flag)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}

void Graphics::BlendFunc(char *src, char *dst)
{
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void Graphics::destroy()
{
/*
	if (font)
		XUnloadFont(display, font->fid);
	glDeleteLists(1000, '~' - ' ');
*/
}

void Graphics::DrawArray(char *type, int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	GLenum err;

	err = glGetError();
	if ( err != GL_NO_ERROR)
	{
		printf("DrawArray GL_ERROR %d: attempting to draw will likely blow things up...\n", err);
		return;
	}

	// This is cached into the current vao
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2)));
	glVertexAttribPointer(4, 1, GL_INT,   GL_FALSE,	sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2) + sizeof(vec3)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,	sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2) + sizeof(vec3) + sizeof(int)));


	// Branches in rendering loop are slow, find faster portable method
	if ( strcmp(type, "triangles") == 0 )
		glDrawElementsBaseVertex(GL_TRIANGLES, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
	else if (strcmp(type, "triangle_strip") == 0)
		glDrawElementsBaseVertex(GL_TRIANGLE_STRIP, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
	else if (strcmp(type, "line_strip") == 0)
		glDrawElementsBaseVertex(GL_LINE_STRIP, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
	else if (strcmp(type, "lines") == 0)
		glDrawElementsBaseVertex(GL_LINES, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
	else if (strcmp(type, "points") == 0)
		glDrawElementsBaseVertex(GL_POINTS, num_index, GL_UNSIGNED_INT, (void *)(start_index * sizeof(int)), start_vertex);
}

int Graphics::CreateIndexBuffer(void *index_buffer, int num_index)
{
	unsigned int	ibo;

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_index * sizeof(int), index_buffer, GL_STATIC_DRAW);
	return ibo;

}

void Graphics::SelectIndexBuffer(int handle)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
}

void Graphics::DeleteIndexBuffer(int handle)
{
	glDeleteBuffers(1, (unsigned int *)&handle);
}


/*
	Vertex Array objects store the following:
	Bound index buffer
	Bound vertex buffer
	glEnableVertexAttribArray() state
	glVertexAttribPointer() state
*/
int Graphics::CreateVertexArrayObject()
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
	return vao;
}

void Graphics::SelectVertexArrayObject(unsigned int vao)
{
		glBindVertexArray(vao);
}

void Graphics::DeleteVertexArrayObject(unsigned int vao)
{
		glDeleteVertexArrays(1, &vao);
}

int Graphics::CreateVertexBuffer(void *vertex_buffer, int num_vertex)
{
	unsigned int	vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, num_vertex * sizeof(vertex_t), vertex_buffer, GL_STATIC_DRAW);

	// This is cached into the current vao
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2)));
	glVertexAttribPointer(4, 1, GL_INT,   GL_FALSE,	sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2) + sizeof(vec3)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,	sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2) + sizeof(vec3) + sizeof(int)));


	return vbo;
}

void Graphics::SelectVertexBuffer(int handle)
{
	glBindBuffer(GL_ARRAY_BUFFER, handle);
}

void Graphics::DeleteVertexBuffer(int handle)
{
	glDeleteBuffers(1, (unsigned int *)&handle);
}


void Graphics::SelectTexture(int level, int texObject)
{
//	if (texObject == -1)
//		return;

	//hack for blended surfaces
	if (texObject < 0)
		texObject = -texObject;

	glActiveTexture(GL_TEXTURE0 + level);
//	glEnable(GL_TEXTURE_2D);  -- this bastard is deprecated and cause my engine to crash and burn
	glBindTexture(GL_TEXTURE_2D, texObject);
}

void Graphics::DeselectTexture(int level)
{
	glActiveTexture(GL_TEXTURE0 + level);
	glBindTexture(GL_TEXTURE_2D, 0);
//	glDisable(GL_TEXTURE_2D);
}

int Graphics::LoadTexture(int width, int height, int components, int format, void *bytes)
{
	unsigned int texObject;

	glGenTextures(1, &texObject);
	glBindTexture(GL_TEXTURE_2D, texObject);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, format, GL_UNSIGNED_BYTE, bytes);
	glGenerateMipmap(GL_TEXTURE_2D);
	return texObject;
}

void Graphics::DeleteTexture(int handle)
{
	glDeleteTextures(1, (unsigned int *)&handle);
}

void Graphics::SelectShader(int program)
{
	glUseProgram(program);
}

void Graphics::CullFace(char *face)
{
	if (strcmp(face, "back") == 0)
		glCullFace(GL_BACK);
	else
		glCullFace(GL_FRONT);
}

void Graphics::Color(bool flag)
{
	if (flag)
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	else
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void Graphics::Stencil(bool flag)
{
	if (flag)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
}

void Graphics::StencilFunc(char *op, int ref, int mask)
{
	if (strcmp(op, "always") == 0)
		glStencilFunc(GL_ALWAYS, ref, mask);
	else if (strcmp(op, "equal") == 0)
		glStencilFunc(GL_EQUAL, ref, mask);
}

void Graphics::DepthFunc(char *op)
{
	if (strcmp(op, "<=") == 0)
		glDepthFunc(GL_LEQUAL);
	else if (strcmp(op, "<") == 0)
		glDepthFunc(GL_LESS);
}

void Graphics::StencilOp(char *stencil_fail, char *zfail, char *zpass)
{
	if (strcmp(zpass, "incr") == 0)
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	else if (strcmp(zpass, "decr") == 0)
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	else if (strcmp(zpass, "keep") == 0)
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}



int Shader::init(Graphics *gfx, char *vertex_file, char *geometry_file, char *fragment_file)
{
	FILE		*fLog;
	int			success;
	int			max_attrib = 0;
	
	Shader::gfx = gfx;
	fLog = fopen("infolog.txt", "a");
	fprintf(fLog, "OpenGL Version %s\n", glGetString(GL_VERSION));
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attrib);
	fprintf(fLog, "Max vertex attribs %d\n", max_attrib);

	if (vertex_file)
	{
		vertex_src = get_file(vertex_file);
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
	}

	if (geometry_file)
	{
		geometry_src = get_file(geometry_file);
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

		geometry_handle = glCreateShader(GL_GEOMETRY_SHADER_EXT);
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
	}

	if (fragment_file)
	{
		fragment_src = get_file(fragment_file);
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
	if (fragment_src)
	{
		delete [] fragment_src;
	}

	vertex_src = NULL;
	fragment_src = NULL;

	glDeleteProgram(program_handle);
	glDeleteShader(vertex_handle);
	glDeleteShader(fragment_handle);
}

#endif

