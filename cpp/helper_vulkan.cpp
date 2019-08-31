#include "stdafx.h"

#ifdef VK_TRUE

#pragma comment(lib, "vulkan-1.lib")
static const uint32_t descriptorPoolSize = 64;

static const VkComponentMapping colorComponentMapping = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
static const VkComponentMapping depthComponentMapping = {};
static const VkClearValue clearValues[2] = { { 0.0f, 0.0f, 0.0f },{ 1.0f, 0u } };

DeviceManVK deviceMan;

VkResult _afHandleVKError(const char* file, const char* func, int line, const char* command, VkResult result)
{
	const char *err = nullptr;
	switch (result)
	{
#define E(er) case er: err = #er; break
		E(VK_INCOMPLETE);
		E(VK_ERROR_VALIDATION_FAILED_EXT);
#undef E
	default:
		aflog("%s %s(%d): err=%d %s\n", file, func, line, result, command);
		assert(0);
		return result;
	case VK_SUCCESS:
		return VK_SUCCESS;
	}
	aflog("%s %s(%d): %s %s\n", file, func, line, err, command);
	assert(0);
	return result;
}

static VkShaderModule CreateShaderModule(VkDevice device, const char* fileName)
{
	int size;
	void* file = LoadFile(fileName, &size);
	assert(file);
	VkShaderModuleCreateInfo info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, (size_t)size, (uint32_t*)file };
	VkShaderModule module = 0;
	afHandleVKError(vkCreateShaderModule(device, &info, nullptr, &module));
	free(file);
	return module;
}

static uint32_t GetCompatibleMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t typeBits, VkMemoryPropertyFlags propertyFlags)
{
	for (uint32_t i = 0; i < (int)memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
		{
			return i;
		}
	}
	afVerify(0);
	return 0;	// Unreachable code
}

void afSafeDeleteBuffer(BufferContext& buffer)
{
	afSafeUnmapVk(buffer.device, buffer.memory, buffer.mappedMemory);
	afSafeDeleteVk(vkDestroyBuffer, buffer.device, buffer.buffer);
	afSafeDeleteVk(vkFreeMemory, buffer.device, buffer.memory);
}

void afWriteBuffer(BufferContext& buffer, int size, const void* srcData)
{
	assert(buffer.mappedMemory);
	memcpy(buffer.mappedMemory, srcData, size);
}

static VkBufferUsageFlagBits BufferTypeToBufferUsageFlagBits(AFBufferType bufferType)
{
	switch (bufferType)
	{
	case AFBT_VERTEX:
	case AFBT_VERTEX_CPUWRITE:
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	case AFBT_INDEX:
		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	case AFBT_CONSTANT:
	case AFBT_CONSTANT_CPUWRITE:
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	assert(0);
	return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
}

static BufferContext afCreateBufferInternal(int size, const void* srcData, VkBufferUsageFlags usage)
{
	VkDevice device = deviceMan.GetDevice();
	const VkBufferCreateInfo bufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, (VkDeviceSize)size, usage };
	BufferContext buffer;
	buffer.device = device;
	afHandleVKError(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer.buffer));
	vkGetBufferMemoryRequirements(device, buffer.buffer, &buffer.memoryRequirement);
	const VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, buffer.memoryRequirement.size, GetCompatibleMemoryTypeIndex(deviceMan.physicalDeviceMemoryProperties, buffer.memoryRequirement.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) };
	afHandleVKError(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &buffer.memory));
	afHandleVKError(vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0));
	afHandleVKError(vkMapMemory(device, buffer.memory, 0, size, 0, &buffer.mappedMemory));
	buffer.size = size;
	if (srcData)
	{
		afWriteBuffer(buffer, size, srcData);
	}
	return buffer;
}

AFBufferResource afCreateBuffer(int size, const void* srcData, AFBufferType bufferType)
{
	return afCreateBufferInternal(size, srcData, BufferTypeToBufferUsageFlagBits(bufferType));
}

void afWriteTexture(AFTexRef textureContext, const TexDesc& texDesc, void *image)
{
	void* mappedMemory = nullptr;
	int size = texDesc.size.x * texDesc.size.y * 4;
	afHandleVKError(vkMapMemory(textureContext->device, textureContext->memory, 0, size, 0, &mappedMemory));
	assert(mappedMemory);
	memcpy(mappedMemory, image, size);
	vkUnmapMemory(textureContext->device, textureContext->memory);
}

