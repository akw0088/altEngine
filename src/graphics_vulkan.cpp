#include "include.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#include "stb_image.h"
#include "assert.h"

#ifdef VULKAN

PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = VK_NULL_HANDLE;


void Graphics::resize(int width, int height)
{
	Graphics::width = width;
	Graphics::height = height;
}

Graphics::Graphics()
{
}


Graphics::~Graphics()
{
}


VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
	VkDebugReportFlagsEXT       /*flags*/,
	VkDebugReportObjectTypeEXT  /*objectType*/,
	uint64_t                    /*object*/,
	size_t                      /*location*/,
	int32_t                     /*messageCode*/,
	const char*                 /*pLayerPrefix*/,
	const char*                 pMessage,
	void*                       pUserData)
{
	printf("%s\n", pMessage);
#ifdef WIN32
	MessageBox((HWND)pUserData, pMessage, "Vulkan Error", 0);

#endif
	return VK_FALSE;
}


VkDebugReportCallbackEXT Graphics::SetupDebugCallback(VkInstance instance)
{
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callbackCreateInfo.flags =
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = &DebugReportCallback;
	callbackCreateInfo.pUserData = hwnd;

	VkDebugReportCallbackEXT callback;
	CreateDebugReportCallback(instance, &callbackCreateInfo, nullptr, &callback);
	return callback;
}


VkSwapchainKHR Graphics::CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device,
	VkSurfaceKHR surface, const int surfaceWidth, const int surfaceHeight,
	const int backbufferCount, VkFormat* swapchainFormat)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,
		surface, &surfaceCapabilities);

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
		surface, &presentModeCount, NULL);

	vector<VkPresentModeKHR> presentModes{ presentModeCount };

	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
		surface, &presentModeCount, presentModes.data());

	VkExtent2D swapChainSize = {};
	swapChainSize = surfaceCapabilities.currentExtent;

	uint32_t swapChainImageCount = backbufferCount;


	VkSurfaceTransformFlagBitsKHR surfaceTransformFlags;

	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		surfaceTransformFlags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		surfaceTransformFlags = surfaceCapabilities.currentTransform;
	}

	auto swapchainFormatColorSpace = GetSwapchainFormatAndColorspace(physicalDevice, surface);

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
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

	VkSwapchainKHR swapchain;
	vkCreateSwapchainKHR(device, &swapchainCreateInfo,
		NULL, &swapchain);

	if (swapchainFormat)
	{
		*swapchainFormat = swapchainFormatColorSpace.format;
	}

	return swapchain;
}

VkBuffer Graphics::AllocateBuffer(VkDevice device, const int size, const VkBufferUsageFlagBits bits)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = static_cast<uint32_t> (size);
	bufferCreateInfo.usage = bits;

	VkBuffer result;
	vkCreateBuffer(device, &bufferCreateInfo, NULL, &result);
	return result;
}



/*
Wrapper function, loops multiple gpu memory allocations
*/
VkDeviceMemory Graphics::AllocateMemory(const vector<MemoryTypeInfo>& memoryInfos, VkDevice device, const int size, const uint32_t memoryBits, unsigned int memoryProperties, bool* isHostCoherent)
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

		VkDeviceMemory deviceMemory;
		vkAllocateMemory(device, &memoryAllocateInfo, NULL,
			&deviceMemory);
		return deviceMemory;
	}

	return VK_NULL_HANDLE;
}



/*
Checks GPU memory sizes
*/
vector<MemoryTypeInfo> Graphics::EnumerateHeaps(VkPhysicalDevice device)
{
	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

	vector<MemoryTypeInfo::Heap> heaps;

	for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; ++i)
	{
		MemoryTypeInfo::Heap info;
		info.size = memoryProperties.memoryHeaps[i].size;
		info.deviceLocal = (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0;

		heaps.push_back(info);
	}

	vector<MemoryTypeInfo> result;

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
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
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.maxAnisotropy = 1.0f;

	vkCreateSampler(device_, &samplerCreateInfo, NULL, &sampler_);
}

