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

#ifndef GRAPHICS_H
#define GRAPHICS_H

//#define ERROR_CHECK


#define FRONTFACE	0x0404
#define BACKFACE	0x0405
#define NONE		2

#define LESS		0x0201
#define EQUAL		0x0202
#define LEQUAL		0x0203
#define GREATER		0x0204
#define NOTEQUAL	0x0205
#define GEQUAL		0x0206
#define ALWAYS		0x0207

#define NEVER		0x0200

#define KEEP		0x1E00
#define REPLACE		0x1E01
#define INCR		0x1E02
#define DECR		0x1E03
#define INCR_WRAP	0x8507
#define DECR_WRAP	0x8508


#ifdef VULKAN
struct MemoryTypeInfo
{
	bool deviceLocal = false;
	bool hostVisible = false;
	bool hostCoherent = false;
	bool hostCached = false;
	bool lazilyAllocated = false;

	struct Heap
	{
		uint64_t size = 0;
		bool deviceLocal = false;
	};

	Heap heap;
	int index;
};

enum MemoryProperties
{
	MT_DeviceLocal = 1,
	MT_HostVisible = 2
};

struct SwapchainFormatColorSpace
{
	VkFormat format;
	VkColorSpaceKHR colorSpace;
};

template <typename T>
T RoundToNextMultiple(const T a, const T multiple)
{
	return ((a + multiple - 1) / multiple) * multiple;
}
#endif


/*
	You could make this a pure virtual class and switch between OpenGL and Direct3D at runtime...
	But then you would need to link to both d3d9 dll's and opengl32 dll's at the same time
	(not to mention the whole vtable lookup ontop of a wrapper function)
	I think a d3d binary and an opengl binary is the better approach
	(Although the idea of making a no operation graphics stub is nice for headless server)
*/
class Graphics
{
public:
	Graphics();
	void init(void *param1, void *param2);
	bool error_check();
	void clear();
	void clear_color(vec3 &color);
	void clear_multi();
	void cleardepth();
	void clearstencil();
	void Depth(bool flag);
	void Blend(bool flag);
	void BlendFuncDstColorOne();
	void BlendFuncDstColorZero();
	void BlendFuncOneOne();
	void BlendFuncZeroOne();
	void BlendFuncOneZero();
	void BlendFuncZeroOneMinusAlpha();
	void BlendFuncOneOneMinusAlpha();
	void BlendFuncDstColorOneMinusDstAlpha();
	void BlendFuncDstColorSrcAlpha();
	void BlendFuncOneMinusSrcAlphaSrcAlpha();
	void BlendFuncSrcAlphaOneMinusSrcAlpha();
	void BlendFuncOneAlpha();
	void BlendFuncZeroSrcColor();
	void BlendFuncDstColorSrcColor();
	void BlendFuncZeroSrcAlpha();
	void BlendFuncOneMinusDstColorZero();
	void BlendFuncOneSrcAlpha();
	void CullFace(int mode);
	void Color(bool flag);
	void DepthFunc(int op);
	void Stencil(bool flag);
	void StencilFunc(int op, int ref, unsigned int mask);
	void StencilOp(int stencil_fail, int zfail, int zpass);
	void TwoSidedStencilOp(int face, int stencil_fail, int zfail, int zpass);
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

	int CreateVertexBuffer(void *vertex_buffer, int num_vertex, bool dynamic = false);
	
	void SelectVertexBuffer(int handle);
	void DeleteVertexBuffer(int handle);

	void SelectTexture(int level, int index);
	void SelectTextureArray(int level, int texObject);
	void DeselectTexture(int level);
	int CreateCubeMap();
	void SelectCubemap(int texObject);
	int LoadTexture(int width, int height, int components, int format, void *bytes, bool clamp, int anisotropic);
	int LoadTextureArray(int width, int height, int components, int format, void **bytes, bool clamp, int num_layer);
	void DeleteTexture(int handle);

	void SelectShader(int program);

	int checkFramebuffer();
	int CreateFramebuffer(int width, int height, unsigned int &fbo, unsigned int &quad_tex, unsigned int &depth_tex, unsigned int &normal_depth, int multisample, bool twoattach);
	void DeleteFrameBuffer(int fbo, int quad, int depth);
	void fbAttachTexture(int texObj);
	void fbAttachTextureOne(int texObj);
	void fbAttachDepth(int texObj);
	void bindFramebuffer(int fbo, int num_attach = 1);
	void GetDebugLog();

#ifdef SOFTWARE
	void clip(int value);
#endif



