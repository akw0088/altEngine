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
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	callbackCreateInfo.flags =
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = &DebugReportCallback;
	callbackCreateInfo.pUserData = hwnd;

	CreateDebugReportCallback(instance, &callbackCreateInfo, NULL, &fp);
}


void Graphics::CreateSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, const int surfaceWidth, const int surfaceHeight, const int backbufferCount, VkFormat* swapchainFormat, VkSwapchainKHR &swapchain)
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

	SwapchainFormatColorSpace swapchainFormatColorSpace;
	GetSwapchainFormatAndColorspace(physicalDevice, surface, swapchainFormatColorSpace);

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

	vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &swapchain);

	if (swapchainFormat)
	{
		*swapchainFormat = swapchainFormatColorSpace.format;
	}
}

void Graphics::AllocateBuffer(VkDevice device, const int size, const VkBufferUsageFlagBits bits, VkBuffer &buffer)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = static_cast<uint32_t> (size);
	bufferCreateInfo.usage = bits;

	VkResult err = vkCreateBuffer(device, &bufferCreateInfo, NULL, &buffer);
	assert(!err);
}



/*
Wrapper function, loops multiple gpu memory allocations
*/
int Graphics::AllocateMemory(VkDeviceMemory &deviceMemory, const vector<MemoryTypeInfo>& memoryInfos, VkDevice device, const int size, const uint32_t memoryBits, unsigned int memoryProperties, bool* isHostCoherent)
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

	vkCreateSampler(vk_device, &samplerCreateInfo, NULL, &sampler_);
}