/*
Sets up GPU texture buffer, copies data
*/
void Graphics::CreateTexture(VkCommandBuffer uploadCommandList, int width, int height, unsigned char *image_data, int image_size)
{
	VkDeviceMemory uploadImageMemory_ = VK_NULL_HANDLE;
	VkBuffer uploadImageBuffer_ = VK_NULL_HANDLE;


	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.queueFamilyIndexCount = 1;

	uint32_t queueFamilyIndex = static_cast<uint32_t> (queueFamilyIndex_);
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

	vkCreateImage(device_, &imageCreateInfo, nullptr, &Image_);

	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements(device_, Image_, &requirements);

	VkDeviceSize requiredSizeForImage = requirements.size;

	auto memoryHeaps = EnumerateHeaps(physicalDevice_);
	deviceImageMemory_ = AllocateMemory(memoryHeaps, device_, static_cast<int> (requiredSizeForImage),
		requirements.memoryTypeBits,
		MT_DeviceLocal);

	vkBindImageMemory(device_, Image_, deviceImageMemory_, 0);

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.queueFamilyIndexCount = 1;
	bufferCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;
	bufferCreateInfo.size = requiredSizeForImage;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	vkCreateBuffer(device_, &bufferCreateInfo, nullptr, &uploadImageBuffer_);

	vkGetBufferMemoryRequirements(device_, uploadImageBuffer_, &requirements);

	VkDeviceSize requiredSizeForBuffer = requirements.size;

	bool memoryIsHostCoherent = false;
	uploadImageMemory_ = AllocateMemory(memoryHeaps, device_,
		static_cast<int> (requiredSizeForBuffer), requirements.memoryTypeBits,
		MT_HostVisible, &memoryIsHostCoherent);

	vkBindBufferMemory(device_, uploadImageBuffer_, uploadImageMemory_, 0);

	void* data = nullptr;
	vkMapMemory(device_, uploadImageMemory_, 0, VK_WHOLE_SIZE,
		0, &data);
	::memcpy(data, image_data, image_size);

	if (!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = uploadImageMemory_;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(device_, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(device_, uploadImageMemory_);

	VkBufferImageCopy bufferImageCopy = {};
	bufferImageCopy.imageExtent.width = width;
	bufferImageCopy.imageExtent.height = height;
	bufferImageCopy.imageExtent.depth = 1;
	bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopy.imageSubresource.mipLevel = 0;
	bufferImageCopy.imageSubresource.layerCount = 1;

	VkImageMemoryBarrier imageBarrier = {};
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier.pNext = nullptr;
	imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier.srcAccessMask = 0;
	imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrier.image = Image_;
	imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBarrier.subresourceRange.layerCount = 1;
	imageBarrier.subresourceRange.levelCount = 1;

	vkCmdPipelineBarrier(uploadCommandList,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, nullptr, 0, nullptr,
		1, &imageBarrier);

	vkCmdCopyBufferToImage(uploadCommandList, uploadImageBuffer_,
		Image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &bufferImageCopy);

	imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(uploadCommandList,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr,
		1, &imageBarrier);

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.image = Image_;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	vkCreateImageView(device_, &imageViewCreateInfo, nullptr, &ImageView_);
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
	descriptorSetLayoutBinding[1].pImmutableSamplers = &sampler_;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo[1] = {};
	descriptorSetLayoutCreateInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo[0].bindingCount = 2;
	descriptorSetLayoutCreateInfo[0].pBindings = descriptorSetLayoutBinding;

	vkCreateDescriptorSetLayout(
		device_, descriptorSetLayoutCreateInfo,
		NULL, &descriptorSetLayout_);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout_;
	pipelineLayoutCreateInfo.setLayoutCount = 1;

	vkCreatePipelineLayout(device_, &pipelineLayoutCreateInfo,
		NULL, &pipelineLayout_);

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

	vkCreateDescriptorPool(device_, &descriptorPoolCreateInfo,
		NULL, &descriptorPool_);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout_;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool_;

	vkAllocateDescriptorSets(device_, &descriptorSetAllocateInfo, &descriptorSet_);

	VkWriteDescriptorSet writeDescriptorSets[1] = {};
	writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[0].dstSet = descriptorSet_;
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeDescriptorSets[0].dstBinding = 0;

	VkDescriptorImageInfo descriptorImageInfo[1] = {};
	descriptorImageInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorImageInfo[0].imageView = ImageView_;

	writeDescriptorSets[0].pImageInfo = &descriptorImageInfo[0];

	vkUpdateDescriptorSets(device_, 1, writeDescriptorSets, 0, NULL);
}

/*
Copies shader to GPU
*/
VkShaderModule Graphics::LoadShader(VkDevice device, const void* shaderContents, const size_t size)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	shaderModuleCreateInfo.pCode = static_cast<const uint32_t*> (shaderContents);
	shaderModuleCreateInfo.codeSize = size;

	VkShaderModule result;
	vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, &result);

	return result;
}

