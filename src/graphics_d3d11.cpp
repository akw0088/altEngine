#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef D3D11

void Graphics::resize(int width, int height)
{
	Graphics::width = width;
	Graphics::height = height;

	if (!height)
		height = 1;
	HRESULT ret;

	// Resize the swap chain and recreate the render target view.
	ret = swapchain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	ID3D11Texture2D* backBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	ret = device->CreateRenderTargetView(backBuffer, 0, &render_target);

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ret = device->CreateTexture2D(&depthStencilDesc, 0, &depth_buffer);
	ret = device->CreateDepthStencilView(depth_buffer, 0, &depth_view);


	// Bind the render target view and depth/stencil view to the pipeline.
	context->OMSetRenderTargets(1, &render_target, depth_view);


	// Set the viewport transform.

	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	context->RSSetViewports(1, &viewport);
}

Graphics::Graphics()
{
}


Graphics::~Graphics() 
{
}

void Graphics::cleardepth()
{
}

void Graphics::DepthFunc(char *op)
{
}

bool Graphics::error_check()
{
	return false;
}

void Graphics::Blend(bool flag)
{
}

void Graphics::BlendFunc(char *src, char *dst)
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

void Graphics::StencilOp(char *stencil_fail, char *zfail, char *zpass)
{
}

void Graphics::CullFace(int mode)
{
}

void Graphics::Depth(bool flag)
{
}