/*
Sets up GPU texture buffer, copies data
*/
void Graphics::CreateTexture(int width, int height, int components, int format, unsigned char *image_data, bool clamp)
{
	VkDeviceMemory uploadImageMemory_ = VK_NULL_HANDLE;
	VkBuffer uploadImageBuffer_ = VK_NULL_HANDLE;
	int image_size = width * height * components * sizeof(char);


	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = NULL;
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

	vkCreateImage(vk_device, &imageCreateInfo, NULL, &Image_);

	VkMemoryRequirements requirements = {};
	vkGetImageMemoryRequirements(vk_device, Image_, &requirements);

	VkDeviceSize requiredSizeForImage = requirements.size;

	auto memoryHeaps = EnumerateHeaps(physicalDevice_);
	AllocateMemory(deviceImageMemory_, memoryHeaps, vk_device, static_cast<int> (requiredSizeForImage), requirements.memoryTypeBits, MT_DeviceLocal);

	vkBindImageMemory(vk_device, Image_, deviceImageMemory_, 0);

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	bufferCreateInfo.queueFamilyIndexCount = 1;
	bufferCreateInfo.pQueueFamilyIndices = &queueFamilyIndex;
	bufferCreateInfo.size = requiredSizeForImage;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VkResult err = vkCreateBuffer(vk_device, &bufferCreateInfo, NULL, &uploadImageBuffer_);
	assert(!err);

	vkGetBufferMemoryRequirements(vk_device, uploadImageBuffer_, &requirements);

	VkDeviceSize requiredSizeForBuffer = requirements.size;

	bool memoryIsHostCoherent = false;
	AllocateMemory(uploadImageMemory_, memoryHeaps, vk_device, static_cast<int> (requiredSizeForBuffer), requirements.memoryTypeBits, MT_HostVisible, &memoryIsHostCoherent);

	vkBindBufferMemory(vk_device, uploadImageBuffer_, uploadImageMemory_, 0);

	void* data = NULL;
	vkMapMemory(vk_device, uploadImageMemory_, 0, VK_WHOLE_SIZE, 0, &data);
	memcpy(data, image_data, image_size);

	if (!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = uploadImageMemory_;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(vk_device, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(vk_device, uploadImageMemory_);

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
	imageBarrier.image = Image_;
	imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBarrier.subresourceRange.layerCount = 1;
	imageBarrier.subresourceRange.levelCount = 1;

	vkCmdPipelineBarrier(setupCommandBuffer_, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);

	vkCmdCopyBufferToImage(setupCommandBuffer_, uploadImageBuffer_, Image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

	imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(setupCommandBuffer_, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.format = imageCreateInfo.format;
	imageViewCreateInfo.image = Image_;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	vkCreateImageView(vk_device, &imageViewCreateInfo, NULL, &ImageView_);
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

	vkCreateDescriptorSetLayout(vk_device, descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout_);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout_;
	pipelineLayoutCreateInfo.setLayoutCount = 1;

	vkCreatePipelineLayout(vk_device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout_);

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

	vkCreateDescriptorPool(vk_device, &descriptorPoolCreateInfo, NULL, &descriptorPool_);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout_;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool_;

	vkAllocateDescriptorSets(vk_device, &descriptorSetAllocateInfo, &descriptorSet_);

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

	vkUpdateDescriptorSets(vk_device, 1, writeDescriptorSets, 0, NULL);
}

/*
Copies shader to GPU
*/
void Graphics::LoadShader(VkDevice device, const void* shaderContents, const size_t size, VkShaderModule &shader)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	shaderModuleCreateInfo.pCode = static_cast<const uint32_t*> (shaderContents);
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
void Graphics::CreatePipelineStateObject()
{
	LoadShader(vk_device, BasicVertexShader, sizeof(BasicVertexShader), vertexShader_);
	LoadShader(vk_device, TexturedFragmentShader, sizeof(TexturedFragmentShader), fragmentShader_);

	CreatePipeline(vk_device, renderPass_, pipelineLayout_,	vertexShader_, fragmentShader_, pipeline_);
}

/*
Full screen quad vertex / index buffers, single upload is kind of interesting
*/
void Graphics::CreateMeshBuffers(VkCommandBuffer uploadCommandBuffer, vertex_t *vertices, int num_vertex, unsigned int *indices, int num_index)
{
	auto memoryHeaps = EnumerateHeaps(physicalDevice_);
	VkBufferUsageFlagBits index_flag;
	index_flag = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	VkBufferUsageFlagBits vertex_flag;
	vertex_flag = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	AllocateBuffer(vk_device, sizeof(unsigned int) * num_index, index_flag, indexBuffer_);
	AllocateBuffer(vk_device, sizeof(vertex_t) * num_vertex, vertex_flag, vertexBuffer_);

	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, vertexBuffer_, &vertexBufferMemoryRequirements);
	VkMemoryRequirements indexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, indexBuffer_, &indexBufferMemoryRequirements);

	VkDeviceSize bufferSize = vertexBufferMemoryRequirements.size;
	// We want to place the index buffer behind the vertex buffer. Need to take
	// the alignment into account to find the next suitable location
	VkDeviceSize indexBufferOffset = RoundToNextMultiple(bufferSize, indexBufferMemoryRequirements.alignment);

	bufferSize = indexBufferOffset + indexBufferMemoryRequirements.size;
	AllocateMemory(deviceMemory_, memoryHeaps, vk_device, static_cast<int>(bufferSize), vertexBufferMemoryRequirements.memoryTypeBits & indexBufferMemoryRequirements.memoryTypeBits, MT_DeviceLocal);

	vkBindBufferMemory(vk_device, vertexBuffer_, deviceMemory_, 0);
	vkBindBufferMemory(vk_device, indexBuffer_, deviceMemory_, indexBufferOffset);

	AllocateBuffer(vk_device, static_cast<int> (bufferSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, uploadBuffer_);
	VkMemoryRequirements uploadBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, uploadBuffer_, &uploadBufferMemoryRequirements);

	bool memoryIsHostCoherent = false;
	AllocateMemory(uploadMemory_, memoryHeaps, vk_device, static_cast<int>(uploadBufferMemoryRequirements.size), vertexBufferMemoryRequirements.memoryTypeBits & indexBufferMemoryRequirements.memoryTypeBits,	MT_HostVisible, &memoryIsHostCoherent);

	vkBindBufferMemory(vk_device, uploadBuffer_, uploadMemory_, 0);

	void* mapping = NULL;
	vkMapMemory(vk_device, uploadMemory_, 0, VK_WHOLE_SIZE, 0, &mapping);
	memcpy(mapping, vertices, sizeof(vertex_t) * num_vertex);

	memcpy(static_cast<uint8_t*> (mapping) + indexBufferOffset, indices, sizeof(unsigned int) * num_index);

	if (!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = uploadMemory_;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(vk_device, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(vk_device, uploadMemory_);

	VkBufferCopy vertexCopy = {};
	vertexCopy.size = sizeof(vertex_t) * num_vertex;

	VkBufferCopy indexCopy = {};
	indexCopy.size = sizeof(unsigned int) * num_index;
	indexCopy.srcOffset = indexBufferOffset;

	vkCmdCopyBuffer(uploadCommandBuffer, uploadBuffer_, vertexBuffer_, 1, &vertexCopy);
	vkCmdCopyBuffer(uploadCommandBuffer, uploadBuffer_, indexBuffer_, 1, &indexCopy);

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

	vkCmdPipelineBarrier(uploadCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, NULL, 1, uploadBarriers, 0, NULL);
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
		pipelineLayout_, 0, 1, &descriptorSet_, 0, NULL);

	vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
}

void Graphics::render()
{
	vkAcquireNextImageKHR(vk_device, swapchain_, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE, &currentBackBuffer_);

	vkWaitForFences(vk_device, 1, &frameFences_[currentBackBuffer_], VK_TRUE, UINT64_MAX);
	vkResetFences(vk_device, 1, &frameFences_[currentBackBuffer_]);

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

	vkQueueSubmit(queue_, 0, NULL, frameFences_[currentBackBuffer_]);
	// Wait for all rendering to finish
	vkWaitForFences(vk_device, 3, frameFences_, VK_TRUE, UINT64_MAX);
}


/*
destroy the thousand handles to bullcrap
*/
void Graphics::destroy()
{
	vkDestroySemaphore(vk_device, imageAcquiredSemaphore, NULL);
	vkDestroySemaphore(vk_device, renderingCompleteSemaphore, NULL);

	vkDestroyPipeline(vk_device, pipeline_, NULL);
	vkDestroyPipelineLayout(vk_device, pipelineLayout_, NULL);

	vkDestroyBuffer(vk_device, vertexBuffer_, NULL);
	vkDestroyBuffer(vk_device, indexBuffer_, NULL);
	vkFreeMemory(vk_device, deviceMemory_, NULL);

	vkDestroyImageView(vk_device, ImageView_, NULL);
	vkDestroyImage(vk_device, Image_, NULL);
	vkFreeMemory(vk_device, deviceImageMemory_, NULL);

	vkDestroyBuffer(vk_device, uploadImageBuffer_, NULL);
	vkFreeMemory(vk_device, uploadImageMemory_, NULL);

	vkDestroyBuffer(vk_device, uploadBuffer_, NULL);
	vkFreeMemory(vk_device, uploadMemory_, NULL);

	vkDestroyDescriptorSetLayout(vk_device, descriptorSetLayout_, NULL);
	vkDestroyDescriptorPool(vk_device, descriptorPool_, NULL);

	vkDestroySampler(vk_device, sampler_, NULL);

	vkDestroyShaderModule(vk_device, vertexShader_, NULL);
	vkDestroyShaderModule(vk_device, fragmentShader_, NULL);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		vkDestroyFence(vk_device, frameFences_[i], NULL);
	}

	vkDestroyRenderPass(vk_device, renderPass_, NULL);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		vkDestroyFramebuffer(vk_device, framebuffer_[i], NULL);
		vkDestroyImageView(vk_device, swapChainImageViews_[i], NULL);
	}

	vkDestroyCommandPool(vk_device, commandPool_, NULL);

	vkDestroySwapchainKHR(vk_device, swapchain_, NULL);
	vkDestroySurfaceKHR(vk_instance, surface_, NULL);


	DestroyDebugReportCallback(vk_instance, callback, NULL);


	vkDestroyDevice(vk_device, NULL);
	vkDestroyInstance(vk_instance, NULL);
}

void Graphics::CreateRenderPass(VkDevice device, VkFormat swapchainFormat, VkRenderPass &rp)
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

	vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &rp);
}