/*
Sets parameters like depth, viewport, vertex + pixel formats etc
*/
VkPipeline Graphics::CreatePipeline(VkDevice device, VkRenderPass renderPass, VkPipelineLayout layout,
	VkShaderModule vertexShader, VkShaderModule fragmentShader)
{
	VkVertexInputBindingDescription vertexInputBindingDescription;
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBindingDescription.stride = sizeof(float) * 5;

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

	VkPipeline pipeline;
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo,
		NULL, &pipeline);

	return pipeline;
}


/*
Takes two shaders and combines into "pipeline"
*/
void Graphics::CreatePipelineStateObject()
{
	vertexShader_ = LoadShader(device_, BasicVertexShader, sizeof(BasicVertexShader));
	fragmentShader_ = LoadShader(device_, TexturedFragmentShader, sizeof(TexturedFragmentShader));

	pipeline_ = CreatePipeline(device_, renderPass_, pipelineLayout_,
		vertexShader_, fragmentShader_);
}

/*
Full screen quad vertex / index buffers, single upload is kind of interesting
*/
void Graphics::CreateMeshBuffers(VkCommandBuffer uploadCommandBuffer)
{
	struct Vertex
	{
		float position[3];
		float uv[2];
	};

	static const Vertex vertices[4] =
	{
		// Upper Left
		{ { -1.0f,  1.0f, 0 },{ 0, 1 } },
		// Upper Right
		{ { 1.0f,  1.0f, 0 },{ 1, 1 } },
		// Bottom right
		{ { 1.0f, -1.0f, 0 },{ 1, 0 } },
		// Bottom left
		{ { -1.0f, -1.0f, 0 },{ 0, 0 } }
	};

	static const int indices[6] =
	{
		0, 1, 2, 2, 3, 0
	};

	auto memoryHeaps = EnumerateHeaps(physicalDevice_);

	indexBuffer_ = AllocateBuffer(device_, sizeof(indices),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	vertexBuffer_ = AllocateBuffer(device_, sizeof(vertices),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(device_, vertexBuffer_,
		&vertexBufferMemoryRequirements);
	VkMemoryRequirements indexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(device_, indexBuffer_,
		&indexBufferMemoryRequirements);

	VkDeviceSize bufferSize = vertexBufferMemoryRequirements.size;
	// We want to place the index buffer behind the vertex buffer. Need to take
	// the alignment into account to find the next suitable location
	VkDeviceSize indexBufferOffset = RoundToNextMultiple(bufferSize,
		indexBufferMemoryRequirements.alignment);

	bufferSize = indexBufferOffset + indexBufferMemoryRequirements.size;
	deviceBufferMemory_ = AllocateMemory(memoryHeaps, device_,
		static_cast<int>(bufferSize),
		vertexBufferMemoryRequirements.memoryTypeBits & indexBufferMemoryRequirements.memoryTypeBits,
		MT_DeviceLocal);

	vkBindBufferMemory(device_, vertexBuffer_, deviceBufferMemory_, 0);
	vkBindBufferMemory(device_, indexBuffer_, deviceBufferMemory_,
		indexBufferOffset);

	uploadBufferBuffer_ = AllocateBuffer(device_, static_cast<int> (bufferSize),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	VkMemoryRequirements uploadBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(device_, uploadBufferBuffer_,
		&uploadBufferMemoryRequirements);

	bool memoryIsHostCoherent = false;
	uploadBufferMemory_ = AllocateMemory(memoryHeaps, device_,
		static_cast<int>(uploadBufferMemoryRequirements.size),
		vertexBufferMemoryRequirements.memoryTypeBits & indexBufferMemoryRequirements.memoryTypeBits,
		MT_HostVisible, &memoryIsHostCoherent);

	vkBindBufferMemory(device_, uploadBufferBuffer_, uploadBufferMemory_, 0);

	void* mapping = NULL;
	vkMapMemory(device_, uploadBufferMemory_, 0, VK_WHOLE_SIZE,
		0, &mapping);
	::memcpy(mapping, vertices, sizeof(vertices));

	::memcpy(static_cast<uint8_t*> (mapping) + indexBufferOffset,
		indices, sizeof(indices));

	if (!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = uploadBufferMemory_;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(device_, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(device_, uploadBufferMemory_);

	VkBufferCopy vertexCopy = {};
	vertexCopy.size = sizeof(vertices);

	VkBufferCopy indexCopy = {};
	indexCopy.size = sizeof(indices);
	indexCopy.srcOffset = indexBufferOffset;

	vkCmdCopyBuffer(uploadCommandBuffer, uploadBufferBuffer_, vertexBuffer_,
		1, &vertexCopy);
	vkCmdCopyBuffer(uploadCommandBuffer, uploadBufferBuffer_, indexBuffer_,
		1, &indexCopy);

	VkBufferMemoryBarrier uploadBarriers[2] = {};
	uploadBarriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	uploadBarriers[0].buffer = vertexBuffer_;
	uploadBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	uploadBarriers[0].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	uploadBarriers[0].size = VK_WHOLE_SIZE;

	uploadBarriers[1].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	uploadBarriers[1].buffer = indexBuffer_;
	uploadBarriers[1].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[1].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[1].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	uploadBarriers[1].dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
	uploadBarriers[1].size = VK_WHOLE_SIZE;

	vkCmdPipelineBarrier(uploadCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, NULL, 2, uploadBarriers,
		0, NULL);
}


void Graphics::render_cmdbuffer(VkCommandBuffer commandBuffer, int width, int height)
{
	VkViewport viewports[1] = {};
	viewports[0].width = static_cast<float> (width);
	viewports[0].height = static_cast<float> (height);
	viewports[0].minDepth = 0;
	viewports[0].maxDepth = 1;

	vkCmdSetViewport(commandBuffer, 0, 1, viewports);

	VkRect2D scissors[1] = {};
	scissors[0].extent.width = width;
	scissors[0].extent.height = height;
	vkCmdSetScissor(commandBuffer, 0, 1, scissors);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline_);
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer_, offsets);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout_, 0, 1, &descriptorSet_, 0, nullptr);

	vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
}

void Graphics::render()
{
	vkAcquireNextImageKHR(
		device_, swapchain_, UINT64_MAX, imageAcquiredSemaphore,
		VK_NULL_HANDLE, &currentBackBuffer_);

	vkWaitForFences(device_, 1, &frameFences_[currentBackBuffer_], VK_TRUE,
		UINT64_MAX);
	vkResetFences(device_, 1, &frameFences_[currentBackBuffer_]);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(commandBuffers_[currentBackBuffer_], &beginInfo);

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.framebuffer = framebuffer_[currentBackBuffer_];
	renderPassBeginInfo.renderArea.extent.width = 1024;
	renderPassBeginInfo.renderArea.extent.height = 768;
	renderPassBeginInfo.renderPass = renderPass_;

	VkClearValue clearValue = {};

	clearValue.color.float32[0] = 0.042f;
	clearValue.color.float32[1] = 0.042f;
	clearValue.color.float32[2] = 0.042f;
	clearValue.color.float32[3] = 1.0f;

	renderPassBeginInfo.pClearValues = &clearValue;
	renderPassBeginInfo.clearValueCount = 1;

	vkCmdBeginRenderPass(commandBuffers_[currentBackBuffer_],
		&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	render_cmdbuffer(commandBuffers_[currentBackBuffer_], 1024, 768);



	vkCmdEndRenderPass(commandBuffers_[currentBackBuffer_]);
	vkEndCommandBuffer(commandBuffers_[currentBackBuffer_]);

	// Submit rendering work to the graphics queue
	const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAcquiredSemaphore;
	submitInfo.pWaitDstStageMask = &waitDstStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers_[currentBackBuffer_];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderingCompleteSemaphore;
	vkQueueSubmit(queue_, 1, &submitInfo, VK_NULL_HANDLE);

	// Submit present operation to present queue
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderingCompleteSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain_;
	presentInfo.pImageIndices = &currentBackBuffer_;
	vkQueuePresentKHR(queue_, &presentInfo);

	vkQueueSubmit(queue_, 0, nullptr, frameFences_[currentBackBuffer_]);
	// Wait for all rendering to finish
	vkWaitForFences(device_, 3, frameFences_, VK_TRUE, UINT64_MAX);
}


/*
destroy the thousand handles to bullcrap
*/
void Graphics::destroy()
{
	vkDestroySemaphore(device_, imageAcquiredSemaphore, nullptr);
	vkDestroySemaphore(device_, renderingCompleteSemaphore, nullptr);

	vkDestroyPipeline(device_, pipeline_, nullptr);
	vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);

	vkDestroyBuffer(device_, vertexBuffer_, nullptr);
	vkDestroyBuffer(device_, indexBuffer_, nullptr);
	vkFreeMemory(device_, deviceBufferMemory_, nullptr);

	vkDestroyImageView(device_, ImageView_, nullptr);
	vkDestroyImage(device_, Image_, nullptr);
	vkFreeMemory(device_, deviceImageMemory_, nullptr);

	vkDestroyBuffer(device_, uploadImageBuffer_, nullptr);
	vkFreeMemory(device_, uploadImageMemory_, nullptr);

	vkDestroyBuffer(device_, uploadBufferBuffer_, nullptr);
	vkFreeMemory(device_, uploadBufferMemory_, nullptr);

	vkDestroyDescriptorSetLayout(device_, descriptorSetLayout_, nullptr);
	vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);

	vkDestroySampler(device_, sampler_, nullptr);

	vkDestroyShaderModule(device_, vertexShader_, nullptr);
	vkDestroyShaderModule(device_, fragmentShader_, nullptr);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		vkDestroyFence(device_, frameFences_[i], nullptr);
	}

	vkDestroyRenderPass(device_, renderPass_, nullptr);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		vkDestroyFramebuffer(device_, framebuffer_[i], nullptr);
		vkDestroyImageView(device_, swapChainImageViews_[i], nullptr);
	}

	vkDestroyCommandPool(device_, commandPool_, nullptr);

	vkDestroySwapchainKHR(device_, swapchain_, nullptr);
	vkDestroySurfaceKHR(instance_, surface_, nullptr);


	DestroyDebugReportCallback(instance_, callback, NULL);


	vkDestroyDevice(device_, nullptr);
	vkDestroyInstance(instance_, nullptr);
}