void afWriteTexture(AFTexRef textureContext, const TexDesc& texDesc, int mipCount, const AFTexSubresourceData datas[])
{
	const uint32_t maxSubresources = 100;
	const uint32_t subResources = mipCount * texDesc.arraySize;
	VkBufferImageCopy copyInfo[maxSubresources];
	assert(subResources <= dimof(copyInfo));
	VkDeviceSize total = 0;

	for (uint32_t i = 0; i < subResources; i++)
	{
		uint32_t mipIdx = i % mipCount;
		copyInfo[i] = { total, 0, 0,{ VK_IMAGE_ASPECT_COLOR_BIT, mipIdx, i / mipCount, 1 },{},{ (uint32_t)texDesc.size.x >> mipIdx, (uint32_t)texDesc.size.y >> mipIdx, 1 } };
		total += datas[i].pitchSlice;
	}

	BufferContext staging = afCreateBufferInternal((int)total, nullptr, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	for (uint32_t i = 0; i < subResources; i++)
	{
		memcpy((uint8_t*)staging.mappedMemory + copyInfo[i].bufferOffset, datas[i].ptr, datas[i].pitchSlice);
	}

	VkCommandBuffer cmd = deviceMan.commandBuffer;
	const VkImageMemoryBarrier undefToDest = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, textureContext->image,{ VK_IMAGE_ASPECT_COLOR_BIT, 0, (uint32_t)mipCount, 0, (texDesc.isCubeMap ? 6u : 1u) } };
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &undefToDest);
	vkCmdCopyBufferToImage(cmd, staging.buffer, textureContext->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subResources, copyInfo);
	const VkImageMemoryBarrier destToRead = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, undefToDest.dstAccessMask, VK_ACCESS_SHADER_READ_BIT, undefToDest.newLayout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, textureContext->image,{ VK_IMAGE_ASPECT_COLOR_BIT, 0, (uint32_t)mipCount, 0, (texDesc.isCubeMap ? 6u : 1u) } };
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &destToRead);
	deviceMan.Flush();
	afSafeDeleteBuffer(staging);
}

static void CreateTextureDescriptorSet(TextureContext& textureContext)
{
	VkDevice device = deviceMan.GetDevice();
	const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, deviceMan.descriptorPool, 1, &deviceMan.commonTextureDescriptorSetLayout };
	afHandleVKError(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &textureContext.descriptorSet));
	const VkDescriptorImageInfo descriptorImageInfo = { deviceMan.sampler, textureContext.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	const VkWriteDescriptorSet writeDescriptorSets[] =
	{
		{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, textureContext.descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfo },
	};
	vkUpdateDescriptorSets(device, arrayparam(writeDescriptorSets), 0, nullptr);
}

static VkImageAspectFlags FormatToAspectFlags(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	return VK_IMAGE_ASPECT_COLOR_BIT;
}

AFTexRef afCreateDynamicTexture(VkFormat format, const IVec2& size, uint32_t flags)
{
	//	VkFormatProperties formatProperties;
	//	vkGetPhysicalDeviceFormatProperties(deviceMan.physicalDevice, format, &formatProperties);
	//	assert(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

	VkDevice device = deviceMan.GetDevice();

	AFTexRef textureContext(new TextureContext);
	textureContext->device = device;
	textureContext->format = format;
	textureContext->texDesc.size = size;
	textureContext->mipCount = 1;

	VkImageCreateInfo TextureCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0, VK_IMAGE_TYPE_2D, format,{ (uint32_t)size.x, (uint32_t)size.y, 1 }, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, 0, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED };
	if (flags & AFTF_CPU_WRITE)
	{
		TextureCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	}
	if (flags & AFTF_SRV)
	{
		TextureCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	if (flags & AFTF_RTV)
	{
		TextureCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (flags & AFTF_DSV)
	{
		TextureCreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	afHandleVKError(vkCreateImage(device, &TextureCreateInfo, nullptr, &textureContext->image));

	VkMemoryRequirements req = {};
	vkGetImageMemoryRequirements(device, textureContext->image, &req);
	const VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, req.size, GetCompatibleMemoryTypeIndex(deviceMan.physicalDeviceMemoryProperties, req.memoryTypeBits, (flags & AFTF_CPU_WRITE) ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) };
	afHandleVKError(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &textureContext->memory));

	afHandleVKError(vkBindImageMemory(device, textureContext->image, textureContext->memory, 0));

	VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, textureContext->image, VK_IMAGE_VIEW_TYPE_2D, format, (flags & AFTF_DSV) ? depthComponentMapping : colorComponentMapping,{ FormatToAspectFlags(format), 0, 1, 0, 1 } };
	afHandleVKError(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &textureContext->view));

	if (flags & AFTF_SRV)
	{
		CreateTextureDescriptorSet(*textureContext);
	}
	if (flags & AFTF_RTV)
	{
	}
	if (flags & AFTF_CPU_WRITE)
	{
		vkCmdPipelineBarrier(deviceMan.commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1, ToPtr<VkImageMemoryBarrier>({ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0, VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, textureContext->image,{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1u, 0, 1u } }));
	}
	return textureContext;
}

