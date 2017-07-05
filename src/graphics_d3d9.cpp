#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define ERROR_CHECK

#ifdef D3D9
void Graphics::resize(int width, int height)
{
	Graphics::width = width;
	Graphics::height = height;

	if (!height)
		height = 1;

	D3DVIEWPORT9 viewport;

	viewport.Width = width;
	viewport.Height = height;
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;
	viewport.X = 0;
	viewport.Y = 0;

	HRESULT ret = device->SetViewport(&viewport);
	if (ret != D3D_OK)
	{
		printf("resize error!!!\n");
	}
}

Graphics::Graphics()
{
}

Graphics::~Graphics() 
{
	//font->Release();
}

void Graphics::cleardepth()
{
	device->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,255,255), 1.0f, 0);
}

void Graphics::DepthFunc(char *op)
{
	if (strcmp(op, "<=") == 0)
		device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	else if (strcmp(op, "<") == 0)
		device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
}

bool Graphics::error_check()
{
	return false;
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

void Graphics::CullFace(int mode)
{
	if (mode == 0)
		device->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	else if (mode == 1)
		device->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
	else
		device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
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
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.hDeviceWindow = hwnd;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL)
		return;

	ret = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &device);
	if (ret != D3D_OK)
		return;

	// Render States
    device->SetRenderState(D3DRS_ZENABLE, TRUE);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW); // d3d and opengl will have opposite winding
	device->SetRenderState(D3DRS_LIGHTING, FALSE);
//	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	
	device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

	// Texture Unit States
	for (int i = 0; i < 4; i++)
	{
		device->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		device->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		device->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		device->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		device->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	}
	//D3DXCreateFont( device, 8, 0, FW_THIN, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
	//	DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "System", &font );


	D3DVERTEXELEMENT9 decl[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{0, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
		{0, 28, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 40, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		{0, 44, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
		D3DDECL_END()
	};

	ret = device->CreateVertexDeclaration(decl, &vertex_decl);
	device->SetVertexDeclaration(vertex_decl);
}

void Graphics::DrawText(const char *str, float x, float y)
{
	RECT rect;

	rect.left = (int)(x * width);
	rect.top = (int)(y * height);
	rect.right = rect.left + width;
	rect.bottom = rect.top + height;

	//font->DrawText(NULL, str, -1, &rect, DT_LEFT | DT_NOCLIP, 0xFFFFFFFF );
}

void Graphics::swap()
{
	device->EndScene();
	device->Present(NULL, NULL, NULL, NULL);
}

void Graphics::clear()
{
	vec4 clear = { 0.5f, 0.5f, 0.5f, 1.0f };

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
	HRESULT ret;
	void *pIndex = NULL;

	ret = device->CreateIndexBuffer(num_index * sizeof(int), 0, D3DFMT_INDEX32,  D3DPOOL_DEFAULT, d3d9_buffer, NULL);
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

void Graphics::DrawArray(primitive_t primitive, int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	/* Branches in rendering loop are slow, find faster portable method */
	if (primitive == PRIM_TRIANGLES)
		device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, start_vertex, 0, num_verts, start_index, num_index / 3);
	else if (primitive == PRIM_TRIANGLE_STRIP)
		device->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, start_vertex, 0, num_verts, start_index, num_index - 2);
	else if (primitive == PRIM_LINE_STRIP)
		device->DrawIndexedPrimitive(D3DPT_LINESTRIP, start_vertex, 0, num_verts, start_index, num_index - 1);
	else if (primitive == PRIM_POINTS)
		device->DrawIndexedPrimitive(D3DPT_POINTLIST, start_vertex, 0, num_verts, start_index, num_index);
}

void Graphics::DrawArrayTri(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, start_vertex, 0, num_verts, start_index, num_index / 3);
}

void Graphics::DrawArrayTriStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	device->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, start_vertex, 0, num_verts, start_index, num_index - 2);
}

void Graphics::DrawArrayLineStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	device->DrawIndexedPrimitive(D3DPT_LINESTRIP, start_vertex, 0, num_verts, start_index, num_index - 1);
}