VkRenderPass Graphics::CreateRenderPass(VkDevice device, VkFormat swapchainFormat)
{
	VkAttachmentDescription attachmentDescription = {};
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.format = swapchainFormat;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkAttachmentReference attachmentReference = {};
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pColorAttachments = &attachmentReference;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.pAttachments = &attachmentDescription;

	VkRenderPass result = NULL;
	vkCreateRenderPass(device, &renderPassCreateInfo, NULL,
		&result);

	return result;
}

void Graphics::CreateFramebuffers(VkDevice device, VkRenderPass renderPass,
	const int width, const int height,
	const int count, const VkImageView* imageViews, VkFramebuffer* framebuffers)
{
	for (int i = 0; i < count; ++i)
	{
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments = &imageViews[i];
		framebufferCreateInfo.height = height;
		framebufferCreateInfo.width = width;
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.renderPass = renderPass;

		vkCreateFramebuffer(device, &framebufferCreateInfo, NULL,
			&framebuffers[i]);
	}
}

void Graphics::CreateSwapchainImageViews(VkDevice device, VkFormat format,
	const int count, const VkImage* images, VkImageView* imageViews)
{
	for (int i = 0; i < count; ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = images[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		vkCreateImageView(device, &imageViewCreateInfo, NULL,
			&imageViews[i]);
	}
}

VkSurfaceKHR Graphics::CreateSurface(VkInstance instance, HWND hwnd)
{
	VkWin32SurfaceCreateInfoKHR win32surfaceCreateInfo = {};
	win32surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32surfaceCreateInfo.hwnd = hwnd;
	win32surfaceCreateInfo.hinstance = ::GetModuleHandle(NULL);

	VkSurfaceKHR surface = NULL;
	vkCreateWin32SurfaceKHR(instance, &win32surfaceCreateInfo, NULL,
		&surface);

	return surface;
}

SwapchainFormatColorSpace Graphics::GetSwapchainFormatAndColorspace(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
		surface, &surfaceFormatCount, NULL);

	vector<VkSurfaceFormatKHR> surfaceFormats{ surfaceFormatCount };
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
		surface, &surfaceFormatCount, surfaceFormats.data());

	SwapchainFormatColorSpace result;

	if (surfaceFormatCount == 1 && surfaceFormats.front().format == VK_FORMAT_UNDEFINED)
	{
		result.format = VK_FORMAT_R8G8B8A8_UNORM;
	}
	else
	{
		result.format = surfaceFormats.front().format;
	}

	result.colorSpace = surfaceFormats.front().colorSpace;

	return result;
}

