#include "Vulkan.h"
#include "VulkanContext.h"
#include "Platform/PlatformUtils.h"

namespace DT::Convert
{
	VkFormat ToVulkanFormat(ImageFormat format)
	{
		switch (format)
		{
			case ImageFormat::RGBA8:   return VK_FORMAT_R8G8B8A8_UNORM;
			case ImageFormat::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case ImageFormat::D32:     return VK_FORMAT_D32_SFLOAT;
		}
		ASSERT(false);
		return VK_FORMAT_UNDEFINED;
	}

	VkPolygonMode ToVulkanPolygonMode(PolygonMode polygonMode)
	{
		switch (polygonMode)
		{
			case PolygonMode::Fill:      return VK_POLYGON_MODE_FILL;
			case PolygonMode::Wireframe: return VK_POLYGON_MODE_LINE;
			case PolygonMode::Point:     return VK_POLYGON_MODE_POINT;
		}
		ASSERT(false);
		return VK_POLYGON_MODE_MAX_ENUM;
	}

	VkPrimitiveTopology ToVulkanPrimitiveTopology(PrimitiveTopology topology)
	{
		switch (topology)
		{
			case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PrimitiveTopology::LineList:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PrimitiveTopology::PointList:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		}
		ASSERT(false);
		return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}

	VkCullModeFlags ToVulkanCullMode(FaceCulling culling)
	{
		switch (culling)
		{
			case FaceCulling::Back:  return VK_CULL_MODE_BACK_BIT;
			case FaceCulling::Front: return VK_CULL_MODE_FRONT_BIT;
			case FaceCulling::None:  return VK_CULL_MODE_NONE;
		}
		ASSERT(false);
		return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
	}
}

namespace DT::Vulkan
{
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateInfo* pAllocationCreateInfo, VulkanBuffer* pBuffer)
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext                 = nullptr;
		bufferCreateInfo.flags                 = 0u;
		bufferCreateInfo.size                  = size;
		bufferCreateInfo.usage                 = usage;
		bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.queueFamilyIndexCount = 0u;
		bufferCreateInfo.pQueueFamilyIndices   = nullptr;
		VK_CALL(vmaCreateBuffer(allocator, &bufferCreateInfo, pAllocationCreateInfo, &pBuffer->Buffer, &pBuffer->Allocation, &pBuffer->AllocationInfo));
	}

	void DestroyBuffer(VulkanBuffer& buffer)
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyBuffer(allocator, buffer.Buffer, buffer.Allocation);
		buffer.Buffer = VK_NULL_HANDLE;
		buffer.Allocation = VK_NULL_HANDLE;
	}

	void CreateBufferStaging(const void* data, uint64 size, VkBufferUsageFlags usage, VulkanBuffer* pBuffer)
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		// create staging buffer
		VulkanBuffer stagingBuffer{};
		VmaAllocationCreateInfo stagingAllocationCreateInfo{};
		stagingAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		stagingAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingAllocationCreateInfo, &stagingBuffer);
		
		// upload data to it
		ASSERT(stagingBuffer.AllocationInfo.pMappedData != nullptr);
		memcpy(stagingBuffer.AllocationInfo.pMappedData, data, size);

		// create device local buffer
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		CreateBuffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, &allocationCreateInfo, pBuffer);
		
		// copy the staging buffer to the device local buffer
		VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();
		VkCommandBuffer commandBuffer = vulkanDevice.BeginCommandBuffer(QueueType::Transfer);
		vkCmd::CopyBuffer(commandBuffer, stagingBuffer.Buffer, pBuffer->Buffer, size);
		vulkanDevice.EndCommandBuffer();

		// destroy staging buffer and free staging allocation
		vmaDestroyBuffer(allocator, stagingBuffer.Buffer, stagingBuffer.Allocation);
	}

	bool HasDepthComponent(VkFormat format)
	{
		switch (format)
		{
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
			case VK_FORMAT_D32_SFLOAT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D16_UNORM_S8_UINT:
			case VK_FORMAT_D16_UNORM:
				return true;
		}
		return false;
	}
	
	bool HasStencilComponent(VkFormat format)
	{
		switch (format)
		{
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
			case VK_FORMAT_D24_UNORM_S8_UINT:
			case VK_FORMAT_D16_UNORM_S8_UINT:
				return true;
		}
		return false;
	}

	bool HasColorComponent(VkFormat format)
	{
		return !HasDepthComponent(format) && !HasStencilComponent(format);
	}
	
	MemoryDependency CalculateMemoryDependency(VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		MemoryDependency dependency{};

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		{
			dependency.SrcAccess = 0u;
			dependency.SrcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			dependency.SrcAccess = VK_ACCESS_TRANSFER_READ_BIT;
			dependency.SrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			dependency.SrcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
			dependency.SrcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else 
		{ 
			ASSERT(false); 
		}

		if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			dependency.DstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
			dependency.DstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			dependency.DstAccess = VK_ACCESS_TRANSFER_READ_BIT;
			dependency.DstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			dependency.DstAccess = VK_ACCESS_SHADER_READ_BIT;
			dependency.DstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			dependency.DstAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependency.DstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			dependency.DstAccess = 0u;
			dependency.DstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		else
		{
			ASSERT(false);
		}

		return dependency;
	}

	VkImageAspectFlags GetImageAspectFlags(VkFormat format)
	{
		VkImageAspectFlags imageAspect = 0u;

		if (Vulkan::HasDepthComponent(format))
			imageAspect |= VK_IMAGE_ASPECT_DEPTH_BIT;

		if (Vulkan::HasStencilComponent(format))
			imageAspect |= VK_IMAGE_ASPECT_STENCIL_BIT;

		if (Vulkan::HasColorComponent(format))
			imageAspect |= VK_IMAGE_ASPECT_COLOR_BIT;

		return imageAspect;
	}
}