void Graphics::CreateFramebuffers(VkDevice device, VkRenderPass renderPass, const int width, const int height, const int count, const VkImageView* imageViews, VkFramebuffer* framebuffers)
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

		vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, &framebuffers[i]);
	}
}

void Graphics::CreateSwapchainImageViews(VkDevice device, VkFormat format, const int count, const VkImage* images, VkImageView* imageViews)
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

 void Graphics::CreateSurface(VkInstance instance, HWND hwnd, VkSurfaceKHR &surface)
{
	VkWin32SurfaceCreateInfoKHR win32surfaceCreateInfo = {};
	win32surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32surfaceCreateInfo.hwnd = hwnd;
	win32surfaceCreateInfo.hinstance = ::GetModuleHandle(NULL);

	vkCreateWin32SurfaceKHR(instance, &win32surfaceCreateInfo, NULL, &surface);
}

void Graphics::GetSwapchainFormatAndColorspace(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapchainFormatColorSpace &result)
{
	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
		surface, &surfaceFormatCount, NULL);

	vector<VkSurfaceFormatKHR> surfaceFormats{ surfaceFormatCount };
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
		surface, &surfaceFormatCount, surfaceFormats.data());

	if (surfaceFormatCount == 1 && surfaceFormats.front().format == VK_FORMAT_UNDEFINED)
	{
		result.format = VK_FORMAT_R8G8B8A8_UNORM;
	}
	else
	{
		result.format = surfaceFormats.front().format;
	}

	result.colorSpace = surfaceFormats.front().colorSpace;
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
	vkEnumerateDeviceLayerProperties(device, &layerCount, NULL);

	std::vector<VkLayerProperties> deviceLayers{ layerCount };
	vkEnumerateDeviceLayerProperties(device, &layerCount, deviceLayers.data());

	std::vector<const char*> result;
	for (const auto& p : deviceLayers)
	{
		if (strcmp(p.layerName, "VK_LAYER_LUNARG_standard_validation") == 0)
		{
//			result.push_back("VK_LAYER_LUNARG_standard_validation");
		}
	}

	return result;
}