AFTexRef afCreateTexture2D(AFFormat format, const struct TexDesc& desc, int mipCount, const AFTexSubresourceData datas[])
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(deviceMan.physicalDevice, format, &formatProperties);
	assert(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);

	AFTexRef textureContext(new TextureContext);
	VkDevice device = deviceMan.GetDevice();
	textureContext->device = device;
	textureContext->format = format;
	textureContext->texDesc = desc;
	textureContext->mipCount = mipCount;
	const VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, (desc.isCubeMap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u), VK_IMAGE_TYPE_2D, format,{ (uint32_t)desc.size.x, (uint32_t)desc.size.y, 1 }, (uint32_t)mipCount, desc.isCubeMap ? 6u : 1u, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED };
	afHandleVKError(vkCreateImage(device, &imageCreateInfo, nullptr, &textureContext->image));

	VkMemoryRequirements req = {};
	vkGetImageMemoryRequirements(device, textureContext->image, &req);
	const VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, req.size, GetCompatibleMemoryTypeIndex(deviceMan.physicalDeviceMemoryProperties, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) };
	afHandleVKError(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &textureContext->memory));

	afHandleVKError(vkBindImageMemory(device, textureContext->image, textureContext->memory, 0));

	VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, textureContext->image,  (desc.isCubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D), format, colorComponentMapping,{ VK_IMAGE_ASPECT_COLOR_BIT, 0, (uint32_t)mipCount, 0, desc.isCubeMap ? 6u : 1u } };
	afHandleVKError(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &textureContext->view));

	CreateTextureDescriptorSet(*textureContext);
	if (datas)
	{
		afWriteTexture(textureContext, desc, mipCount, datas);
	}
	return textureContext;
}

TextureContext::~TextureContext()
{
	if (descriptorSet)
	{
		afHandleVKError(vkFreeDescriptorSets(device, deviceMan.descriptorPool, 1, &descriptorSet));
		descriptorSet = 0;
	}
	afSafeDeleteVk(vkDestroyImageView, device, view);
	afSafeDeleteVk(vkDestroyImage, device, image);
	afSafeDeleteVk(vkFreeMemory, device, memory);
}

void afBindBuffer(VkPipelineLayout pipelineLayout, int size, const void* buf, int descritorSetIndex)
{
	AFBufferStackAllocator& ubo = deviceMan.uboAllocator;
	uint32_t dynamicOffset = ubo.Allocate(size, buf);
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, descritorSetIndex, 1, &deviceMan.commonUboDescriptorSet, 1, &dynamicOffset);
}

void afBindTexture(VkPipelineLayout pipelineLayout, AFTexRef textureContext, int descritorSetIndex)
{
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, descritorSetIndex, 1, &textureContext->descriptorSet, 0, nullptr);
}

void afSetVertexBuffer(int size, const void* buffer, int stride)
{
	(void)stride;
	AFBufferStackAllocator& allocator = deviceMan.vertexBufferAllocator;
	VkDeviceSize offset = allocator.Allocate(size, buffer);
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &allocator.bufferContext.buffer, &offset);
}

void afSetVertexBuffer(AFBufferResource vertexBuffer, int stride)
{
	(void)stride;
	VkDeviceSize offsets[1] = {};
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
}

void afSetIndexBuffer(AFBufferResource indexBuffer)
{
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, AFIndexTypeToDevice);
}

void afDrawIndexed(int numIndices, int start, int instanceCount)
{
	afVerify(!start);
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdDrawIndexed(commandBuffer, numIndices, instanceCount, 0, 0, 0);
}

void afDraw(int numVertices, int start, int instanceCount)
{
	afVerify(!start);
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdDraw(commandBuffer, numVertices, instanceCount, 0, 0);
}

#if 0
void afSetTextureName(AFTexRef tex, const char* name)
{
	static PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT_ = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetDeviceProcAddr(deviceMan.GetDevice(), "vkDebugMarkerSetObjectNameEXT"));
	if (!vkDebugMarkerSetObjectNameEXT_)
	{
		return;
	}
	VkDebugMarkerObjectNameInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT, nullptr, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, (uint64_t)tex->image, name };
	vkDebugMarkerSetObjectNameEXT_(deviceMan.GetDevice(), &info);
}
#else
void afSetTextureName(AFTexRef tex, const char* name)
{
	static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT_ = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetDeviceProcAddr(deviceMan.GetDevice(), "vkSetDebugUtilsObjectNameEXT"));
	if (!vkSetDebugUtilsObjectNameEXT_)
	{
		return;
	}
	VkDebugUtilsObjectNameInfoEXT info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, nullptr, VK_OBJECT_TYPE_IMAGE, (uint64_t)tex->image, name };
	afHandleVKError(vkSetDebugUtilsObjectNameEXT_(deviceMan.GetDevice(), &info));
}
#endif

static VkRenderPass CreateRenderPassForPresent(VkFormat colorBufferFormat)
{
	const VkAttachmentReference colorAttachmentReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	const VkSubpassDescription subpassDescriptions[] = { { 0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorAttachmentReference } };
	const VkAttachmentDescription attachments[] = { { 0, colorBufferFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR } };
	const VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, arrayparam(attachments), arrayparam(subpassDescriptions) };
	VkRenderPass renderPass = 0;
	afHandleVKError(vkCreateRenderPass(deviceMan.GetDevice(), &renderPassInfo, nullptr, &renderPass));
	return renderPass;
}