void Graphics::DrawArrayPoint(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	device->DrawIndexedPrimitive(D3DPT_POINTLIST, start_vertex, 0, num_verts, start_index, num_index);
}


void Graphics::CreateVertexArrayObject(unsigned int &vao)
{
	return;
}

void Graphics::SelectVertexArrayObject(unsigned int vao)
{
	return;
}

void Graphics::bindFramebuffer(int index)
{
	device->SetRenderTarget(0, surface[index]);
	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(128, 128, 128), 1, 0);
	return;
}

void Graphics::DeleteFrameBuffer(unsigned int index)
{
	surface[index]->Release();
	surface.erase(surface.begin() + index);
	return;
}

int Graphics::checkFramebuffer()
{
	return 0;
}

int Graphics::setupFramebuffer(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &depth_tex, int multisample)
{
	IDirect3DSurface9* surf = NULL;

	HRESULT ret = device->CreateRenderTarget(width, height,
		D3DFMT_X8R8G8B8, D3DMULTISAMPLE_NONE, 0, false, &surf, NULL);

	surface.push_back(surf);
	return surface.size() - 1;
}

void Graphics::fbAttachTexture(int fbo)
{
	return;
}

void Graphics::fbAttachDepth(int fbo)
{
	return;
}

void Graphics::GetDebugLog(void)
{
	return;
}

void Graphics::BlendFuncDstColorOne()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
}

void Graphics::BlendFuncDstColorZero()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
}

void Graphics::BlendFuncZeroOneMinusAlpha()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void Graphics::BlendFuncOneAlpha()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTALPHA);
}


void Graphics::BlendFuncOneOneMinusAlpha()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA);
}


void Graphics::BlendFuncOneOne()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
}


void Graphics::BlendFuncZeroSrcColor()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
}

void Graphics::BlendFuncZeroOne()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
}

void Graphics::BlendFuncDstColorOneMinusDstAlpha()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA);
}

void Graphics::BlendFuncDstColorSrcAlpha()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
}

void Graphics::BlendFuncOneMinusSrcAlphaSrcAlpha()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
}

void Graphics::BlendFuncSrcAlphaOneMinusSrcAlpha()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}


void Graphics::BlendFuncOneSrcAlpha()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
}

void Graphics::BlendFuncOneMinusDstColorZero()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
}

void Graphics::BlendFuncDstColorSrcColor()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
}

void Graphics::BlendFuncZeroSrcAlpha()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
}

void Graphics::BlendFuncOneZero()
{
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
}


int Graphics::CreateVertexBuffer(void *vertex_array, int num_verts)
{
	LPDIRECT3DVERTEXBUFFER9 *d3d9_buffer = new LPDIRECT3DVERTEXBUFFER9;
	HRESULT ret;
	void *pVert = NULL;

	ret = device->CreateVertexBuffer(num_verts * sizeof(vertex_t), D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, d3d9_buffer, NULL);
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
	if (handle < 0)
		return;
	device->SetTexture(level, texture[handle]);
}

void Graphics::DeselectTexture(int level)
{
	device->SetTexture(level, NULL);
}

int Graphics::LoadTexture(int width, int height, int components, int format, void *bytes, bool clamp)
{
	IDirect3DTexture9	**d3d9_buffer = new IDirect3DTexture9 *;
	D3DLOCKED_RECT		rect;
	//D3DFMT_A8B8G8R8
	//D3DFMT_A8R8G8B8
	
	if (components == 4)
		device->CreateTexture(width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, d3d9_buffer, NULL);
	else if (components == 3)
		device->CreateTexture(width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_R8G8B8, D3DPOOL_MANAGED, d3d9_buffer, NULL);
	else if (components == 1)
		device->CreateTexture(width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_L8, D3DPOOL_MANAGED, d3d9_buffer, NULL);
		
	(*d3d9_buffer)->LockRect(0, &rect, NULL, 0);
	memcpy((void *)rect.pBits, bytes, width * height * components);
	(*d3d9_buffer)->UnlockRect(0);
	(*d3d9_buffer)->GenerateMipSubLevels();
	texture.push_back(*d3d9_buffer);
	return texture.size() - 1;
}

