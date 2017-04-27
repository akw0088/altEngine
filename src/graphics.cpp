#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define ERROR_CHECK

void Graphics::resize(int width, int height)
{
#ifndef DEDICATED
	Graphics::width = width;
	Graphics::height = height;

	if (!height)
		height = 1;
#ifndef DIRECTX
	glViewport(0, 0, width, height);
#else
#ifdef D3D11
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;

	d3d->RSSetViewports(1, &viewport);
#else
	D3DVIEWPORT9 viewport;

	viewport.Width = width;
	viewport.Height = height;
	viewport.MinZ = -1.0f;
	viewport.MaxZ = 1.0f;
	viewport.X = 0;
	viewport.Y = 0;

	HRESULT ret = device->SetViewport(&viewport);
	if (ret != D3D_OK)
	{
		printf("resize error!!!\n");
	}
#endif
#endif
#endif
}

Graphics::Graphics()
{
}


#ifndef DEDICATED
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

#ifdef D3D11
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hwnd;                                // the window to be used
	scd.SampleDesc.Count = 0;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapchain,
		&device,
		NULL,
		&d3d);

	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	device->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	// set the render target as the back buffer
	d3d->OMSetRenderTargets(1, &backbuffer, NULL);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;

	d3d->RSSetViewports(1, &viewport);


	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//		{ 0, sizeof(vec3), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		//		{ 0, sizeof(vec3) + sizeof(vec2), D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	device->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
	d3d->IASetInputLayout(pLayout);
#else
	memset(&d3dpp, sizeof(d3dpp), 0);
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.hDeviceWindow = hwnd;

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
	
	device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);

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
	D3DXCreateFont( device, 8, 0, FW_THIN, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "System", &font );


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
#endif
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
#ifdef D3D11
	swapchain->Present(0, 0);
#else
	device->EndScene();
	device->Present(NULL, NULL, NULL, NULL);
#endif
}

void Graphics::clear()
{
#ifdef D3D11
	d3d->ClearRenderTargetView(backbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));
#else
	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(128, 128, 128), 1.0f, 0);
	device->BeginScene();
#endif
}

void Graphics::destroy()
{
#ifdef D3D11
	if (device)
		device->Release();
	if (d3d)
		d3d->Release();
	if (swapchain)
		swapchain->Release();
	if (backbuffer)
		backbuffer->Release();
#else
	if (device)
		device->Release();
	if (d3d)
		d3d->Release();
#endif

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

int Graphics::setupFramebuffer(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &depth_tex)
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
#ifdef D3D11
	Shader::gfx = gfx;
	FILE *fLog = fopen("infolog.txt", "a");

	if (vertex_file)
	{
		ID3D10Blob *VS;

		D3DX11CompileFromFile(vertex_file, 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, 0, 0);
		gfx->device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &(gfx->pVS));
		gfx->d3d->VSSetShader(gfx->pVS, 0, 0);
	}

	if (fragment_file)
	{
		ID3D10Blob *PS;

		D3DX11CompileFromFile(fragment_file, 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, 0, 0);
		gfx->device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &(gfx->pPS));
		gfx->d3d->PSSetShader(gfx->pPS, 0, 0);
	}
#else

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
			NULL, NULL, "main", "vs_3_0", D3DXSHADER_PACKMATRIX_COLUMNMAJOR, &vertex_binary, &err, &uniform_vs);
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
		fragment_src = (char *)get_file(fragment_file, NULL);
		if (fragment_src == NULL)
		{
			fprintf(fLog, "Unable to load fragment shader %s\n", fragment_file);
			fclose(fLog);
			return -1;
		}

		D3DXCompileShader(fragment_src, strlen(fragment_src) + 1,
			NULL, NULL, "main", "ps_3_0", D3DXSHADER_PACKMATRIX_COLUMNMAJOR, &pixel_binary, &err, &uniform_ps);
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
#endif
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

#ifndef DIRECTX
	if (vertex_src)
		delete[] vertex_src;
	if (fragment_src)
		delete[] fragment_src;
#endif
}

Shader::~Shader()
{
	destroy();
}
#endif