static VkRenderPass CreateRenderPass(VkFormat colorBufferFormat, VkFormat depthStencilFormat)
{
	const VkAttachmentReference colorAttachmentReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	const VkAttachmentReference depthStencilAttachmentReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	const VkSubpassDescription subpassDescriptions[] = { { 0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &colorAttachmentReference, nullptr, depthStencilFormat == AFF_INVALID ? nullptr : &depthStencilAttachmentReference } };
	const VkAttachmentDescription attachments[2] = { { 0, colorBufferFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }, { 0, depthStencilFormat, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL } };
	const VkRenderPassCreateInfo renderPassInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, nullptr, 0, depthStencilFormat == AFF_INVALID ? 1u : 2u, attachments, arrayparam(subpassDescriptions) };
	VkRenderPass renderPass = 0;
	afHandleVKError(vkCreateRenderPass(deviceMan.GetDevice(), &renderPassInfo, nullptr, &renderPass));
	return renderPass;
}

static uint32_t GetVkFormatSize(VkFormat format)
{
	switch (format)
	{
	case AFF_R8G8B8A8_UNORM: return 4;
	case AFF_R32G32B32_FLOAT: return 12;
	case AFF_R32G32_FLOAT: return 8;
	case AFF_R8G8B8A8_UINT: return 4;
	case AFF_R32_UINT: return 4;
	}
	assert(0);
	return 0;
}

static VkPrimitiveTopology RenderFlagsToPrimitiveTopology(uint32_t flags)
{
	if (flags & AFRS_PRIMITIVE_TRIANGLELIST)
	{
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
	else if (flags & AFRS_PRIMITIVE_LINELIST)
	{
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	}
	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
}

static VkRenderPass VkFormatToRenderPassForOffScreenRenderTarget(VkFormat renderTargetFormat, VkFormat depthStencilFormat)
{
	if (renderTargetFormat == VK_FORMAT_R8G8B8A8_UNORM && depthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		return deviceMan.offscreenR8G8B8A8D32S8RenderPass;
	}
	if (renderTargetFormat == VK_FORMAT_R16G16B16A16_SFLOAT && depthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		return deviceMan.offscreenR16G16B16A16D32S8RenderPass;
	}
	if (renderTargetFormat == VK_FORMAT_R8G8B8A8_UNORM && depthStencilFormat == VK_FORMAT_UNDEFINED)
	{
		return deviceMan.offscreenR8G8B8A8RenderPass;
	}
	if (renderTargetFormat == VK_FORMAT_R16G16B16A16_SFLOAT && depthStencilFormat == VK_FORMAT_UNDEFINED)
	{
		return deviceMan.offscreenR16G16B16A16RenderPass;
	}
	assert(0);
	return 0;
}

static VkRenderPass RenderFlagsToRenderPass(uint32_t flags)
{
	if (afIsRenderFlagsRenderToSwapchainSurface(flags))
	{
		return deviceMan.primaryRenderPass;
	}
	return VkFormatToRenderPassForOffScreenRenderTarget(afRenderFlagsToRTFormat(flags), afRenderFlagsToDSFormat(flags));
}

static bool IsPresentModeSupported(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentMode)
{
	uint32_t numPresentModes = 0;
	VkPresentModeKHR presentModes[VK_PRESENT_MODE_RANGE_SIZE_KHR] = {};
	afHandleVKError(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &numPresentModes, nullptr));
	assert(numPresentModes <= _countof(presentModes));
	afHandleVKError(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &numPresentModes, presentModes));
	auto it = std::find(presentModes, presentModes + numPresentModes, presentMode);
	assert(it != presentModes + numPresentModes);
	return it != presentModes + numPresentModes;
}