void Graphics::DeleteTexture(int handle)
{
	if (handle < 0)
		return;

	IDirect3DTexture9	*d3d9_buffer = texture[handle];
//	texture.remove(d3d9_buffer);
	d3d9_buffer->Release();
//	delete d3d9_buffer;
}

int Graphics::CreateCubeMap()
{
	//TBD
	return 0;
}

void SelectCubemap(int texObject)
{
	//TDB
	return;
}


void Graphics::SelectShader(int handle)
{
	device->SetVertexShader(NULL);
	device->SetPixelShader(NULL);
}

Shader::Shader()
{
	vertex_src = NULL;
	geometry_src = NULL;
	fragment_src = NULL;
	gfx = NULL;
}

int Shader::init(Graphics *gfx, char *vertex_file,  char *geometry_file, char *fragment_file)
{
	Shader::gfx = gfx;
	LPD3DXBUFFER err;
	FILE *fLog = fopen("infolog.txt", "a");
	HRESULT ret;

	if (vertex_file)
	{
		LPD3DXBUFFER vertex_binary;
		vertex_src = (char *)get_file(vertex_file, NULL);
		if (vertex_src == NULL)
		{
			fprintf(fLog, "Unable to load vertex shader %s\n", vertex_file);
			fclose(fLog);
			return -1;
		}

		D3DXCompileShader(vertex_src, strlen(vertex_src) + 1,
			NULL, NULL, "main", "vs_3_0", D3DXSHADER_PACKMATRIX_COLUMNMAJOR | D3DXSHADER_SKIPOPTIMIZATION, &vertex_binary, &err, &uniform_vs);
		if (err)
		{
			fprintf(fLog, "Unable to load vertex shader %s\n%s\n", vertex_file, (char  *)err->GetBufferPointer());
			fclose(fLog);
			return -1;
		}



		D3DXCONSTANTTABLE_DESC constantDesc;

		uniform_vs->GetDesc(&constantDesc);

		printf("%s has %d constants\n", vertex_file, constantDesc.Constants);

		ret = gfx->device->CreateVertexShader((DWORD *)vertex_binary->GetBufferPointer(), &vertex_shader);
		vertex_binary->Release();
		fprintf(fLog, "Loaded vertex shader %s\n", vertex_file);
	}

	if (geometry_file)
	{
		/*
		LPD3DXBUFFER geometry_binary;
		geometry_src = (char *)get_file(geometry_file, NULL);
		if (geometry_src == NULL)
		{
			fprintf(fLog, "Unable to load geometry shader %s\n", fragment_file);
			fclose(fLog);
			return -1;
		}

		D3DXCompileShader(geometry_src, strlen(geometry_src) + 1,
			NULL, NULL, "main", "gs_3_0", 0, &geometry_binary, NULL, &constants);
		device->CreateGeometryShader((DWORD *)geometry_binary->GetBufferPointer(), &geometry_shader);
		geometry_binary->Release();
		*/
	}

	if (fragment_file)
	{
		LPD3DXBUFFER pixel_binary;
		fragment_src = (char *)get_file(fragment_file, NULL);
		if (fragment_src == NULL)
		{
			fprintf(fLog, "Unable to load fragment shader %s\n", fragment_file);
			fclose(fLog);
			return -1;
		}

		D3DXCompileShader(fragment_src, strlen(fragment_src) + 1,
			NULL, NULL, "main", "ps_3_0", D3DXSHADER_PACKMATRIX_COLUMNMAJOR | D3DXSHADER_SKIPOPTIMIZATION, &pixel_binary, &err, &uniform_ps);
		if (err)
		{
			fprintf(fLog, "Unable to load fragment shader %s\n%s\n", fragment_file, (char  *)err->GetBufferPointer());
			fclose(fLog);
			return -1;
		}

		D3DXCONSTANTTABLE_DESC constantDesc;

		uniform_ps->GetDesc(&constantDesc);

		printf("%s has %d constants\n", fragment_file, constantDesc.Constants);


		ret = gfx->device->CreatePixelShader((DWORD *)pixel_binary->GetBufferPointer(), &pixel_shader);
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

#endif