void Graphics::FindPhysicalDeviceWithGraphicsQueue(const vector<VkPhysicalDevice>& physicalDevices, VkPhysicalDevice* outputDevice, int* outputGraphicsQueueIndex)
{
	for (auto physicalDevice : physicalDevices)
	{
		uint32_t queueFamilyPropertyCount = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
			&queueFamilyPropertyCount, NULL);

		vector<VkQueueFamilyProperties> queueFamilyProperties{ queueFamilyPropertyCount };
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
			&queueFamilyPropertyCount, queueFamilyProperties.data());

		int i = 0;
		for (const auto& queueFamilyProperty : queueFamilyProperties)
		{
			if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				if (outputDevice)
				{
					*outputDevice = physicalDevice;
				}

				if (outputGraphicsQueueIndex)
				{
					*outputGraphicsQueueIndex = i;
				}

				return;
			}

			++i;
		}
	}
}

std::vector<const char*> GetDebugDeviceLayerNames(VkPhysicalDevice device)
{
	uint32_t layerCount = 0;
	vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr);

	std::vector<VkLayerProperties> deviceLayers{ layerCount };
	vkEnumerateDeviceLayerProperties(device, &layerCount, deviceLayers.data());

	std::vector<const char*> result;
	for (const auto& p : deviceLayers)
	{
		if (strcmp(p.layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
		{
			result.push_back("VK_LAYER_LUNARG_standard_validation");
		}
	}

	return result;
}


void Graphics::CreateDeviceAndQueue(VkInstance instance, VkDevice* outputDevice, VkQueue* outputQueue, int* outputQueueIndex, VkPhysicalDevice* outputPhysicalDevice)
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

	std::vector<VkPhysicalDevice> devices{ physicalDeviceCount };
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount,
		devices.data());

	VkPhysicalDevice physicalDevice = nullptr;
	int graphicsQueueIndex = -1;

	FindPhysicalDeviceWithGraphicsQueue(devices, &physicalDevice, &graphicsQueueIndex);

	assert(physicalDevice);

	VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
	deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.queueFamilyIndex = graphicsQueueIndex;

	static const float queuePriorities[] = { 1.0f };
	deviceQueueCreateInfo.pQueuePriorities = queuePriorities;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;

	std::vector<const char*> deviceLayers;

	auto debugDeviceLayerNames = GetDebugDeviceLayerNames(physicalDevice);
	deviceLayers.insert(deviceLayers.end(),
		debugDeviceLayerNames.begin(), debugDeviceLayerNames.end());


	deviceCreateInfo.ppEnabledLayerNames = deviceLayers.data();
	deviceCreateInfo.enabledLayerCount = static_cast<uint32_t> (deviceLayers.size());


	std::vector<const char*> deviceExtensions =
	{
		"VK_KHR_swapchain"
	};

	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t> (deviceExtensions.size());

	VkDevice device = nullptr;
	vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);

	VkQueue queue = nullptr;
	vkGetDeviceQueue(device, graphicsQueueIndex, 0, &queue);

	if (outputQueue)
	{
		*outputQueue = queue;
	}

	if (outputDevice)
	{
		*outputDevice = device;
	}

	if (outputQueueIndex)
	{
		*outputQueueIndex = graphicsQueueIndex;
	}

	if (outputPhysicalDevice)
	{
		*outputPhysicalDevice = physicalDevice;
	}
}