static VkPipeline afCreatePipeline(VkDevice device, VkPipelineCache pipelineCache, const char* name, VkPipelineLayout pipelineLayout, uint32_t numAttributes, const VkVertexInputAttributeDescription attributes[], uint32_t flags)
{
	char path[MAX_PATH];
	sprintf_s(path, sizeof(path), "spv/%s.vert.spv", name);
	VkShaderModule vertexShader = CreateShaderModule(device, path);
	sprintf_s(path, sizeof(path), "spv/%s.frag.spv", name);
	VkShaderModule fragmentShader = CreateShaderModule(device, path);
	VkVertexInputBindingDescription binding = {};
	std::for_each(attributes, attributes + numAttributes, [&](const VkVertexInputAttributeDescription& attr) { binding.stride += GetVkFormatSize(attr.format); assert(attr.binding == 0); });
	const VkPipelineShaderStageCreateInfo shaderStageCreationInfos[] = { { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vertexShader, "main" },{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader, "main" } };
	const VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, !!numAttributes, &binding, numAttributes, attributes };
	const VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, RenderFlagsToPrimitiveTopology(flags) };
	const VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, 1, nullptr, 1, nullptr };
	const VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_FALSE, (flags & AFRS_WIREFRAME) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0, 0, 0, 1.0f };
	const VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0, VK_SAMPLE_COUNT_1_BIT };
	const VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, nullptr, 0, !!(flags & (AFRS_DEPTH_CLOSEREQUAL_READONLY | AFRS_DEPTH_ENABLE)), !!(flags & AFRS_DEPTH_ENABLE), (flags & AFRS_DEPTH_CLOSEREQUAL_READONLY) ? VK_COMPARE_OP_LESS_OR_EQUAL : VK_COMPARE_OP_LESS };
	const VkPipelineColorBlendAttachmentState colorBlendAttachmentStateNone = { VK_FALSE, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, 0xf };
	const VkPipelineColorBlendAttachmentState colorBlendAttachmentStateAlphaBlend = { VK_TRUE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ZERO, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, 0xf };
	const VkPipelineColorBlendStateCreateInfo colorBlendState = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_LOGIC_OP_CLEAR, 1, (flags & AFRS_ALPHA_BLEND) ? &colorBlendAttachmentStateAlphaBlend : &colorBlendAttachmentStateNone };
	const VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	const VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, arrayparam(dynamicStates) };
	const VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfos[] = { { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0, arrayparam(shaderStageCreationInfos), &pipelineVertexInputStateCreateInfo, &pipelineInputAssemblyStateCreateInfo, nullptr, &viewportStateCreateInfo, &rasterizationStateCreateInfo, &multisampleStateCreateInfo, &depthStencilStateCreateInfo, &colorBlendState, &pipelineDynamicStateCreateInfo, pipelineLayout, RenderFlagsToRenderPass(flags) } };
	VkPipeline pipeline = 0;
	afHandleVKError(vkCreateGraphicsPipelines(device, pipelineCache, arrayparam(graphicsPipelineCreateInfos), nullptr, &pipeline));
	afSafeDeleteVk(vkDestroyShaderModule, device, vertexShader);
	afSafeDeleteVk(vkDestroyShaderModule, device, fragmentShader);
	return pipeline;
}

void DeviceManVK::Create(HWND hWnd)
{
	const char* instanceExtensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#ifndef NDEBUG
//		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,	// old
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
	};
	const char* instanceLayers[] =
	{
		"VK_LAYER_LUNARG_standard_validation",
	};
	const VkInstanceCreateInfo instInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, nullptr,
#ifndef NDEBUG
		1,
#else
		0,
