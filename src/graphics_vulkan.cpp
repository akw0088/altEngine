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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "assert.h"

#ifdef VULKAN


PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = VK_NULL_HANDLE;



void Graphics::resize(int width, int height)
{
	Graphics::width = width;
	Graphics::height = height;


	/*
	The framebuffers need to be reconstructed after a resize event,
	but the render pass doesn't have to be reconstructed.
	This means it's not necessary to rebuild the pipeline.
	*/

	// Wait for all rendering to finish
	vkWaitForFences(vk_device, 3, vk_frameFences, VK_TRUE, UINT64_MAX);

	vkDestroySwapchainKHR(vk_device, vk_swapchain, NULL);
	vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);


#ifdef WIN32
	CreateWin32Surface(vk_instance, hwnd, vk_surface);
#endif


	VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
	CreateSwapchain(vk_physical, vk_device, vk_surface, width, height, QUEUE_SLOT_COUNT, swapchainFormat, vk_swapchain);

	unsigned int swapchainImageCount = 0; // three swap buffers
	vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &swapchainImageCount, NULL);

	vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &swapchainImageCount, vk_swapchainImages);

	CreateSwapchainImageViews(vk_device, swapchainFormat, QUEUE_SLOT_COUNT, vk_swapchainImages, vk_swapChainImageViews);
	CreateFramebuffers(vk_device, vk_renderpass, width, height, QUEUE_SLOT_COUNT, vk_swapChainImageViews, vk_framebuffer);


	// partial swap to prepare back buffer
	vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, vk_imageAcquiredSemaphore, VK_NULL_HANDLE, &vk_currentBackBuffer);
	vkWaitForFences(vk_device, 1, &vk_frameFences[vk_currentBackBuffer], VK_TRUE, UINT64_MAX);
	vkResetFences(vk_device, 1, &vk_frameFences[vk_currentBackBuffer]);


	DrawArrayTri(0, 0, 6, 6);
	swap();
}

Graphics::Graphics()
{
}


Graphics::~Graphics()
{
}


VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT  bjectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	printf("%s\n", pMessage);
#ifdef WIN32
	MessageBox((HWND)pUserData, pMessage, "Vulkan Error", 0);
#endif
	return VK_FALSE;
}


void Graphics::SetupDebugCallback(VkInstance instance, VkDebugReportCallbackEXT &fp)
{
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;


	memset(&callbackCreateInfo, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callbackCreateInfo.flags =
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = &DebugReportCallback;
	callbackCreateInfo.pUserData = hwnd;

	CreateDebugReportCallback(instance, &callbackCreateInfo, NULL, &fp);
}


void Graphics::CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, const int surfaceWidth, const int surfaceHeight, const int backbufferCount, VkFormat &swapchainFormat, VkSwapchainKHR &swapchain)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

	unsigned int presentModeCount; // 3 preset modes
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);


	VkPresentModeKHR presentModes[4];

	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);

	VkExtent2D swapChainSize = {};
	swapChainSize = surfaceCapabilities.currentExtent;

	unsigned int swapChainImageCount = backbufferCount;


	VkSurfaceTransformFlagBitsKHR surfaceTransformFlags;

	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		surfaceTransformFlags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		surfaceTransformFlags = surfaceCapabilities.currentTransform;
	}

	SwapchainFormatColorSpace swapchainFormatColorSpace;

	unsigned int surfaceFormatCount = 0;

	// two formats
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, NULL);
	if (surfaceFormatCount == 0)
	{
		printf("Error: No surface formats\r\n");
		return;
	}

	VkSurfaceFormatKHR surfaceFormats[2];

	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats);

	if (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		swapchainFormatColorSpace.format = VK_FORMAT_R8G8B8A8_UNORM;
	}
	swapchainFormatColorSpace.format = surfaceFormats[0].format;
	swapchainFormatColorSpace.colorSpace = surfaceFormats[0].colorSpace;


	VkSwapchainCreateInfoKHR swapchainCreateInfo;

	memset(&swapchainCreateInfo, 0, sizeof(VkSwapchainCreateInfoKHR));
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = swapChainImageCount;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform = surfaceTransformFlags;
	swapchainCreateInfo.imageColorSpace = swapchainFormatColorSpace.colorSpace;
	swapchainCreateInfo.imageFormat = swapchainFormatColorSpace.format;
	swapchainCreateInfo.pQueueFamilyIndices = NULL;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.imageExtent = swapChainSize;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;

	vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &swapchain);

	swapchainFormat = swapchainFormatColorSpace.format;
}

void Graphics::AllocateBuffer(VkDevice device, const int size, const VkBufferUsageFlagBits bits, VkBuffer &buffer)
{
	VkBufferCreateInfo bufferCreateInfo;


	memset(&bufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = (unsigned int)size;
	bufferCreateInfo.usage = bits;

	VkResult err = vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer);
	assert(!err);
}



/*
Wrapper function, loops multiple gpu memory allocations
*/
int Graphics::AllocateMemory(VkDeviceMemory &deviceMemory, const vector<MemoryTypeInfo>& memoryInfos, VkDevice device, const int size, const unsigned int memoryBits, unsigned int memoryProperties, bool* isHostCoherent)
{
	for (auto& memoryInfo : memoryInfos)
	{
		if (((1 << memoryInfo.index) & memoryBits) == 0)
		{
			continue;
		}

		if ((memoryProperties & MT_DeviceLocal) && !memoryInfo.deviceLocal)
		{
			continue;
		}

		if ((memoryProperties & MT_HostVisible) && !memoryInfo.hostVisible)
		{
			continue;
		}

		if (isHostCoherent)
		{
			*isHostCoherent = memoryInfo.hostCoherent;
		}

		VkMemoryAllocateInfo memoryAllocateInfo = {};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.memoryTypeIndex = memoryInfo.index;
		memoryAllocateInfo.allocationSize = size;

		vkAllocateMemory(device, &memoryAllocateInfo, NULL, &deviceMemory);
		return 0;
	}

	return -1;
}