namespace DT::vkCmd
{
	void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkBufferCopy bufferCopyRegion{};
		bufferCopyRegion.srcOffset = 0u;
		bufferCopyRegion.dstOffset = 0u;
		bufferCopyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1u, &bufferCopyRegion);
	}

	void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, uint32 width, uint32 height)
	{
		VkBufferImageCopy copyRegion{};
		copyRegion.bufferOffset                    = 0u;
		copyRegion.bufferRowLength                 = 0u;
		copyRegion.bufferImageHeight               = 0u;
		copyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel       = 0u;
		copyRegion.imageSubresource.baseArrayLayer = 0u;
		copyRegion.imageSubresource.layerCount     = 1u;
		copyRegion.imageOffset                     = { 0u,0u,0u };
		copyRegion.imageExtent.width               = width;
		copyRegion.imageExtent.height              = height;
		copyRegion.imageExtent.depth               = 1u;
		vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &copyRegion);
	}

	//static void TransitionImageLayout(
	//		VkCommandBuffer commandBuffer,
	//		VkImage image,
	//		VkImageLayout oldImageLayout,
	//		VkImageLayout newImageLayout,
	//		VkImageSubresourceRange subresourceRange,
	//		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	//		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
	//{
	//	VkAccessFlags srcAccess = 0u;
	//	VkAccessFlags dstAccess = 0u;

	//	switch (oldImageLayout)
	//	{
	//		case VK_IMAGE_LAYOUT_UNDEFINED:                        srcAccess = 0u;                                           break;
	//		case VK_IMAGE_LAYOUT_PREINITIALIZED:                   srcAccess = VK_ACCESS_HOST_WRITE_BIT;                     break;
	//		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:         srcAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;         break;
	//		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: srcAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; break;
	//		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:             srcAccess = VK_ACCESS_TRANSFER_READ_BIT;                  break;
	//		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:             srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;                 break;
	//		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:         srcAccess = VK_ACCESS_SHADER_READ_BIT;                    break;
	//	}

	//	switch (newImageLayout)
	//	{
	//		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:             dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;                  break;
	//		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:             dstAccess = VK_ACCESS_TRANSFER_READ_BIT;                   break;
	//		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:         dstAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;          break;
	//		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: dstAccess |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; break;
	//		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:         dstAccess = VK_ACCESS_SHADER_READ_BIT;
	//			if (srcAccess == 0u)
	//				srcAccess = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
	//			break;
	//	}

	//	VkImageMemoryBarrier imageMemoryBarrier{};
	//	imageMemoryBarrier.sType			   = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	//	imageMemoryBarrier.pNext			   = nullptr;
	//	imageMemoryBarrier.srcAccessMask	   = srcAccess;
	//	imageMemoryBarrier.dstAccessMask	   = dstAccess;
	//	imageMemoryBarrier.oldLayout		   = oldImageLayout;
	//	imageMemoryBarrier.newLayout		   = newImageLayout;
	//	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	//	imageMemoryBarrier.image			   = image;
	//	imageMemoryBarrier.subresourceRange	   = subresourceRange;

	//	vkCmdPipelineBarrier(
	//		commandBuffer,
	//		srcStageMask,
	//		dstStageMask,
	//		0u,
	//		0u, nullptr,
	//		0u, nullptr,
	//		1u, &imageMemoryBarrier
	//	);
	//}

	void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& subresource, const MemoryDependency& dependency)
	{
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext               = nullptr;
		imageMemoryBarrier.srcAccessMask       = dependency.SrcAccess;
		imageMemoryBarrier.dstAccessMask       = dependency.DstAccess;
		imageMemoryBarrier.oldLayout           = oldLayout;
		imageMemoryBarrier.newLayout           = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image               = image;
		imageMemoryBarrier.subresourceRange    = subresource;

		vkCmdPipelineBarrier(
			commandBuffer,
			dependency.SrcStage, dependency.DstStage, 0u,
			0u, nullptr,            // memory barrier
			0u, nullptr,            // buffer memory barrier
			1u, &imageMemoryBarrier // image memory barrier
		);
	}

	void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& subresource)
	{
		MemoryDependency dependency = Vulkan::CalculateMemoryDependency(oldLayout, newLayout);
		TransitionImageLayout(commandBuffer, image, oldLayout, newLayout, subresource, dependency);
	}

	void TransitionImageLayoutAllMips(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkFormat format, uint32 mipLevels)
	{
		VkImageSubresourceRange subresource{};
		subresource.aspectMask	   = Vulkan::GetImageAspectFlags(format);
		subresource.baseMipLevel   = 0u;
		subresource.levelCount	   = mipLevels;
		subresource.baseArrayLayer = 0u;
		subresource.layerCount	   = 1u;
		TransitionImageLayout(commandBuffer, image, oldLayout, newLayout, subresource);
	}

	void TransitionImageLayoutSingleMip(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkFormat format, uint32 mip)
	{
		VkImageSubresourceRange subresource{};
		subresource.aspectMask	   = Vulkan::GetImageAspectFlags(format);
		subresource.baseMipLevel   = mip;
		subresource.levelCount	   = 1u;
		subresource.baseArrayLayer = 0u;
		subresource.layerCount	   = 1u;
		TransitionImageLayout(commandBuffer, image, oldLayout, newLayout, subresource);
	}


	void TransitionImageMipLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32 mipLevel)
	{
	}
}