#endif
		instanceLayers, arrayparam(instanceExtensions) };
	afHandleVKError(vkCreateInstance(&instInfo, nullptr, &inst));

	VkPhysicalDevice devices[16] = {};
	uint32_t numDevices = _countof(devices);
	afHandleVKError(vkEnumeratePhysicalDevices(inst, &numDevices, devices));
	physicalDevice = devices[0];
	afVerify(physicalDevice);

	// query properties
	uint32_t numQueueFamilyProperties = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilyProperties, nullptr);
	VkQueueFamilyProperties queueFamilyProperties[3];
	assert(numQueueFamilyProperties <= _countof(queueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilyProperties, queueFamilyProperties);
	assert(queueFamilyProperties[0].queueFlags & VK_QUEUE_GRAPHICS_BIT);
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

	VkExtensionProperties deviceExtensions[128] = {};
	uint32_t deviceExtensionCount = _countof(deviceExtensions);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, deviceExtensions);

	float priorities[] = { 0 };
	const VkDeviceQueueCreateInfo devQueueInfos[] = { { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, priorities } };
	const char* enabledDeviceExtensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	//	VK_EXT_DEBUG_MARKER_EXTENSION_NAME	// old
	};
	VkPhysicalDeviceFeatures features = {};
	features.fillModeNonSolid = VK_TRUE;
	const VkDeviceCreateInfo devInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, nullptr, 0, 1, devQueueInfos, 0, nullptr, arrayparam(enabledDeviceExtensions), &features };
	afHandleVKError(vkCreateDevice(physicalDevice, &devInfo, nullptr, &device));

	// preallocated resources and descriptors
	uboAllocator.Create(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 0x20000);
	vertexBufferAllocator.Create(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0x50000);
	const VkDescriptorPoolSize descriptorPoolSizes[2] = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, descriptorPoolSize },{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, descriptorPoolSize } };
	const VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, descriptorPoolSize, arrayparam(descriptorPoolSizes) };
	afHandleVKError(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));
	const VkDescriptorSetLayoutBinding textureDescriptorSetLayoutBindings[] = { { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT } };
	const VkDescriptorSetLayoutCreateInfo textureDescriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, arrayparam(textureDescriptorSetLayoutBindings) };
	afHandleVKError(vkCreateDescriptorSetLayout(device, &textureDescriptorSetLayoutCreateInfo, nullptr, &commonTextureDescriptorSetLayout));
	const VkDescriptorSetLayoutBinding uboDescriptorSetLayoutBindings[] = { { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT } };
	const VkDescriptorSetLayoutCreateInfo uboDescriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, arrayparam(uboDescriptorSetLayoutBindings) };
	afHandleVKError(vkCreateDescriptorSetLayout(device, &uboDescriptorSetLayoutCreateInfo, nullptr, &commonUboDescriptorSetLayout));
	const VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, descriptorPool, 1, &commonUboDescriptorSetLayout };
	afHandleVKError(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &commonUboDescriptorSet));
	const VkDescriptorBufferInfo descriptorBufferInfo = { uboAllocator.bufferContext.buffer, 0, VK_WHOLE_SIZE };
	const VkWriteDescriptorSet writeDescriptorSets[] = { { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, commonUboDescriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, nullptr, &descriptorBufferInfo } };
	vkUpdateDescriptorSets(device, arrayparam(writeDescriptorSets), 0, nullptr);
	const VkSamplerCreateInfo samplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, nullptr, 0, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, 0, VK_FALSE, 1.0f };
	vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler);
	const VkPipelineCacheCreateInfo pipelineCacheCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
	afHandleVKError(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache));

	// command buffer
	const VkCommandPoolCreateInfo commandPoolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	afHandleVKError(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));
	const VkCommandBufferAllocateInfo commandBufferAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1 };
	afHandleVKError(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));
	const VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	afHandleVKError(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore));
	const VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	afHandleVKError(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));

	// Windows
	const VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, GetModuleHandle(nullptr), hWnd };
	afHandleVKError(vkCreateWin32SurfaceKHR(inst, &surfaceInfo, nullptr, &surface));
	uint32_t numSurfaceFormats = 0;
	VkSurfaceFormatKHR surfaceFormats[32] = {};
	afHandleVKError(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numSurfaceFormats, nullptr));
	assert(numSurfaceFormats <= _countof(surfaceFormats));
	afHandleVKError(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &numSurfaceFormats, surfaceFormats));
	VkPresentModeKHR presentMode = AF_WAIT_VBLANK ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
	IsPresentModeSupported(physicalDevice, surface, presentMode);
	VkSurfaceCapabilitiesKHR surfaceCaps = {};
	afHandleVKError(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));
	VkBool32 physicalDeviceSurfaceSupport = VK_FALSE;
	afHandleVKError(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, surface, &physicalDeviceSurfaceSupport));
	assert(physicalDeviceSurfaceSupport);
	GetClientRect(hWnd, &rc);
	const VkSwapchainCreateInfoKHR swapchainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr, 0, surface, surfaceCaps.minImageCount, surfaceFormats[0].format, surfaceFormats[0].colorSpace,{ uint32_t(rc.right), uint32_t(rc.bottom) }, 1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, presentMode, VK_TRUE };
	afHandleVKError(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain));
	afHandleVKError(vkGetSwapchainImagesKHR(device, swapchain, &swapChainCount, nullptr));
	assert(swapChainCount <= _countof(swapChainImages));
	assert(swapChainCount <= _countof(imageViews));
	assert(swapChainCount <= _countof(framebuffers));
	afHandleVKError(vkGetSwapchainImagesKHR(device, swapchain, &swapChainCount, swapChainImages));

	// render pass
	primaryRenderPass = CreateRenderPassForPresent(swapchainInfo.imageFormat);
	offscreenR8G8B8A8D32S8RenderPass = CreateRenderPass(AFF_R8G8B8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT);
	offscreenR16G16B16A16D32S8RenderPass = CreateRenderPass(AFF_R16G16B16A16_FLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT);
	offscreenR8G8B8A8RenderPass = CreateRenderPass(AFF_R8G8B8A8_UNORM, VK_FORMAT_UNDEFINED);
	offscreenR16G16B16A16RenderPass = CreateRenderPass(AFF_R16G16B16A16_FLOAT, VK_FORMAT_UNDEFINED);
	for (int i = 0; i < (int)swapChainCount; i++)
	{
		const VkImageViewCreateInfo imageViewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, swapChainImages[i], VK_IMAGE_VIEW_TYPE_2D, surfaceFormats[0].format, colorComponentMapping,{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
		afHandleVKError(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]));
		const VkImageView frameBufferAttachmentImageView[] = { imageViews[i] };
		const VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, primaryRenderPass, arrayparam(frameBufferAttachmentImageView), (uint32_t)rc.right, (uint32_t)rc.bottom, 1 };
		afHandleVKError(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]));
	}

	afHandleVKError(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &frameIndex));
}

void DeviceManVK::BeginRenderToSwapChain()
{
	BeginRenderPass(primaryRenderPass, framebuffers[frameIndex], { (int)rc.right, (int)rc.bottom }, false);
}

void DeviceManVK::EndRenderPass()
{
	vkCmdEndRenderPass(commandBuffer);
}