void Graphics::CreateDeviceAndQueue(VkInstance instance, VkDevice* outputDevice, VkQueue* outputQueue, int* outputQueueIndex, VkPhysicalDevice* outputPhysicalDevice)
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);

	std::vector<VkPhysicalDevice> devices{ physicalDeviceCount };
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, devices.data());

	VkPhysicalDevice physicalDevice = NULL;
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

	VkPhysicalDeviceFeatures features;

	memset(&features, 0, sizeof(features));
	features.shaderClipDistance = true;

	deviceCreateInfo.pEnabledFeatures = &features;

	std::vector<const char*> deviceLayers;

	auto debugDeviceLayerNames = GetDebugDeviceLayerNames(physicalDevice);
	deviceLayers.insert(deviceLayers.end(), debugDeviceLayerNames.begin(), debugDeviceLayerNames.end());


	deviceCreateInfo.ppEnabledLayerNames = deviceLayers.data();
	deviceCreateInfo.enabledLayerCount = static_cast<uint32_t> (deviceLayers.size());


	std::vector<const char*> deviceExtensions =
	{
		"VK_KHR_swapchain"
	};

	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t> (deviceExtensions.size());

	VkDevice device = NULL;
	vkCreateDevice(physicalDevice, &deviceCreateInfo, NULL, &device);

	VkQueue queue = NULL;
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
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

	std::vector<VkExtensionProperties> instanceExtensions{ extensionCount };
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, instanceExtensions.data());

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
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	std::vector<VkLayerProperties> instanceLayers{ layerCount };
	vkEnumerateInstanceLayerProperties(&layerCount, instanceLayers.data());

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
	instanceExtensions.insert(instanceExtensions.end(), debugInstanceExtensionNames.begin(), debugInstanceExtensionNames.end());

	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t> (instanceExtensions.size());

	std::vector<const char*> instanceLayers;

	auto debugInstanceLayerNames = GetDebugInstanceLayerNames();
	instanceLayers.insert(instanceLayers.end(), debugInstanceLayerNames.begin(), debugInstanceLayerNames.end());

	instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t> (instanceLayers.size());

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pApplicationName = "Vulkan";
	applicationInfo.pEngineName = "Vulkan";

	instanceCreateInfo.pApplicationInfo = &applicationInfo;

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

	vk_instance = CreateInstance();
	if (vk_instance == VK_NULL_HANDLE)
	{
		printf("Unable to create Vulkan instance\r\n");
		return;
	}

	CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vk_instance, "vkCreateDebugReportCallbackEXT");
	DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vk_instance, "vkDestroyDebugReportCallbackEXT");


	SetupDebugCallback(vk_instance, callback);

	VkPhysicalDevice physicalDevice;
	CreateDeviceAndQueue(vk_instance, &vk_device, &queue_, &queueFamilyIndex_, &physicalDevice);
	physicalDevice_ = physicalDevice;


	CreateSurface(vk_instance, hwnd, surface_);

	VkBool32 presentSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface_, &presentSupported);

	VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
	CreateSwapchain(physicalDevice, vk_device, surface_, width, height, QUEUE_SLOT_COUNT, &swapchainFormat, swapchain_);


	uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(vk_device, swapchain_, &swapchainImageCount, NULL);

	vkGetSwapchainImagesKHR(vk_device, swapchain_, &swapchainImageCount, swapchainImages_);

	CreateRenderPass(vk_device, swapchainFormat, renderPass_);

	CreateSwapchainImageViews(vk_device, swapchainFormat, QUEUE_SLOT_COUNT, swapchainImages_, swapChainImageViews_);
	CreateFramebuffers(vk_device, renderPass_, 1024, 768, QUEUE_SLOT_COUNT, swapChainImageViews_, framebuffer_);

	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex_;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	vkCreateCommandPool(vk_device, &commandPoolCreateInfo, NULL, &commandPool_);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = QUEUE_SLOT_COUNT + 1;
	commandBufferAllocateInfo.commandPool = commandPool_;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer commandBuffers[QUEUE_SLOT_COUNT + 1];

	vkAllocateCommandBuffers(vk_device, &commandBufferAllocateInfo, commandBuffers);

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

		vkCreateFence(vk_device, &fenceCreateInfo, NULL, &frameFences_[i]);
	}


	vkResetFences(vk_device, 1, &frameFences_[0]);
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(setupCommandBuffer_, &beginInfo);


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
	CreatePipelineStateObject();
	CreateMeshBuffers(setupCommandBuffer_, vertex_array, 4, index_array, 6);

	vkEndCommandBuffer(setupCommandBuffer_);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &setupCommandBuffer_;
	vkQueueSubmit(queue_, 1, &submitInfo, frameFences_[0]);

	vkWaitForFences(vk_device, 1, &frameFences_[0], VK_TRUE, UINT64_MAX);

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	vkCreateSemaphore(vk_device, &semaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
	vkCreateSemaphore(vk_device, &semaphoreCreateInfo, NULL, &renderingCompleteSemaphore);
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
	return 0;
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = QUEUE_SLOT_COUNT + 1;
	commandBufferAllocateInfo.commandPool = commandPool_;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;


	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkCommandBuffer i_setupCommandBuffer_;

	VkCommandBuffer i_commandBuffers[QUEUE_SLOT_COUNT + 1];
	vkAllocateCommandBuffers(vk_device, &commandBufferAllocateInfo, i_commandBuffers);

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		i_commandBuffers_[i] = i_commandBuffers[i];
	}

	i_setupCommandBuffer_ = i_commandBuffers[QUEUE_SLOT_COUNT];



	auto memoryHeaps = EnumerateHeaps(physicalDevice_);

	VkBufferUsageFlagBits bflag;
	bflag = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	AllocateBuffer(vk_device, sizeof(int) * num_index, bflag, indexBuffer_);

	VkMemoryRequirements indexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, indexBuffer_, &indexBufferMemoryRequirements);

	VkDeviceSize bufferSize = indexBufferMemoryRequirements.size;

	AllocateMemory(deviceMemory_, memoryHeaps, vk_device, static_cast<int>(bufferSize), indexBufferMemoryRequirements.memoryTypeBits, MT_DeviceLocal);

	vkBindBufferMemory(vk_device, indexBuffer_, deviceMemory_, 0);

	AllocateBuffer(vk_device, static_cast<int> (bufferSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, uploadBuffer_);
	VkMemoryRequirements uploadBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, uploadBuffer_, &uploadBufferMemoryRequirements);

	bool memoryIsHostCoherent = false;
	AllocateMemory(uploadMemory_, memoryHeaps, vk_device, static_cast<int>(uploadBufferMemoryRequirements.size), indexBufferMemoryRequirements.memoryTypeBits, MT_HostVisible, &memoryIsHostCoherent);

	vkBindBufferMemory(vk_device, uploadBuffer_, uploadMemory_, 0);

	void* mapping = NULL;
	vkMapMemory(vk_device, uploadMemory_, 0, VK_WHOLE_SIZE, 0, &mapping);
	memcpy(mapping, index_buffer, sizeof(int) * num_index);

	if (!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = uploadMemory_;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(vk_device, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(vk_device, uploadMemory_);

	VkBufferCopy indexCopy = {};
	indexCopy.size = sizeof(int) * num_index;

	vkCmdCopyBuffer(i_setupCommandBuffer_, uploadBuffer_, indexBuffer_, 1, &indexCopy);

	VkBufferMemoryBarrier uploadBarriers[1] = {};
	uploadBarriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	uploadBarriers[0].buffer = vertexBuffer_;
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

int Graphics::CreateVertexBuffer(void *vertex_buffer, int num_vertex, bool)
{
	return 0;
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandBufferCount = QUEUE_SLOT_COUNT + 1;
	commandBufferAllocateInfo.commandPool = commandPool_;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;


	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(setupCommandBuffer_, &beginInfo);

	VkCommandBuffer v_commandBuffers[QUEUE_SLOT_COUNT + 1];
	VkCommandBuffer v_setupCommandBuffer_;

	vkAllocateCommandBuffers(vk_device, &commandBufferAllocateInfo, v_commandBuffers);

	v_setupCommandBuffer_ = v_commandBuffers[QUEUE_SLOT_COUNT];

	vkBeginCommandBuffer(v_setupCommandBuffer_, &beginInfo);

	auto memoryHeaps = EnumerateHeaps(physicalDevice_);

	AllocateBuffer(vk_device, sizeof(vertex_t) * num_vertex, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vertexBuffer_);

	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, vertexBuffer_, &vertexBufferMemoryRequirements);

	VkDeviceSize bufferSize = vertexBufferMemoryRequirements.size;

	AllocateMemory(deviceMemory_, memoryHeaps, vk_device, static_cast<int>(bufferSize), vertexBufferMemoryRequirements.memoryTypeBits, MT_DeviceLocal);

	vkBindBufferMemory(vk_device, vertexBuffer_, deviceMemory_, 0);

	AllocateBuffer(vk_device, static_cast<int> (bufferSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, uploadBuffer_);
	VkMemoryRequirements uploadBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(vk_device, uploadBuffer_, &uploadBufferMemoryRequirements);

	bool memoryIsHostCoherent = false;
	AllocateMemory(uploadMemory_, memoryHeaps, vk_device, static_cast<int>(uploadBufferMemoryRequirements.size), vertexBufferMemoryRequirements.memoryTypeBits, MT_HostVisible, &memoryIsHostCoherent);

	vkBindBufferMemory(vk_device, uploadBuffer_, uploadMemory_, 0);

	void* mapping = NULL;
	vkMapMemory(vk_device, uploadMemory_, 0, VK_WHOLE_SIZE, 0, &mapping);
	memcpy(mapping, vertex_buffer, sizeof(vertex_t) * num_vertex);

	if (!memoryIsHostCoherent)
	{
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = uploadMemory_;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		vkFlushMappedMemoryRanges(vk_device, 1, &mappedMemoryRange);
	}

	vkUnmapMemory(vk_device, uploadMemory_);

	VkBufferCopy vertexCopy = {};
	vertexCopy.size = sizeof(vertex_t) * num_vertex;

	vkCmdCopyBuffer(v_uploadCommandBuffer, uploadBuffer_, vertexBuffer_, 1, &vertexCopy);

	VkBufferMemoryBarrier uploadBarriers[1] = {};
	uploadBarriers[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	uploadBarriers[0].buffer = vertexBuffer_;
	uploadBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	uploadBarriers[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	uploadBarriers[0].dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	uploadBarriers[0].size = VK_WHOLE_SIZE;

	vkCmdPipelineBarrier(v_uploadCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 1, uploadBarriers, 0, NULL);
	vkEndCommandBuffer(v_uploadCommandBuffer);
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
#endif