	~Graphics();

#ifdef VULKAN
private:
	VkInstance  CreateInstance();
	void CreateDeviceAndQueue(VkInstance instance, VkDevice &outputDevice, VkQueue &outputQueue, int &outputQueueIndex, VkPhysicalDevice &outputPhysicalDevice);
	void CreateSwapchainImageViews(VkDevice device, VkFormat format, const int count, const VkImage* images, VkImageView* imageViews);
	void CreateFramebuffers(VkDevice device, VkRenderPass renderPass, const int width, const int height, const int count, const VkImageView* imageViews, VkFramebuffer* framebuffers);
	void CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, const int surfaceWidth, const int surfaceHeight, const int backbufferCount, VkFormat &swapchainFormat, VkSwapchainKHR &swapchain);

	void AllocateBuffer(VkDevice device, const int size, const VkBufferUsageFlagBits bits, VkBuffer &buffer);
	int AllocateMemory(VkDeviceMemory &deviceMemory, const vector<MemoryTypeInfo>& memoryInfos, VkDevice device, const int size, const unsigned int memoryBits, unsigned int memoryProperties, bool* isHostCoherent = nullptr);

	void CreateWin32Surface(VkInstance instance, HWND hwnd, VkSurfaceKHR &surface);
	void CreateTexture(int width, int height, int components, int format, unsigned char *image_data, bool clamp);
	void CreateSampler();
	void CreateDescriptors();
	void LoadShader(VkDevice device, const void* shaderContents, const unsigned int size, VkShaderModule &shader);

	void CreatePipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout, VkShaderModule vertexShader, VkShaderModule fragmentShader, VkPipeline &pipeline);
	void LoadCommandBuffer(VkCommandBuffer &cmd_buffer);
	int LoadShadersAndCreatePipelineStateObject(char *vertex_shader_file, char *fragment_shader_file);
	void CreateMeshBuffers(VkCommandBuffer uploadCommandBuffer, vertex_t *vertices, int num_vertex, unsigned int *indices, int num_index);
	void CreateRenderPass(VkDevice device, VkFormat swapchainFormat, VkRenderPass &rp);
	void SetupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT &fp);

	void UniformSetup(matrix4 &mvp);
	void CreateVertexBufferObject(vertex_t *vertex_array, int num_vert);

	VkCommandBuffer v_uploadCommandBuffer;
	VkCommandBuffer i_uploadCommandBuffer;
	vector<MemoryTypeInfo> EnumerateHeaps(VkPhysicalDevice device);
#endif

public:
	int width, height;
	gpustat_t gpustat;
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
#ifdef SOFTWARE
	#ifdef WIN32
	HDC	hdcMem;
	HBITMAP	hBitmap;
	HGDIOBJ hObject;
	BITMAP	bitmap;
	DWORD tid[16];
	HANDLE thread_handle[16];
	#endif
	#ifdef __linux__
        int screen;
        Visual *vis;
        int depth;
        GC gc;
	XImage *image;
	pthread_t tid[16];
	#endif
	vec2	center;
	int	*pixels;
	int clip_enabled;
#ifdef THREAD
	int	*pixel[16];
	float *zbuff[16];
#endif
	float *zbuffer;

	vertex_t *vertex_array[4096];
	int *index_array[4096];
	vector <texinfo_t> texture_array;
	int num_index_array;
	int num_vertex_array;
	int num_texture_array;
	int current_vbo;
	int current_ibo;
	int current_tex;
	int lightmap_tex;
	static matrix4 current_mvp;

#endif

#ifdef DIRECTX
#ifdef D3D11
	ID3D11Device			*device;
	ID3D11DeviceContext		*context;
	IDXGISwapChain			*swapchain;
	ID3D11Texture2D			*back_buffer;
	ID3D11Texture2D			*depth_buffer;
	ID3D11RenderTargetView	*render_target;
	ID3D11DepthStencilView	*depth_view;



	vector<ID3D11Buffer *>	vertex_buffers;
	vector<ID3D11Buffer *> index_buffers;
	//vector<ID3D11Texture2D *> texture;
	vector<ID3D11ShaderResourceView *> texture;

	ID3D11RasterizerState *render_state;
	ID3D11RasterizerState *render_state_wireframe;