void DeviceManVK::BeginRenderPass(VkRenderPass nextRenderPass, VkFramebuffer nextFramebuffer, IVec2 size, bool needDepth)
{
	const VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, nullptr, nextRenderPass, nextFramebuffer,{ {},{ (uint32_t)size.x, (uint32_t)size.y } }, needDepth ? 2u : 1u, clearValues };
	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	VkViewport v = { 0, 0, (float)size.x, (float)size.y, 0, 1 };
	VkRect2D s = { 0, 0, (uint32_t)size.x, (uint32_t)size.y };
	vkCmdSetViewport(commandBuffer, 0, 1, &v);
	vkCmdSetScissor(commandBuffer, 0, 1, &s);
}

void DeviceManVK::Present()
{
	afHandleVKError(vkEndCommandBuffer(commandBuffer));

	VkQueue queue = 0;
	vkGetDeviceQueue(device, 0, 0, &queue);
	assert(queue);

	const VkSubmitInfo submitInfos[] = { { VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &commandBuffer } };
	afHandleVKError(vkQueueSubmit(queue, arrayparam(submitInfos), 0));

	afHandleVKError(vkQueueWaitIdle(queue));
	uboAllocator.ResetAllocation();
	vertexBufferAllocator.ResetAllocation();

	const VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr, 1, &semaphore, 1, &swapchain, &frameIndex };
	afHandleVKError(vkQueuePresentKHR(queue, &presentInfo));

	afHandleVKError(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &frameIndex));

	const VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	afHandleVKError(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
}

void DeviceManVK::Destroy()
{
	afSafeDeleteVk(vkDestroySampler, device, sampler);
	if (commonUboDescriptorSet)
	{
		afHandleVKError(vkFreeDescriptorSets(device, descriptorPool, 1, &commonUboDescriptorSet));
		commonUboDescriptorSet = 0;
	}
	afSafeDeleteVk(vkDestroyDescriptorSetLayout, device, commonUboDescriptorSetLayout);
	afSafeDeleteVk(vkDestroyDescriptorSetLayout, device, commonTextureDescriptorSetLayout);
	uboAllocator.Destroy();
	vertexBufferAllocator.Destroy();
	afSafeDeleteVk(vkDestroyDescriptorPool, device, descriptorPool);
	afSafeDeleteVk(vkDestroyPipelineCache, device, pipelineCache);
	if (commandBuffer)
	{
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
		commandBuffer = 0;
	}
	afSafeDeleteVk(vkDestroySemaphore, device, semaphore);
	for (auto& it : imageViews)
	{
		afSafeDeleteVk(vkDestroyImageView, device, it);
	}
	afSafeDeleteVk(vkDestroyCommandPool, device, commandPool);
	std::for_each(framebuffers, framebuffers + _countof(framebuffers), [&](VkFramebuffer& framebuffer) { afSafeDeleteVk(vkDestroyFramebuffer, device, framebuffer);	});
	afSafeDeleteVk(vkDestroyRenderPass, device, primaryRenderPass);
	afSafeDeleteVk(vkDestroyRenderPass, device, offscreenR8G8B8A8D32S8RenderPass);
	afSafeDeleteVk(vkDestroyRenderPass, device, offscreenR16G16B16A16D32S8RenderPass);
	afSafeDeleteVk(vkDestroyRenderPass, device, offscreenR8G8B8A8RenderPass);
	afSafeDeleteVk(vkDestroyRenderPass, device, offscreenR16G16B16A16RenderPass);
	afSafeDeleteVk(vkDestroySwapchainKHR, device, swapchain);
	if (surface)
	{
		vkDestroySurfaceKHR(inst, surface, nullptr);
		surface = 0;
	}
	if (device)
	{
		vkDestroyDevice(device, nullptr);
		device = nullptr;
	}
	if (inst)
	{
		vkDestroyInstance(inst, nullptr);
		inst = nullptr;
	}
}