void Graphics::init(void *param1, void *param2)
{
	hwnd = *((HWND *)param1);
	hdc = *((HDC *)param2);
	HRESULT		ret;

	DXGI_SWAP_CHAIN_DESC scd;
	memset(&scd, 0, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hwnd;                                // the window to be used
	scd.SampleDesc.Count = 0;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode


	D3D_FEATURE_LEVEL feature_level_out;
	D3D_FEATURE_LEVEL feature_level_in;

	feature_level_in = D3D_FEATURE_LEVEL_11_0;

	ret = D3D11CreateDevice(
		0,                 // default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		0,                 // no software device
		D3D11_CREATE_DEVICE_SINGLETHREADED,
		&feature_level_in,
		1,
		D3D11_SDK_VERSION,
		&device,
		&feature_level_out,
		&context);

	if (FAILED(ret))
	{
		printf("Failed to initialize dx11\n");
		return;
	}

	if (feature_level_out != feature_level_in)
	{
		printf("Hardware doesnt support dx11.1");
		return;
	}

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hwnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;


	IDXGIDevice* dxgiDevice = 0;
	device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

	IDXGIAdapter* dxgiAdapter = 0;
	dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);

	IDXGIFactory* dxgiFactory = 0;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

	dxgiFactory->CreateSwapChain(device, &sd, &swapchain);


	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer);

	ret = device->CreateRenderTargetView(back_buffer, NULL, &render_target);
	back_buffer->Release();

	// set the render target as the back buffer
	context->OMSetRenderTargets(1, &render_target, NULL);

	// Set the viewport
	D3D11_VIEWPORT viewport;
	memset(&viewport, 0, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;

	context->RSSetViewports(1, &viewport);


	D3D11_RASTERIZER_DESC rsDesc;
	memset(&rsDesc, 0, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;
	device->CreateRasterizerState(&rsDesc, &render_state);
	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	device->CreateRasterizerState(&rsDesc, &render_state_wireframe);

	context->RSSetState(render_state);
}

void Graphics::swap()
{
	swapchain->Present(0, 0);
}

void Graphics::clear()
{
	vec4 clear = { 0.5f, 0.5f, 0.5f, 1.0f };

	context->ClearRenderTargetView(render_target, (float *)&clear);
	context->ClearDepthStencilView(depth_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Graphics::destroy()
{
	if (device)
		device->Release();
	if (context)
		context->Release();
	if (swapchain)
		swapchain->Release();
	if (back_buffer)
		back_buffer->Release();
}

int Graphics::CreateIndexBuffer(void *index_array, int num_index)
{
	D3D11_BUFFER_DESC ibd;
	D3D11_SUBRESOURCE_DATA iinitData;

	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(unsigned int) * num_index;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	iinitData.pSysMem = index_array;
	iinitData.SysMemPitch = 0;
	iinitData.SysMemSlicePitch = 0;

	// Create the index buffer.
	ID3D11Buffer *d3d11_buffer;
	device->CreateBuffer(&ibd, &iinitData, &d3d11_buffer);

	index_buffers.push_back(d3d11_buffer);
	return index_buffers.size() - 1;
}

void Graphics::SelectIndexBuffer(int handle)
{
	ID3D11Buffer *d3d11_buffer = index_buffers[handle];

	context->IASetIndexBuffer(d3d11_buffer, DXGI_FORMAT_R32_UINT, 0);
}

void Graphics::DeleteIndexBuffer(int handle)
{

}

void Graphics::DrawArray(primitive_t primitive, int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	if (primitive == PRIM_TRIANGLES)
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	else if (primitive == PRIM_TRIANGLE_STRIP)
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	else if (primitive == PRIM_LINE_STRIP)
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
	else if (primitive == PRIM_POINTS)
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	context->Draw(num_index, start_index);
}

void Graphics::DrawArrayTri(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->Draw(num_index, start_index);
}

void Graphics::DrawArrayTriStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->Draw(num_index, start_index);
}

void Graphics::DrawArrayLineStrip(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ);
	context->Draw(num_index, start_index);
}

void Graphics::DrawArrayPoint(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->Draw(num_index, start_index);
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
	return;
}

void Graphics::DeleteFrameBuffer(unsigned int index)
{
	return;
}

int Graphics::checkFramebuffer()
{
	return 0;
}

int Graphics::setupFramebuffer(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &depth_tex, int multisample)
{
	return 0;
}

int setupFramebufferArray(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &depth_tex, int layer, int multisample)
{
	return 0;
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


int Graphics::CreateVertexBuffer(void *vertex_array, int num_verts)
{
	D3D11_BUFFER_DESC vbd;
	D3D11_SUBRESOURCE_DATA vinitData;
	ID3D11Buffer *d3d11_buffer;

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(vertex_t) * num_verts;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	vinitData.pSysMem = vertex_array;
	vinitData.SysMemPitch = 0;
	vinitData.SysMemSlicePitch = 0;

	device->CreateBuffer(&vbd, &vinitData, &d3d11_buffer);
	vertex_buffers.push_back(d3d11_buffer);
	return vertex_buffers.size() - 1;
}

void Graphics::SelectVertexBuffer(int handle)
{
	ID3D11Buffer *d3d11_buffer = vertex_buffers[handle];

	if (handle != 0)
	{
		unsigned int stride = sizeof(vertex_t);
		unsigned int offset = 0;
		context->IASetVertexBuffers(0, 1, &d3d11_buffer, &stride, &offset);
	}
}

void Graphics::DeleteVertexBuffer(int handle)
{
}

void Graphics::SelectTexture(int level, int handle)
{
}

void Graphics::DeselectTexture(int level)
{
}

int Graphics::LoadTexture(int width, int height, int components, int format, void *bytes, bool clamp)
{
	D3DX11_IMAGE_LOAD_INFO iload;

	ID3D11Texture2D *tex;

	iload.Width = width;
	iload.Height = height;
	iload.Depth = D3DX11_FROM_FILE;
	iload.FirstMipLevel = 0;
	iload.MipLevels = D3DX11_FROM_FILE;
	iload.Usage = D3D11_USAGE_STAGING;
	iload.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	iload.BindFlags = 0;
	iload.CpuAccessFlags = 0;// D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	iload.MiscFlags = 0;
	iload.Filter = 0;
	iload.MipFilter = 0;
	iload.pSrcInfo = 0;


	HRESULT ret = D3DX11CreateTextureFromMemory(device, bytes, width * height * components, &iload, 0, (ID3D11Resource**)&tex, 0);
	texture.push_back(tex);
	return texture.size() - 1;
}

void Graphics::DeleteTexture(int handle)
{
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
}

Shader::Shader()
{
}

int Shader::init(Graphics *gfx, char *vertex_file,  char *geometry_file, char *fragment_file)
{
	Shader::gfx = gfx;
	FILE *fLog = fopen("infolog.txt", "a");
	HRESULT result;
	ID3D10Blob *vertex = 0;
	ID3D10Blob *fragment = 0;

	if (vertex_file)
	{

		ID3D10Blob *infolog = 0;
		WCHAR wfile[512];

		mbstowcs(wfile, vertex_file, strlen(vertex_file));

		result = D3DX11CompileFromFile(vertex_file, 0, 0, "main", "vs_3_0", 0, 0, 0, &vertex, &infolog, 0);
//		result = D3DCompileFromFile(wfile, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
//			"main", "vs_4_0", 0, 0, &vertex, &infolog);

		if (FAILED(result))
		{
			if (infolog != 0)
			{
				fprintf(fLog, "%s\n", (char*)infolog->GetBufferPointer());
				fclose(fLog);
			}
			return -1;
		}

		// Create the vertex shader from the buffer.
		result = gfx->device->CreateVertexShader(vertex->GetBufferPointer(), vertex->GetBufferSize(), NULL, &vertex_shader);
		if (FAILED(result))
		{
			fclose(fLog);
			return -1;
		}

		gfx->context->VSSetShader(vertex_shader, 0, 0);
	}

	if (fragment_file)
	{
		ID3D10Blob *infolog = 0;
		WCHAR wfile[512];

		mbstowcs(wfile, fragment_file, strlen(fragment_file));

		result = D3DX11CompileFromFile(fragment_file, 0, 0, "main", "ps_3_0", 0, 0, 0, &fragment, &infolog, 0);
//		result = D3DCompileFromFile(wfile, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
//			"main", "ps_4_0", 0, 0, &fragment, &infolog);

		if (FAILED(result))
		{
			if (infolog != 0)
			{
				fprintf(fLog, "%s\n", (char*)infolog->GetBufferPointer());
				fclose(fLog);
			}
			return -1;
		}

		// Create the pixel shader from the buffer.
		result = gfx->device->CreatePixelShader(fragment->GetBufferPointer(), fragment->GetBufferSize(), NULL, &fragment_shader);
		if (FAILED(result))
		{
			fclose(fLog);
			return -1;
		}

		gfx->context->PSSetShader(fragment_shader, 0, 0);
	}

	D3D11_INPUT_ELEMENT_DESC vertex_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_UINT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	gfx->device->CreateInputLayout(vertex_desc, 6, vertex->GetBufferPointer(), vertex->GetBufferSize(), &layout);
	gfx->context->IASetInputLayout(layout);

	fclose(fLog);
	return 0;
}

void Shader::Select()
{
	gfx->context->VSSetShader(vertex_shader, 0, 0);
	gfx->context->PSSetShader(fragment_shader, 0, 0);
}

void Shader::destroy()
{
}

Shader::~Shader()
{
	destroy();
}
#endif