#else
	IDirect3D9				*d3d;
	IDirect3DDevice9		*device;
	D3DPRESENT_PARAMETERS	d3dpp;
	LPDIRECT3DVERTEXDECLARATION9		vertex_decl;
	vector<IDirect3DVertexBuffer9 *>	vertex_buffers;
	vector<IDirect3DIndexBuffer9 *> index_buffers;
	vector<IDirect3DTexture9 *> texture;
	vector<IDirect3DSurface9 *> surface;
	unsigned char clear_r;
	unsigned char clear_g;
	unsigned char clear_b;
//	ID3DXFont *font;
#endif
#endif

#ifdef VULKAN
	static const int QUEUE_SLOT_COUNT = 3;

	VkInstance vk_instance = VK_NULL_HANDLE;
	VkDevice vk_device = VK_NULL_HANDLE;

	VkViewport vk_viewport;
	VkSwapchainKHR vk_swapchain = VK_NULL_HANDLE;
	VkPhysicalDevice vk_physical = VK_NULL_HANDLE;
	VkQueue vk_queue = VK_NULL_HANDLE;
	VkFramebuffer vk_framebuffer[QUEUE_SLOT_COUNT];
	VkRenderPass vk_renderpass = VK_NULL_HANDLE;
	VkSampler vk_sampler = VK_NULL_HANDLE;
	VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
	int vk_queueFamilyIndex = -1;

	VkFence vk_frameFences[QUEUE_SLOT_COUNT];
	VkImage vk_swapchainImages[QUEUE_SLOT_COUNT];
	VkImageView vk_swapChainImageViews[QUEUE_SLOT_COUNT];
	VkCommandPool vk_commandPool;
	VkCommandBuffer vk_cmd_buffer_array[QUEUE_SLOT_COUNT];
	uint32_t vk_currentBackBuffer = 0;


	VkSemaphore vk_renderingCompleteSemaphore;
	VkSemaphore vk_imageAcquiredSemaphore;
	VkDebugReportCallbackEXT vk_callback;

	VkDeviceMemory vk_deviceMemory = VK_NULL_HANDLE;

	VkBuffer vk_vertexBuffer = VK_NULL_HANDLE;
	VkBuffer vk_indexBuffer = VK_NULL_HANDLE;
	VkBuffer vk_uploadBuffer = VK_NULL_HANDLE; // for vertex buffer
	VkBuffer vk_uploadImageBuffer = VK_NULL_HANDLE; // for images

	VkDeviceMemory vk_uploadMemory = VK_NULL_HANDLE;


	VkShaderModule vk_vertexShader = VK_NULL_HANDLE;
	VkShaderModule vk_fragmentShader = VK_NULL_HANDLE;

	VkPipeline vk_pipeline = VK_NULL_HANDLE;
	VkPipelineLayout vk_pipelineLayout = VK_NULL_HANDLE;

	VkDeviceMemory vk_deviceImageMemory = VK_NULL_HANDLE;
	VkImage vk_Image = VK_NULL_HANDLE;
	VkImageView vk_ImageView = VK_NULL_HANDLE;

	VkDeviceMemory vk_uploadImageMemory = VK_NULL_HANDLE;

	VkDescriptorPool vk_descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet vk_descriptorSet = VK_NULL_HANDLE;
	VkDescriptorSetLayout vk_descriptorSetLayout = VK_NULL_HANDLE;



	bool initialized = false;
	bool initialized_once = false;
#endif
};

class Shader
{
public:
	Shader();

	int init(Graphics *gfx, char *vertex_file,  char *geometry_file, char *fragment_file);
	virtual void Select();
	void destroy();
	virtual ~Shader();
	virtual void prelink() = 0;
protected:
	const char	*vertex_src, *geometry_src, *fragment_src;
#ifdef DIRECTX
#ifdef D3D11
	Graphics *gfx;

	ID3D11InputLayout *layout;
	ID3D11VertexShader *vertex_shader;
	ID3D11PixelShader *fragment_shader;
#else
	Graphics *gfx;
	IDirect3DVertexShader9		*vertex_shader;
	IDirect3DPixelShader9		*pixel_shader;
//	IDirect3DGeometryShader9	*geometry_shader;
	LPD3DXCONSTANTTABLE		uniform_vs;
	LPD3DXCONSTANTTABLE		uniform_ps;
#endif
#else
	int		program_handle;
	int		vertex_handle;
	int		geometry_handle;
	int		fragment_handle;
#endif
};




#endif
