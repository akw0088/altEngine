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



	~Graphics();

#ifdef VULKAN
private:
	VkInstance  CreateInstance();
	void CreateDeviceAndQueue(VkInstance instance, VkDevice* outputDevice, VkQueue* outputQueue, int* outputQueueIndex, VkPhysicalDevice* outputPhysicalDevice);
	void FindPhysicalDeviceWithGraphicsQueue(const vector<VkPhysicalDevice>& physicalDevices, VkPhysicalDevice* outputDevice, int* outputGraphicsQueueIndex);
	void CreateSwapchainImageViews(VkDevice device, VkFormat format, const int count, const VkImage* images, VkImageView* imageViews);
	void CreateFramebuffers(VkDevice device, VkRenderPass renderPass, const int width, const int height, const int count, const VkImageView* imageViews, VkFramebuffer* framebuffers);
	void CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, const int surfaceWidth, const int surfaceHeight, const int backbufferCount, VkFormat* swapchainFormat, VkSwapchainKHR &swapchain);
	void GetSwapchainFormatAndColorspace(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapchainFormatColorSpace &result);

	void AllocateBuffer(VkDevice device, const int size, const VkBufferUsageFlagBits bits, VkBuffer &buffer);
	int AllocateMemory(VkDeviceMemory &deviceMemory, const vector<MemoryTypeInfo>& memoryInfos, VkDevice device, const int size, const unsigned int memoryBits, unsigned int memoryProperties, bool* isHostCoherent = nullptr);

	void CreateSurface(VkInstance instance, HWND hwnd, VkSurfaceKHR &surface);
	void CreateTexture(int width, int height, int components, int format, unsigned char *image_data, bool clamp);
	void CreateSampler();
	void CreateDescriptors();
	void LoadShader(VkDevice device, const void* shaderContents, const unsigned int size, VkShaderModule &shader);

	void render_cmdbuffer(VkCommandBuffer commandBuffer, int width, int height);

	void CreatePipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout, VkShaderModule vertexShader, VkShaderModule fragmentShader, VkPipeline &pipeline);
	void CreatePipelineStateObject();
	void CreateMeshBuffers(VkCommandBuffer uploadCommandBuffer, vertex_t *vertices, int num_vertex, unsigned int *indices, int num_index);
	void CreateRenderPass(VkDevice device, VkFormat swapchainFormat, VkRenderPass &rp);
	void SetupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT &fp);

	VkCommandBuffer v_uploadCommandBuffer;
	VkCommandBuffer i_uploadCommandBuffer;
	vector<MemoryTypeInfo> EnumerateHeaps(VkPhysicalDevice device);
public:
	void render();
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
	#endif
	#ifdef __linux__
        int screen;
        Visual *vis;
        int depth;
        GC gc;
	XImage *image;

	#endif
	POINT	center;
	int	*pixels;
	float *zbuffer;

	vertex_t *vertex_array[4096];
	int *index_array[4096];
	texinfo_t texture_array[8192];
	int num_index_array;
	int num_vertex_array;
	int num_texture_array;
	int current_vbo;
	int current_ibo;
	int current_tex;
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


	VkSemaphore renderingCompleteSemaphore;
	VkSemaphore imageAcquiredSemaphore;
	VkDebugReportCallbackEXT callback;


	VkDeviceMemory deviceMemory_ = VK_NULL_HANDLE;
	VkBuffer vertexBuffer_ = VK_NULL_HANDLE;
	VkBuffer indexBuffer_ = VK_NULL_HANDLE;

	VkDeviceMemory uploadMemory_ = VK_NULL_HANDLE;
	VkBuffer uploadBuffer_ = VK_NULL_HANDLE;

	VkShaderModule vertexShader_ = VK_NULL_HANDLE;
	VkShaderModule fragmentShader_ = VK_NULL_HANDLE;

	VkPipeline pipeline_ = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;

	VkDeviceMemory deviceImageMemory_ = VK_NULL_HANDLE;
	VkImage Image_ = VK_NULL_HANDLE;
	VkImageView ImageView_ = VK_NULL_HANDLE;

	VkDeviceMemory uploadImageMemory_ = VK_NULL_HANDLE;
	VkBuffer uploadImageBuffer_ = VK_NULL_HANDLE;

	VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;
	VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout_ = VK_NULL_HANDLE;

	VkSampler sampler_ = VK_NULL_HANDLE;

	VkViewport viewport_;

	VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkQueue queue_ = VK_NULL_HANDLE;

	VkSurfaceKHR surface_ = VK_NULL_HANDLE;

	VkFence frameFences_[QUEUE_SLOT_COUNT];
	VkImage swapchainImages_[QUEUE_SLOT_COUNT];
	VkImageView swapChainImageViews_[QUEUE_SLOT_COUNT];
	VkFramebuffer framebuffer_[QUEUE_SLOT_COUNT];

	VkRenderPass renderPass_ = VK_NULL_HANDLE;

	int queueFamilyIndex_ = -1;

	VkCommandPool commandPool_;
	VkCommandBuffer commandBuffers_[QUEUE_SLOT_COUNT];
	VkCommandBuffer i_commandBuffers_[QUEUE_SLOT_COUNT];
	VkCommandBuffer v_commandBuffers_[QUEUE_SLOT_COUNT];
	VkCommandBuffer setupCommandBuffer_;
	uint32_t currentBackBuffer_ = 0;
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