std::vector<const char*> GetDebugInstanceExtensionNames()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
		nullptr);

	std::vector<VkExtensionProperties> instanceExtensions{ extensionCount };
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
		instanceExtensions.data());

	std::vector<const char*> result;
	for (const auto& e : instanceExtensions)
	{
		if (strcmp(e.extensionName, "VK_EXT_debug_report") == 0)
		{
			result.push_back("VK_EXT_debug_report");
		}
	}

	return result;
}

std::vector<const char*> GetDebugInstanceLayerNames()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount,
		nullptr);

	std::vector<VkLayerProperties> instanceLayers{ layerCount };
	vkEnumerateInstanceLayerProperties(&layerCount,
		instanceLayers.data());

	std::vector<const char*> result;
	for (const auto& p : instanceLayers)
	{
		if (strcmp(p.layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
		{
			result.push_back("VK_LAYER_LUNARG_standard_validation");
		}
	}

	return result;
}

VkInstance Graphics::CreateInstance()
{
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	std::vector<const char*> instanceExtensions =
	{
		"VK_KHR_surface", "VK_KHR_win32_surface"
	};

	auto debugInstanceExtensionNames = GetDebugInstanceExtensionNames();
	instanceExtensions.insert(instanceExtensions.end(),
		debugInstanceExtensionNames.begin(), debugInstanceExtensionNames.end());

	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t> (instanceExtensions.size());

	std::vector<const char*> instanceLayers;

	auto debugInstanceLayerNames = GetDebugInstanceLayerNames();
	instanceLayers.insert(instanceLayers.end(),
		debugInstanceLayerNames.begin(), debugInstanceLayerNames.end());

	instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t> (instanceLayers.size());

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pApplicationName = "AMD Vulkan Sample application";
	applicationInfo.pEngineName = "AMD Vulkan Sample Engine";

	instanceCreateInfo.pApplicationInfo = &applicationInfo;

	VkInstance instance = VK_NULL_HANDLE;
	vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

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

	instance_ = CreateInstance();
	if (instance_ == VK_NULL_HANDLE)
	{
		// just bail out if the user does not have a compatible Vulkan driver
		return;
	}

	CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT");
	DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugReportCallbackEXT");


	callback = SetupDebugCallback(instance_);

	VkPhysicalDevice physicalDevice;
	CreateDeviceAndQueue(instance_, &device_, &queue_, &queueFamilyIndex_,
		&physicalDevice);
	physicalDevice_ = physicalDevice;


	surface_ = CreateSurface(instance_, hwnd);

	VkBool32 presentSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
		0, surface_, &presentSupported);

	VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
	swapchain_ = CreateSwapchain(physicalDevice, device_, surface_,
		width, height, QUEUE_SLOT_COUNT, &swapchainFormat);


	uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(device_, swapchain_,
		&swapchainImageCount, nullptr);

	vkGetSwapchainImagesKHR(device_, swapchain_,
		&swapchainImageCount, swapchainImages_);

	renderPass_ = CreateRenderPass(device_, swapchainFormat);

	CreateSwapchainImageViews(device_, swapchainFormat,
		QUEUE_SLOT_COUNT, swapchainImages_, swapChainImageViews_);
	CreateFramebuffers(device_, renderPass_, 1024, 768,
		QUEUE_SLOT_COUNT, swapChainImageViews_, framebuffer_);

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex_;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool(device_, &commandPoolCreateInfo, nullptr,
		&commandPool_);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = QUEUE_SLOT_COUNT + 1;
	commandBufferAllocateInfo.commandPool = commandPool_;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer commandBuffers[QUEUE_SLOT_COUNT + 1];

	vkAllocateCommandBuffers(device_, &commandBufferAllocateInfo,
		commandBuffers);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		commandBuffers_[i] = commandBuffers[i];
	}

	setupCommandBuffer_ = commandBuffers[QUEUE_SLOT_COUNT];

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		VkFenceCreateInfo fenceCreateInfo = {};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		// We need this so we can wait for them on the first try
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		vkCreateFence(device_, &fenceCreateInfo, nullptr, &frameFences_[i]);
	}


	vkResetFences(device_, 1, &frameFences_[0]);
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(setupCommandBuffer_, &beginInfo);

	CreateSampler();
	// load texture, will call gfx loadtexture, which will call CreateTexture below
	load_texture(*this, "media/menu.tga", false, false);
	//	CreateTexture(setupCommandBuffer_, image_width, image_height, image_data, image_size);
	CreateDescriptors();
	CreatePipelineStateObject();
	CreateMeshBuffers(setupCommandBuffer_);

	vkEndCommandBuffer(setupCommandBuffer_);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &setupCommandBuffer_;
	vkQueueSubmit(queue_, 1, &submitInfo, frameFences_[0]);

	vkWaitForFences(device_, 1, &frameFences_[0], VK_TRUE, UINT64_MAX);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore(device_, &semaphoreCreateInfo,
		nullptr, &imageAcquiredSemaphore);

	vkCreateSemaphore(device_, &semaphoreCreateInfo,
		nullptr, &renderingCompleteSemaphore);

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
	static bool once = true;

	if (once)
	{
		CreateTexture(setupCommandBuffer_, width, height, (unsigned char *)bytes, width * height * components * sizeof(char));
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