void DeviceManVK::Flush()
{
	if (!commandBuffer)
	{
		return;
	}

	afHandleVKError(vkEndCommandBuffer(commandBuffer));

	VkQueue queue = 0;
	vkGetDeviceQueue(device, 0, 0, &queue);
	assert(queue);

	const VkSubmitInfo submitInfos[] = { { VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &commandBuffer } };
	afHandleVKError(vkQueueSubmit(queue, arrayparam(submitInfos), 0));

	afHandleVKError(vkQueueWaitIdle(queue));
	uboAllocator.ResetAllocation();
	vertexBufferAllocator.ResetAllocation();

	const VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	afHandleVKError(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
}

void AFBufferStackAllocator::Create(VkBufferUsageFlags usage, int size)
{
	bufferContext = afCreateBufferInternal(size, nullptr, usage);
	ResetAllocation();
}

uint32_t AFBufferStackAllocator::Allocate(int size, const void* data)
{
	const VkDeviceSize& align = bufferContext.memoryRequirement.alignment;
	const uint32_t offset = (uint32_t)allocatedSize;
	const VkDeviceSize sizeAligned = ((VkDeviceSize)size + align - 1) / align * align;
	const VkDeviceSize afterAllocatedSize = allocatedSize + sizeAligned;
	assert(afterAllocatedSize <= bufferContext.size);
	allocatedSize = afterAllocatedSize;
	memcpy((uint8_t*)bufferContext.mappedMemory + offset, data, size);
	return offset;
}

void AFBufferStackAllocator::ResetAllocation()
{
	allocatedSize = 0;
}

void AFBufferStackAllocator::Destroy()
{
	afSafeDeleteBuffer(bufferContext);
}

void AFRenderStates::Create(const char* shaderName, int numInputElements, const InputElement* inputElements, uint32_t flags, int numSamplers, const SamplerType samplerTypes[])
{
	(void)numSamplers;
	(void)samplerTypes;
	VkDevice device = deviceMan.GetDevice();

	// FIXME: hard corded pipeline layout
	std::map<std::string, std::string> shaderToLayout =
	{
		{ "solid", "U" },
		{ "sky_photosphere", "TU" },
		{ "sky_cubemap", "TU" },
		{ "projection_equirectangular_to_stereographic", "TU" },
		{ "sprite", "TU" },
		{ "vivid", "TU" },
		{ "letterbox", "TU" },
		{ "font", "T" },
		{ "skin_instanced", "UUUT" },
		{ "water_es2", "TTTTTTU" },
		{ "glow_extraction", "T" },
		{ "glow_copy", "T" },
		{ "glow_lastpass", "TTTTTTT" },
		{ "water_es3_heightmap", "TU" },
		{ "water_es3_normal", "UT" },
		{ "water_es3_lastpass", "TTTTTTTU" },
	};
	const char* layout = shaderToLayout[shaderName].c_str();

	std::vector<VkDescriptorSetLayout> descriptorLayouts;
	for (size_t i = 0; i < strlen(layout); i++)
	{
		switch (layout[i])
		{
		case 'U':
			descriptorLayouts.push_back(deviceMan.commonUboDescriptorSetLayout);
			break;
		case 'T':
			descriptorLayouts.push_back(deviceMan.commonTextureDescriptorSetLayout);
			break;
		}
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, (uint32_t)descriptorLayouts.size(), descriptorLayouts.data() };
	afHandleVKError(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
	pipeline = afCreatePipeline(device, deviceMan.GetPipelineCache(), shaderName, pipelineLayout, numInputElements, inputElements, flags);
}

void AFRenderStates::Apply()
{
	VkCommandBuffer commandBuffer = deviceMan.commandBuffer;
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void AFRenderStates::Destroy()
{
	VkDevice device = deviceMan.GetDevice();
	afSafeDeleteVk(vkDestroyPipelineLayout, device, pipelineLayout);
	afSafeDeleteVk(vkDestroyPipeline, device, pipeline);
}

void AFRenderTarget::Init(IVec2 size, AFFormat colorFormat, AFFormat depthStencilFormat)
{
	VkDevice device = deviceMan.GetDevice();
	renderTarget = afCreateDynamicTexture(colorFormat, size, AFTF_RTV | AFTF_SRV);
	switch (depthStencilFormat)
	{
	case AFF_D24_UNORM_S8_UINT:
	case AFF_D32_FLOAT_S8_UINT:
	{
		depthStencil = afCreateDynamicTexture(depthStencilFormat, size, AFTF_DSV);
		const VkImageView frameBufferAttachmentImageView[] = { renderTarget->view, depthStencil->view };
		const VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, VkFormatToRenderPassForOffScreenRenderTarget(colorFormat, depthStencilFormat), arrayparam(frameBufferAttachmentImageView), (uint32_t)size.x, (uint32_t)size.y, 1 };
		afHandleVKError(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer));
		break;
	}
	case AFF_INVALID:
	{
		const VkImageView frameBufferAttachmentImageView1[] = { renderTarget->view };
		const VkFramebufferCreateInfo framebufferInfo1 = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, nullptr, 0, VkFormatToRenderPassForOffScreenRenderTarget(colorFormat, VK_FORMAT_UNDEFINED), arrayparam(frameBufferAttachmentImageView1), (uint32_t)size.x, (uint32_t)size.y, 1 };
		afHandleVKError(vkCreateFramebuffer(device, &framebufferInfo1, nullptr, &framebuffer));
		break;
	}
	default:
		assert(0);
	}
}

void AFRenderTarget::Destroy()
{
	VkDevice device = deviceMan.GetDevice();
	afSafeDeleteVk(vkDestroyFramebuffer, device, framebuffer);
	afSafeDeleteTexture(renderTarget);
	afSafeDeleteTexture(depthStencil);
}

void AFRenderTarget::BeginRenderToThis()
{
	AFFormat dsFormat = depthStencil ? depthStencil->format : AFF_INVALID;
	deviceMan.BeginRenderPass(VkFormatToRenderPassForOffScreenRenderTarget(renderTarget->format, dsFormat), framebuffer, renderTarget->texDesc.size, dsFormat != AFF_INVALID);
}

void AFRenderTarget::EndRenderToThis()
{
	deviceMan.EndRenderPass();
}

AFTexRef AFRenderTarget::GetTexture()
{
	return renderTarget;
}

#endif