/*
Checks GPU memory sizes
*/
vector<MemoryTypeInfo> Graphics::EnumerateHeaps(VkPhysicalDevice device)
{
	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

	vector<MemoryTypeInfo::Heap> heaps;

	// two heaps
	for (unsigned int i = 0; i < memoryProperties.memoryHeapCount; ++i)
	{
		MemoryTypeInfo::Heap info;
		info.size = memoryProperties.memoryHeaps[i].size;
		info.deviceLocal = (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0;

		heaps.push_back(info);
	}

	vector<MemoryTypeInfo> result;


	// 11 types
	for (unsigned int i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		MemoryTypeInfo typeInfo;

		typeInfo.deviceLocal = (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0;
		typeInfo.hostVisible = (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
		typeInfo.hostCoherent = (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0;
		typeInfo.hostCached = (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0;
		typeInfo.lazilyAllocated = (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0;

		typeInfo.heap = heaps[memoryProperties.memoryTypes[i].heapIndex];

		typeInfo.index = static_cast<int> (i);

		result.push_back(typeInfo);
	}

	return result;
}




/*
creates texture sampler (sets filtering, mipmaps?)
*/
void Graphics::CreateSampler()
{
	VkSamplerCreateInfo samplerCreateInfo;

	memset(&samplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.maxAnisotropy = 1.0f;

	vkCreateSampler(vk_device, &samplerCreateInfo, NULL, &vk_sampler);
}

/*
Sets up GPU texture buffer, copies data
*/
void Graphics::CreateTexture(int width, int height, int components, int format, unsigned char *image_data, bool clamp)
{
	int image_size = width * height * components * sizeof(char);


	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = NULL;
	imageCreateInfo.queueFamilyIndexCount = 1;

	unsigned int queueFamilyIndex = (unsigned int) (vk_queueFamilyIndex);
	imageCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	vkCreateImage(vk_device, &imageCreateInfo, NULL, &vk_Image);

	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements(vk_device, vk_Image, &requirements);

	VkDeviceSize requiredSizeForImage = requirements.size;

	vector<MemoryTypeInfo> memoryHeaps = EnumerateHeaps(vk_physical);
	AllocateMemory(vk_deviceImageMemory, memoryHeaps, vk_device, static_cast<int> (requiredSizeForImage), requirements.memoryTypeBits, MT_DeviceLocal);

	vkBindImageMemory(vk_device, vk_Image, vk_deviceImageMemory, 0);

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	bufferCreateInfo.queueFamilyIndexCount = 1;
	bufferCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;
	bufferCreateInfo.size = requiredSizeForImage;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VkResult err = vkCreateBuffer(vk_device, &bufferCreateInfo, NULL, &vk_uploadImageBuffer);
	assert(!err);

	vkGetBufferMemoryRequirements(vk_device, vk_uploadImageBuffer, &requirements);

	VkDeviceSize requiredSizeForBuffer = requirements.size;

	bool memoryIsHostCoherent = false;
	AllocateMemory(vk_uploadImageMemory, memoryHeaps, vk_device, static_cast<int> (requiredSizeForBuffer), requirements.memoryTypeBits, MT_HostVisible, &memoryIsHostCoherent);

	vkBindBufferMemory(vk_device, vk_uploadImageBuffer, vk_uploadImageMemory, 0);

	void* data = NULL;
	vkMapMemory(vk_device, vk_uploadImageMemory, 0, VK_WHOLE_SIZE, 0, &data);

	// actually copy texture to GPU
	memcpy(data, image_data, image_size);

	if (!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = vk_uploadImageMemory;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(vk_device, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(vk_device, vk_uploadImageMemory);

	VkBufferImageCopy bufferImageCopy = {};
	bufferImageCopy.imageExtent.width = width;
	bufferImageCopy.imageExtent.height = height;
	bufferImageCopy.imageExtent.depth = 1;
	bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopy.imageSubresource.mipLevel = 0;
	bufferImageCopy.imageSubresource.layerCount = 1;

	VkImageMemoryBarrier imageBarrier = {};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier.pNext = NULL;
	imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier.srcAccessMask = 0;
	imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrier.image = vk_Image;
	imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBarrier.subresourceRange.layerCount = 1;
	imageBarrier.subresourceRange.levelCount = 1;

	vkCmdPipelineBarrier(vk_cmd_buffer_array[QUEUE_SLOT_COUNT], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);

	vkCmdCopyBufferToImage(vk_cmd_buffer_array[QUEUE_SLOT_COUNT], vk_uploadImageBuffer, vk_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

	imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(vk_cmd_buffer_array[QUEUE_SLOT_COUNT], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.image = vk_Image;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	vkCreateImageView(vk_device, &imageViewCreateInfo, NULL, &vk_ImageView);
}

/*
Sets shader types, image buffers used, probably just render state
*/
void Graphics::CreateDescriptors()
{
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[2] = {};
	descriptorSetLayoutBinding[0].binding = 0;
	descriptorSetLayoutBinding[0].descriptorCount = 1;
	descriptorSetLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorSetLayoutBinding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetLayoutBinding[1].binding = 1;
	descriptorSetLayoutBinding[1].descriptorCount = 1;
	descriptorSetLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorSetLayoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetLayoutBinding[1].pImmutableSamplers = &vk_sampler;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo[1] = {};
	descriptorSetLayoutCreateInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo[0].bindingCount = 2;
	descriptorSetLayoutCreateInfo[0].pBindings = descriptorSetLayoutBinding;

	vkCreateDescriptorSetLayout(vk_device, descriptorSetLayoutCreateInfo, NULL, &vk_descriptorSetLayout);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pSetLayouts = &vk_descriptorSetLayout;
	pipelineLayoutCreateInfo.setLayoutCount = 1;

	vkCreatePipelineLayout(vk_device, &pipelineLayoutCreateInfo, NULL, &vk_pipelineLayout);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = 1;

	VkDescriptorPoolSize descriptorPoolSize[2] = {};
	descriptorPoolSize[0].descriptorCount = 1;
	descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorPoolSize[1].descriptorCount = 1;
	descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;

	descriptorPoolCreateInfo.poolSizeCount = 2;
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSize;

	vkCreateDescriptorPool(vk_device, &descriptorPoolCreateInfo, NULL, &vk_descriptorPool);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pSetLayouts = &vk_descriptorSetLayout;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.descriptorPool = vk_descriptorPool;

	vkAllocateDescriptorSets(vk_device, &descriptorSetAllocateInfo, &vk_descriptorSet);

	VkWriteDescriptorSet writeDescriptorSets[1] = {};
	writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[0].dstSet = vk_descriptorSet;
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeDescriptorSets[0].dstBinding = 0;

	VkDescriptorImageInfo descriptorImageInfo[1] = {};
	descriptorImageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfo[0].imageView = vk_ImageView;

	writeDescriptorSets[0].pImageInfo = &descriptorImageInfo[0];

	vkUpdateDescriptorSets(vk_device, 1, writeDescriptorSets, 0, NULL);
}

/*
Copies shader to GPU
*/
void Graphics::LoadShader(VkDevice device, const void* shaderContents, const unsigned int size, VkShaderModule &shader)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	shaderModuleCreateInfo.pCode = (unsigned int *) (shaderContents);
	shaderModuleCreateInfo.codeSize = size;

	vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &shader);
}

/*
Sets parameters like depth, viewport, vertex + pixel formats etc
*/
void Graphics::CreatePipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout, VkShaderModule vertexShader, VkShaderModule fragmentShader, VkPipeline &pipeline)
{
	VkVertexInputBindingDescription vertexInputBindingDescription;
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBindingDescription.stride = sizeof(vertex_t);

	VkVertexInputAttributeDescription vertexInputAttributeDescription[2] = {};
	vertexInputAttributeDescription[0].binding = vertexInputBindingDescription.binding;
	vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescription[0].location = 0;
	vertexInputAttributeDescription[0].offset = 0;

	vertexInputAttributeDescription[1].binding = vertexInputBindingDescription.binding;
	vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescription[1].location = 1;
	vertexInputAttributeDescription[1].offset = sizeof(float) * 3;

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = extent<decltype(vertexInputAttributeDescription)>::value;
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

	VkPipelineInputAssemblyStateCreateInfo  pipelineInputAssemblyStateCreateInfo = {};
	pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
	pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.scissorCount = 1;

	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
	pipelineColorBlendAttachmentState.colorWriteMask = 0xF;
	pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
	pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

	pipelineColorBlendStateCreateInfo.attachmentCount = 1;
	pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
	pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
	pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
	pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
	pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.front = pipelineDepthStencilStateCreateInfo.back;

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {};
	pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2] = {};
	pipelineShaderStageCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageCreateInfos[0].module = vertexShader;
	pipelineShaderStageCreateInfos[0].pName = "main";
	pipelineShaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

	pipelineShaderStageCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageCreateInfos[1].module = fragmentShader;
	pipelineShaderStageCreateInfos[1].pName = "main";
	pipelineShaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	graphicsPipelineCreateInfo.layout = layout;
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos;
	graphicsPipelineCreateInfo.stageCount = 2;

	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, NULL, &pipeline);
}


/*
Takes two shaders and combines into "pipeline"
*/
int Graphics::LoadShadersAndCreatePipelineStateObject(char *vertex_shader_file, char *fragment_shader_file)
{
	unsigned int vertex_size = 0;
	unsigned int fragment_size = 0;
	char *vertex_shader = get_file(vertex_shader_file, &vertex_size);
	if (vertex_shader == NULL)
	{
		printf("unable to open vertex shader\r\n");
		return -1;
	}

	char *fragment_shader = get_file(fragment_shader_file, &fragment_size);
	if (fragment_shader == NULL)
	{
		printf("unable to open fragment shader\r\n");
		return -1;
	}


	LoadShader(vk_device, vertex_shader, vertex_size, vk_vertexShader);
	delete[] vertex_shader;
	LoadShader(vk_device, fragment_shader, fragment_size, vk_fragmentShader);
	delete[] fragment_shader;

	CreatePipeline(vk_device, vk_renderpass, vk_pipelineLayout, vk_vertexShader, vk_fragmentShader, vk_pipeline);
	return 0;
}

/*
Full screen quad vertex / index buffers, single upload is kind of interesting
*/
void Graphics::CreateMeshBuffers(VkCommandBuffer uploadBuffer, vertex_t *vertices, int num_vertex, unsigned int *indices, int num_index)
{
	vector<MemoryTypeInfo> memoryHeaps = EnumerateHeaps(vk_physical);
	VkBufferUsageFlagBits index_flag;
	index_flag = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	VkBufferUsageFlagBits vertex_flag;
	vertex_flag = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	AllocateBuffer(vk_device, sizeof(unsigned int) * num_index, index_flag, vk_indexBuffer);
	AllocateBuffer(vk_device, sizeof(vertex_t) * num_vertex, vertex_flag, vk_vertexBuffer);

	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, vk_vertexBuffer, &vertexBufferMemoryRequirements);
	VkMemoryRequirements indexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, vk_indexBuffer, &indexBufferMemoryRequirements);

	VkDeviceSize bufferSize = vertexBufferMemoryRequirements.size;
	// We want to place the index buffer behind the vertex buffer. Need to take
	// the alignment into account to find the next suitable location
	VkDeviceSize indexBufferOffset = RoundToNextMultiple(bufferSize, indexBufferMemoryRequirements.alignment);

	bufferSize = indexBufferOffset + indexBufferMemoryRequirements.size;
	AllocateMemory(vk_deviceMemory, memoryHeaps, vk_device, static_cast<int>(bufferSize), vertexBufferMemoryRequirements.memoryTypeBits & indexBufferMemoryRequirements.memoryTypeBits, MT_DeviceLocal);

	vkBindBufferMemory(vk_device, vk_vertexBuffer, vk_deviceMemory, 0);
	vkBindBufferMemory(vk_device, vk_indexBuffer, vk_deviceMemory, indexBufferOffset);

	AllocateBuffer(vk_device, static_cast<int> (bufferSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vk_uploadBuffer);
	VkMemoryRequirements uploadBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, vk_uploadBuffer, &uploadBufferMemoryRequirements);

	bool memoryIsHostCoherent = false;
	AllocateMemory(vk_uploadMemory, memoryHeaps, vk_device, static_cast<int>(uploadBufferMemoryRequirements.size), vertexBufferMemoryRequirements.memoryTypeBits & indexBufferMemoryRequirements.memoryTypeBits,	MT_HostVisible, &memoryIsHostCoherent);

	vkBindBufferMemory(vk_device, vk_uploadBuffer, vk_uploadMemory, 0);

	void* mapping = NULL;
	vkMapMemory(vk_device, vk_uploadMemory, 0, VK_WHOLE_SIZE, 0, &mapping);
	memcpy(mapping, vertices, sizeof(vertex_t) * num_vertex);

	memcpy(static_cast<uint8_t*> (mapping) + indexBufferOffset, indices, sizeof(unsigned int) * num_index);

	if (!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = vk_uploadMemory;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(vk_device, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(vk_device, vk_uploadMemory);

	VkBufferCopy vertexCopy = {};
	vertexCopy.size = sizeof(vertex_t) * num_vertex;

	VkBufferCopy indexCopy = {};
	indexCopy.size = sizeof(unsigned int) * num_index;
	indexCopy.srcOffset = indexBufferOffset;

	vkCmdCopyBuffer(uploadBuffer, vk_uploadBuffer, vk_vertexBuffer, 1, &vertexCopy);
	vkCmdCopyBuffer(uploadBuffer, vk_uploadBuffer, vk_indexBuffer, 1, &indexCopy);

	VkBufferMemoryBarrier uploadBarriers[2] = {};
	uploadBarriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	uploadBarriers[0].buffer = vk_vertexBuffer;
	uploadBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	uploadBarriers[0].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	uploadBarriers[0].size = VK_WHOLE_SIZE;

	uploadBarriers[1].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	uploadBarriers[1].buffer = vk_indexBuffer;
	uploadBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	uploadBarriers[1].dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
	uploadBarriers[1].size = VK_WHOLE_SIZE;

	vkCmdPipelineBarrier(uploadBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, NULL, 2, uploadBarriers, 0, NULL);
}


void Graphics::DrawArrayTri(int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
	if (initialized == false)
		return;


	gpustat.drawcall++;
	gpustat.triangle += num_index / 3;


	VkCommandBuffer cmdBuffer = vk_cmd_buffer_array[vk_currentBackBuffer];

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.framebuffer = vk_framebuffer[vk_currentBackBuffer];
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.renderPass = vk_renderpass;

	VkClearValue clearValue = {};

	clearValue.color.float32[0] = 0.042f;
	clearValue.color.float32[1] = 0.042f;
	clearValue.color.float32[2] = 0.042f;
	clearValue.color.float32[3] = 1.0f;

	renderPassBeginInfo.pClearValues = &clearValue;
	renderPassBeginInfo.clearValueCount = 1;

	vkCmdBeginRenderPass(cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);



	VkViewport viewports[1] = {};
	viewports[0].width = (float)width;
	viewports[0].height = (float)height;
	viewports[0].minDepth = 0;
	viewports[0].maxDepth = 1;

	vkCmdSetViewport(cmdBuffer, 0, 1, viewports);

	VkRect2D scissors[1] = {};
	scissors[0].extent.width = width;
	scissors[0].extent.height = height;
	vkCmdSetScissor(cmdBuffer, 0, 1, scissors);

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindIndexBuffer(cmdBuffer, vk_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vk_vertexBuffer, offsets);

	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipelineLayout, 0, 1, &vk_descriptorSet, 0, NULL);

	vkCmdDrawIndexed(cmdBuffer, num_index, 1, start_index, start_vertex, 0);

	vkCmdEndRenderPass(cmdBuffer);
	vkEndCommandBuffer(cmdBuffer);

}


/*
destroy the thousand handles to bullcrap
*/
void Graphics::destroy()
{
	initialized = false;

	vkDestroySemaphore(vk_device, vk_imageAcquiredSemaphore, NULL);
	vkDestroySemaphore(vk_device, vk_renderingCompleteSemaphore, NULL);

	vkDestroyPipeline(vk_device, vk_pipeline, NULL);
	vkDestroyPipelineLayout(vk_device, vk_pipelineLayout, NULL);

	vkDestroyBuffer(vk_device, vk_vertexBuffer, NULL);
	vkDestroyBuffer(vk_device, vk_indexBuffer, NULL);
	vkFreeMemory(vk_device, vk_deviceMemory, NULL);

	vkDestroyImageView(vk_device, vk_ImageView, NULL);
	vkDestroyImage(vk_device, vk_Image, NULL);
	vkFreeMemory(vk_device, vk_deviceImageMemory, NULL);

	vkDestroyBuffer(vk_device, vk_uploadImageBuffer, NULL);
	vkFreeMemory(vk_device, vk_uploadImageMemory, NULL);

	vkDestroyBuffer(vk_device, vk_uploadBuffer, NULL);
	vkFreeMemory(vk_device, vk_uploadMemory, NULL);

	vkDestroyDescriptorSetLayout(vk_device, vk_descriptorSetLayout, NULL);
	vkDestroyDescriptorPool(vk_device, vk_descriptorPool, NULL);

	vkDestroySampler(vk_device, vk_sampler, NULL);

	vkDestroyShaderModule(vk_device, vk_vertexShader, NULL);
	vkDestroyShaderModule(vk_device, vk_fragmentShader, NULL);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		vkDestroyFence(vk_device, vk_frameFences[i], NULL);
	}

	vkDestroyRenderPass(vk_device, vk_renderpass, NULL);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		vkDestroyFramebuffer(vk_device, vk_framebuffer[i], NULL);
		vkDestroyImageView(vk_device, vk_swapChainImageViews[i], NULL);
	}

	vkDestroyCommandPool(vk_device, vk_commandPool, NULL);

	vkDestroySwapchainKHR(vk_device, vk_swapchain, NULL);
	vkDestroySurfaceKHR(vk_instance, vk_surface, NULL);


	DestroyDebugReportCallback(vk_instance, vk_callback, NULL);


	vkDestroyDevice(vk_device, NULL);
	vkDestroyInstance(vk_instance, NULL);

	initialized = false;
}

void Graphics::CreateRenderPass(VkDevice device, VkFormat swapchainFormat, VkRenderPass &rp)
{
	VkAttachmentDescription attachmentDescription;

	memset(&attachmentDescription, 0, sizeof(VkAttachmentDescription));
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.format = swapchainFormat;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkAttachmentReference attachmentReference;

	memset(&attachmentReference, 0, sizeof(VkAttachmentReference));
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;

	memset(&subpassDescription, 0, sizeof(VkSubpassDescription));
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pColorAttachments = &attachmentReference;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkRenderPassCreateInfo renderPassCreateInfo;

	memset(&renderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.pAttachments = &attachmentDescription;

	vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &rp);
}

void Graphics::CreateFramebuffers(VkDevice device, VkRenderPass renderPass, const int width, const int height, const int count, const VkImageView* imageViews, VkFramebuffer* framebuffers)
{
	for (int i = 0; i < count; ++i)
	{
		VkFramebufferCreateInfo framebufferCreateInfo;

		memset(&framebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &imageViews[i];
		framebufferCreateInfo.height = height;
		framebufferCreateInfo.width = width;
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.renderPass = renderPass;

		vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, &framebuffers[i]);
	}
}

void Graphics::CreateSwapchainImageViews(VkDevice device, VkFormat format, const int count, const VkImage* images, VkImageView* imageViews)
{
	for (int i = 0; i < count; ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo;

		memset(&imageViewCreateInfo, 0, sizeof(imageViewCreateInfo));
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = images[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		vkCreateImageView(device, &imageViewCreateInfo, NULL, &imageViews[i]);
	}
}

 void Graphics::CreateWin32Surface(VkInstance instance, HWND hwnd, VkSurfaceKHR &surface)
{
	VkWin32SurfaceCreateInfoKHR win32surfaceCreateInfo;

	memset(&win32surfaceCreateInfo, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
	win32surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32surfaceCreateInfo.hwnd = hwnd;
	win32surfaceCreateInfo.hinstance = ::GetModuleHandle(NULL);

	vkCreateWin32SurfaceKHR(instance, &win32surfaceCreateInfo, NULL, &surface);
}

void Graphics::CreateDeviceAndQueue(VkInstance instance, VkDevice &outputDevice, VkQueue &outputQueue, int &outputQueueIndex, VkPhysicalDevice &outputPhysicalDevice)
{
	VkPhysicalDevice device_array[4];
	VkQueueFamilyProperties prop[4];
	unsigned int num_gpu = 0;
	unsigned int num_prop = 0;
	int graphicsQueueIndex = 0;

	vkEnumeratePhysicalDevices(instance, &num_gpu, NULL); // get count
	vkEnumeratePhysicalDevices(instance, &num_gpu, &device_array[0]);

	vkGetPhysicalDeviceQueueFamilyProperties(device_array[0], &num_prop, NULL); // get count
	vkGetPhysicalDeviceQueueFamilyProperties(device_array[0], &num_prop, prop);

	VkDeviceQueueCreateInfo deviceQueueCreateInfo;
	static const float queuePriorities[] = { 1.0f };


	memset(&deviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.queueFamilyIndex = graphicsQueueIndex;
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

	VkDeviceCreateInfo deviceCreateInfo;

	memset(&deviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;

	VkPhysicalDeviceFeatures features;

	memset(&features, 0, sizeof(VkPhysicalDeviceFeatures));
	features.shaderClipDistance = true;

	deviceCreateInfo.pEnabledFeatures = &features;

	char *debugDeviceLayerNames = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	deviceCreateInfo.ppEnabledLayerNames = &debugDeviceLayerNames;
	deviceCreateInfo.enabledLayerCount = 1;

	char *deviceExtensions[] = {
		"VK_KHR_swapchain"
	};

	deviceCreateInfo.ppEnabledExtensionNames = (const char **)&deviceExtensions;
	deviceCreateInfo.enabledExtensionCount = 1;


	VkDevice device = NULL;
	vkCreateDevice(device_array[0], &deviceCreateInfo, NULL, &device);

	VkQueue queue = NULL;
	vkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

	// output parameters
	outputQueue = queue;
	outputDevice = device;
	outputQueueIndex = graphicsQueueIndex;
	outputPhysicalDevice = device_array[0];
}

VkInstance Graphics::CreateInstance()
{
	VkInstanceCreateInfo instanceCreateInfo;
	VkApplicationInfo applicationInfo;

	char *instanceExtensions[] = {
		"VK_KHR_surface",
		"VK_KHR_win32_surface",
		"VK_EXT_debug_report"
	};
	char *instanceLayers[] = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	memset(&instanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions;
	instanceCreateInfo.enabledExtensionCount = 3;
	instanceCreateInfo.ppEnabledLayerNames = instanceLayers;
	instanceCreateInfo.enabledLayerCount = 1;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;


	memset(&applicationInfo, 0, sizeof(VkApplicationInfo));
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pApplicationName = "Vulkan";
	applicationInfo.pEngineName = "Vulkan";


	VkInstance instance = VK_NULL_HANDLE;
	vkCreateInstance(&instanceCreateInfo, NULL, &instance);

	return instance;
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

	// sometimes we get in here with invalid dimensions
	width = 1920;
	height = 1080;

	// get vulkan driver instance
	vk_instance = CreateInstance();
	if (vk_instance == VK_NULL_HANDLE)
	{
		printf("Unable to create Vulkan instance\r\n");
		return;
	}

	// set up some debug message box call backs
	CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vk_instance, "vkCreateDebugReportCallbackEXT");
	DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugReportCallbackEXT");
	SetupDebugCallback(vk_instance, vk_callback);



	// get GPU device and queue for command buffers. Queue types include: Graphics, Compute, Transfer, sparse
	CreateDeviceAndQueue(vk_instance, vk_device, vk_queue, vk_queueFamilyIndex, vk_physical);

	
	// Create platform specific buffers
#ifdef WIN32
	CreateWin32Surface(vk_instance, hwnd, vk_surface);
#endif

#ifdef __linux__
	// If you already got your X11 window you need to define VK_USE_PLATFORM_XLIB_KHR
	// and create a Vulkan compatible surface from it using vkCreateXlibSurfaceKHR,
	// Also note that you need to provide the proper surface extension at instance creation time.
	// VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif

	VkBool32 presentSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical, 0, vk_surface, &presentSupported);

	VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
	CreateSwapchain(vk_physical, vk_device, vk_surface, width, height, QUEUE_SLOT_COUNT, swapchainFormat, vk_swapchain);


	unsigned int swapchainImageCount = 0; // three swap buffers
	vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &swapchainImageCount, NULL);

	vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &swapchainImageCount, vk_swapchainImages);

	CreateRenderPass(vk_device, swapchainFormat, vk_renderpass);

	CreateSwapchainImageViews(vk_device, swapchainFormat, QUEUE_SLOT_COUNT, vk_swapchainImages, vk_swapChainImageViews);
	CreateFramebuffers(vk_device, vk_renderpass, width, height, QUEUE_SLOT_COUNT, vk_swapChainImageViews, vk_framebuffer);

	VkCommandPoolCreateInfo commandPoolCreateInfo;

	memset(&commandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = vk_queueFamilyIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool(vk_device, &commandPoolCreateInfo, NULL, &vk_commandPool);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo;

	memset(&commandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = QUEUE_SLOT_COUNT + 1;
	commandBufferAllocateInfo.commandPool = vk_commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkAllocateCommandBuffers(vk_device, &commandBufferAllocateInfo, vk_cmd_buffer_array);


	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		VkFenceCreateInfo fenceCreateInfo;


		memset(&fenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Wait on creation
		vkCreateFence(vk_device, &fenceCreateInfo, NULL, &vk_frameFences[i]);
	}

	// load texture, shaders, etc
	LoadCommandBuffer(vk_cmd_buffer_array[QUEUE_SLOT_COUNT]);


	VkSemaphoreCreateInfo semaphoreCreateInfo;

	memset(&semaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore(vk_device, &semaphoreCreateInfo, NULL, &vk_imageAcquiredSemaphore);
	vkCreateSemaphore(vk_device, &semaphoreCreateInfo, NULL, &vk_renderingCompleteSemaphore);


	initialized = true;
	initialized_once = true;


	// partial swap to prepare back buffer
	vkAcquireNextImageKHR(vk_device, vk_swapchain, UINT64_MAX, vk_imageAcquiredSemaphore, VK_NULL_HANDLE, &vk_currentBackBuffer);
	vkWaitForFences(vk_device, 1, &vk_frameFences[vk_currentBackBuffer], VK_TRUE, UINT64_MAX);
	vkResetFences(vk_device, 1, &vk_frameFences[vk_currentBackBuffer]);


	DrawArrayTri(0, 0, 6, 6);
	swap();



}


void Graphics::LoadCommandBuffer(VkCommandBuffer &cmd_buffer)
{
	vkResetFences(vk_device, 1, &vk_frameFences[0]);
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(cmd_buffer, &beginInfo);


	unsigned int index_array[6] =
	{
		0, 1, 2, 2, 3, 0
	};


	vertex_t vertex_array[4];

	memset(vertex_array, 0, sizeof(vertex_t) * 4);
	vertex_array[0].position.x = -1.0f;
	vertex_array[0].position.y = 1.0f;
	vertex_array[0].position.z = 0.0f;
	vertex_array[0].texCoord0.x = 0.0f;
	vertex_array[0].texCoord0.y = 1.0f;

	vertex_array[1].position.x = 1.0f;
	vertex_array[1].position.y = 1.0f;
	vertex_array[1].position.z = 0.0f;
	vertex_array[1].texCoord0.x = 1.0f;
	vertex_array[1].texCoord0.y = 1.0f;

	vertex_array[2].position.x = 1.0f;
	vertex_array[2].position.y = -1.0f;
	vertex_array[2].position.z = 0.0f;
	vertex_array[2].texCoord0.x = 1.0f;
	vertex_array[2].texCoord0.y = 0.0f;

	vertex_array[3].position.x = -1.0f;
	vertex_array[3].position.y = -1.0f;
	vertex_array[3].position.z = 0.0f;
	vertex_array[3].texCoord0.x = 0.0f;
	vertex_array[3].texCoord0.y = 0.0f;

	CreateSampler();
	// load texture, will call gfx loadtexture, which will call CreateTexture below
	load_texture(*this, "media/menu.tga", false, false, 0);
	//	CreateTexture(setupCommandBuffer_, image_width, image_height, image_data, image_size);
	CreateDescriptors();
	LoadShadersAndCreatePipelineStateObject("media/spirv/vulkan.vert.spv", "media/spirv/vulkan.frag.spv");
	CreateMeshBuffers(cmd_buffer, vertex_array, 4, index_array, 6);

	vkEndCommandBuffer(cmd_buffer);



	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd_buffer;
	vkQueueSubmit(vk_queue, 1, &submitInfo, vk_frameFences[0]);

	vkWaitForFences(vk_device, 1, &vk_frameFences[0], VK_TRUE, UINT64_MAX);
}

void Graphics::swap()
{
	gpustat.drawcall = 0;
	gpustat.triangle = 0;

	if (initialized == false)
		return;


	// Submit rendering work to the graphics queue
	const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &vk_imageAcquiredSemaphore;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vk_cmd_buffer_array[vk_currentBackBuffer];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &vk_renderingCompleteSemaphore;
	vkQueueSubmit(vk_queue, 1, &submitInfo, VK_NULL_HANDLE);

	// Submit present operation to present queue
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &vk_renderingCompleteSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &vk_swapchain;
	presentInfo.pImageIndices = &vk_currentBackBuffer;
	vkQueuePresentKHR(vk_queue, &presentInfo);

	vkQueueSubmit(vk_queue, 0, NULL, vk_frameFences[vk_currentBackBuffer]);
	// Wait for all rendering to finish
	vkWaitForFences(vk_device, 3, vk_frameFences, VK_TRUE, UINT64_MAX);


	/*
	vkAcquireNextImageKHR(vk_device, swapchain_, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &currentBackBuffer_);

	vkWaitForFences(vk_device, 1, &frameFences_[currentBackBuffer_], VK_TRUE, UINT64_MAX);
	vkResetFences(vk_device, 1, &frameFences_[currentBackBuffer_]);
*/
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

void Graphics::DrawArray(primitive_t primitive, int start_index, int start_vertex, unsigned int num_index, int num_verts)
{
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
	return 0;
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = QUEUE_SLOT_COUNT + 1;
	commandBufferAllocateInfo.commandPool = vk_commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;


	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkCommandBuffer i_setupCommandBuffer_;

	VkCommandBuffer i_commandBuffers[QUEUE_SLOT_COUNT + 1];
	vkAllocateCommandBuffers(vk_device, &commandBufferAllocateInfo, i_commandBuffers);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
//		i_commandBuffers_[i] = i_commandBuffers[i];
	}

	i_setupCommandBuffer_ = i_commandBuffers[QUEUE_SLOT_COUNT];



	vector<MemoryTypeInfo> memoryHeaps = EnumerateHeaps(vk_physical);

	VkBufferUsageFlagBits bflag;
	bflag = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	AllocateBuffer(vk_device, sizeof(int) * num_index, bflag, vk_indexBuffer);

	VkMemoryRequirements indexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, vk_indexBuffer, &indexBufferMemoryRequirements);

	VkDeviceSize bufferSize = indexBufferMemoryRequirements.size;

	AllocateMemory(vk_deviceMemory, memoryHeaps, vk_device, static_cast<int>(bufferSize), indexBufferMemoryRequirements.memoryTypeBits, MT_DeviceLocal);

	vkBindBufferMemory(vk_device, vk_indexBuffer, vk_deviceMemory, 0);

	AllocateBuffer(vk_device, static_cast<int> (bufferSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vk_uploadBuffer);
	VkMemoryRequirements uploadBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, vk_uploadBuffer, &uploadBufferMemoryRequirements);

	bool memoryIsHostCoherent = false;
	AllocateMemory(vk_uploadMemory, memoryHeaps, vk_device, static_cast<int>(uploadBufferMemoryRequirements.size), indexBufferMemoryRequirements.memoryTypeBits, MT_HostVisible, &memoryIsHostCoherent);

	vkBindBufferMemory(vk_device, vk_uploadBuffer, vk_uploadMemory, 0);

	void* mapping = NULL;
	vkMapMemory(vk_device, vk_uploadMemory, 0, VK_WHOLE_SIZE, 0, &mapping);
	memcpy(mapping, index_buffer, sizeof(int) * num_index);

	if (!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = vk_uploadMemory;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(vk_device, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(vk_device, vk_uploadMemory);

	VkBufferCopy indexCopy = {};
	indexCopy.size = sizeof(int) * num_index;

	vkCmdCopyBuffer(i_setupCommandBuffer_, vk_uploadBuffer, vk_indexBuffer, 1, &indexCopy);

	VkBufferMemoryBarrier uploadBarriers[1] = {};
	uploadBarriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	uploadBarriers[0].buffer = vk_vertexBuffer;
	uploadBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	uploadBarriers[0].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	uploadBarriers[0].size = VK_WHOLE_SIZE;

	vkCmdPipelineBarrier(i_setupCommandBuffer_, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 1, uploadBarriers, 0, NULL);
	vkEndCommandBuffer(i_setupCommandBuffer_);
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

int Graphics::CreateVertexBuffer(void *vertex_buffer, int num_vertex, bool dynamic)
{
	return 0;
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;	commandBufferAllocateInfo.commandBufferCount = QUEUE_SLOT_COUNT + 1;	commandBufferAllocateInfo.commandPool = vk_commandPool;	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VkCommandBufferBeginInfo beginInfo = {};	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;	vkBeginCommandBuffer(vk_cmd_buffer_array[QUEUE_SLOT_COUNT], &beginInfo);
	VkCommandBuffer v_commandBuffers[QUEUE_SLOT_COUNT + 1];	VkCommandBuffer v_setupCommandBuffer_;
	vkAllocateCommandBuffers(vk_device, &commandBufferAllocateInfo, v_commandBuffers);	v_setupCommandBuffer_ = v_commandBuffers[QUEUE_SLOT_COUNT];	vkBeginCommandBuffer(v_setupCommandBuffer_, &beginInfo);
	vector<MemoryTypeInfo> memoryHeaps = EnumerateHeaps(vk_physical);
	VkBufferUsageFlagBits vertex_flag;
	vertex_flag = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	AllocateBuffer(vk_device, sizeof(vertex_t) * num_vertex, vertex_flag, vk_vertexBuffer);

	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, vk_vertexBuffer, &vertexBufferMemoryRequirements);

	VkDeviceSize bufferSize = vertexBufferMemoryRequirements.size;

	AllocateMemory(vk_deviceMemory, memoryHeaps, vk_device, static_cast<int>(bufferSize), vertexBufferMemoryRequirements.memoryTypeBits, MT_DeviceLocal);

	vkBindBufferMemory(vk_device, vk_vertexBuffer, vk_deviceMemory, 0);

	AllocateBuffer(vk_device, static_cast<int> (bufferSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vk_uploadBuffer);
	VkMemoryRequirements uploadBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, vk_uploadBuffer, &uploadBufferMemoryRequirements);

	bool memoryIsHostCoherent = false;
	AllocateMemory(vk_uploadMemory, memoryHeaps, vk_device, static_cast<int>(uploadBufferMemoryRequirements.size), vertexBufferMemoryRequirements.memoryTypeBits, MT_HostVisible, &memoryIsHostCoherent);

	vkBindBufferMemory(vk_device, vk_uploadBuffer, vk_uploadMemory, 0);

	void* mapping = NULL;
	vkMapMemory(vk_device, vk_uploadMemory, 0, VK_WHOLE_SIZE, 0, &mapping);
	memcpy(mapping, vertex_buffer, sizeof(vertex_t) * num_vertex);

	if (!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = vk_uploadMemory;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(vk_device, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(vk_device, vk_uploadMemory);

	VkBufferCopy vertexCopy = {};
	vertexCopy.size = sizeof(vertex_t) * num_vertex;
	vkCmdCopyBuffer(v_setupCommandBuffer_, vk_uploadBuffer, vk_vertexBuffer, 1, &vertexCopy);

	VkBufferMemoryBarrier uploadBarriers[1] = {};
	uploadBarriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	uploadBarriers[0].buffer = vk_vertexBuffer;
	uploadBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	uploadBarriers[0].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	uploadBarriers[0].size = VK_WHOLE_SIZE;

	vkCmdPipelineBarrier(v_setupCommandBuffer_, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, NULL, 1, uploadBarriers, 0, NULL);
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

int Graphics::LoadTexture(int width, int height, int components, int format, void *bytes, bool clamp, int anisotrophy)
{
	static bool once = true;

	if (once)
	{
		CreateTexture(width, height, components, format, (unsigned char *)bytes, false);
		once = false;
	}
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

void Graphics::TwoSidedStencilOp(int face, int stencil_fail, int zfail, int zpass)
{

}

void Graphics::clear_color(vec3 &color)
{
}

void mLight2::set_contrast(float value)
{
}


int Graphics::checkFramebuffer()
{
	return 0;
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



void EnumDebugInstanceExtensionNames()
{
	unsigned int extensionCount = 0; // 12 extensions
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

	VkExtensionProperties extension[32];;
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extension);

	for (int i = 0; i < extensionCount; i++)
	{
		printf("%s\n", extension[i].extensionName);
	}
}

void EnumDebugInstanceLayerNames()
{
	unsigned int layerCount = 0; // 4 layers
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	VkLayerProperties layers[4];
	vkEnumerateInstanceLayerProperties(&layerCount, layers);

	for (int i = 0; i < layerCount; i++)
	{
		printf("%s\r\n", layers[i].layerName);
	}
}

void EnumDebugDeviceLayerNames(VkPhysicalDevice device)
{
	VkLayerProperties deviceLayers[4];
	unsigned int layerCount = 0; // 2 layers

	vkEnumerateDeviceLayerProperties(device, &layerCount, NULL); // get count
	vkEnumerateDeviceLayerProperties(device, &layerCount, deviceLayers);

	for (int i = 0; i < layerCount; i++)
	{
		printf("%s\r\n", deviceLayers[i].layerName);
	}

}
#endif