#ifdef OPENGL
/*====================================================================================
	Opengl support while still easy to cheat and use functions directly for rapid dev
======================================================================================*/
Graphics::~Graphics()
{
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
	memset(&gpustat, 0, sizeof(gpustat_t));
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearStencil(0);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(10.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
//	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	float bias = -3.0f;
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);

#ifdef ERROR_CHECK
	error_check();
#endif
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

void Graphics::cleardepth()
{
	glClear( GL_DEPTH_BUFFER_BIT );

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


void Graphics::BlendFunc(char *src, char *dst)
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
/*
	if (font)
		XUnloadFont(display, font->fid);
	glDeleteLists(1000, '~' - ' ');
*/
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

int Graphics::CreateVertexBuffer(void *vertex_buffer, int num_vertex)
{
	unsigned int	vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// This is cached into the current vbo
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2)));
	glVertexAttribPointer(4, 1, GL_INT,   GL_FALSE,	sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2) + sizeof(vec3)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,	sizeof(vertex_t), (void *)(sizeof(vec3) + sizeof(vec2) + sizeof(vec2) + sizeof(vec3) + sizeof(int)));

	glBufferData(GL_ARRAY_BUFFER, num_vertex * sizeof(vertex_t), vertex_buffer, GL_STATIC_DRAW);

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

int Graphics::LoadTexture(int width, int height, int components, int format, void *bytes, bool clamp)
{
	unsigned int texObject = -1;

	glGenTextures(1, &texObject);
	glBindTexture(GL_TEXTURE_2D, texObject);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, format, GL_UNSIGNED_BYTE, bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

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
	if (mode == 0)
		glCullFace(GL_BACK);
	else if (mode == 1)
		glCullFace(GL_FRONT);
	else if (mode == 2)
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

void Graphics::StencilFunc(char *op, int ref, int mask)
{
	if (strcmp(op, "always") == 0)
		glStencilFunc(GL_ALWAYS, ref, mask);
	else if (strcmp(op, "equal") == 0)
		glStencilFunc(GL_EQUAL, ref, mask);

#ifdef ERROR_CHECK
	error_check();
#endif

}

void Graphics::DepthFunc(char *op)
{
	if (strcmp(op, "<=") == 0)
		glDepthFunc(GL_LEQUAL);
	else if (strcmp(op, "<") == 0)
		glDepthFunc(GL_LESS);

#ifdef ERROR_CHECK
	error_check();
#endif

}

void Graphics::StencilOp(char *stencil_fail, char *zfail, char *zpass)
{
	if (strcmp(zpass, "incr") == 0)
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	else if (strcmp(zpass, "decr") == 0)
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	else if (strcmp(zpass, "keep") == 0)
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

#ifdef ERROR_CHECK
	error_check();
#endif

}

Shader::Shader()
{
	vertex_src = NULL;
	geometry_src = NULL;
	fragment_src = NULL;
}


int Shader::init(Graphics *gfx, char *vertex_file, char *geometry_file, char *fragment_file)
{
	FILE		*fLog;
	int			success;
	int			max_attrib = 0;

	fLog = fopen("infolog.txt", "a");
	fprintf(fLog, "OpenGL Version %s\n", glGetString(GL_VERSION));
	fprintf(fLog, "OpenGL Renderer %s\n", glGetString(GL_RENDERER));
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attrib);
	fprintf(fLog, "Max vertex attribs %d\n", max_attrib);

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

void Graphics::fbAttachDepth(int texObj)
{
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texObj, 0);
}

void Graphics::bindFramebuffer(int fbo)
{
	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	if (fbo)
	{
		glDrawBuffers(1, &attachments[0]);
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

int Graphics::setupFramebuffer(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &depth_tex)
{
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//glGenRenderbuffers(1, &rbo);
	//glGenRenderbuffers(1, &depth);

	//glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 1024, 1024);

	//glBindRenderbuffer(GL_RENDERBUFFER, depth);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);

	//glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
	//glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, quad_tex, 0);

	glGenTextures(1, &quad_tex);
	glBindTexture(GL_TEXTURE_2D, quad_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &depth_tex);
	glBindTexture(GL_TEXTURE_2D, depth_tex);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Linear seems to work too
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, quad_tex, 0);
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);

	if (checkFramebuffer() != 0)
	{
		return -1;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	return 0;
}

void Graphics::DeleteFrameBuffer(unsigned int fbo)
{
	glDeleteFramebuffers(1, &fbo);
}

void Graphics::GetDebugLog()
{
#ifdef WIN32
	GLint maxMsgLen = 0;
	glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH, &maxMsgLen);

	char buffer[4096] = { 0 };
	GLenum	source[1000];
	GLenum	type[1000];
	GLenum	severity[1000];
	GLuint	id[1000];
	GLsizei	length[1000];


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
#endif

#ifdef DEDICATED
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
