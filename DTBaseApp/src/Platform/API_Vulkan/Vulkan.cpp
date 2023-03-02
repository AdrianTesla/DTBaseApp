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
		}
		ASSERT(false);
		return VK_FORMAT_UNDEFINED;
	}

	VkImageUsageFlagBits ToVulkanImageUsage(ImageUsage::Usage usage)
	{
		switch (usage)
		{
			case ImageUsage::TransferSrc:		  return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			case ImageUsage::TransferDst:		  return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			case ImageUsage::Texture:			  return VK_IMAGE_USAGE_SAMPLED_BIT;
			case ImageUsage::Storage:			  return VK_IMAGE_USAGE_STORAGE_BIT;
			case ImageUsage::ColorAttachment:	  return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			case ImageUsage::DepthAttachment:	  return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			case ImageUsage::TransientAttachment: return VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
			case ImageUsage::InputAttachment:	  return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		}
		ASSERT(false);
		return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
	}

	VkImageUsageFlags ToVulkanImageUsageFlags(ImageUsageFlags usageFlags)
	{
		VkImageUsageFlags usage = 0u;
		for (uint8 i = 0u; i < 32u; i++) {
			ImageUsage::Usage currentFlag = ImageUsage::Usage(1 << i);
			if (currentFlag & usageFlags) {
				usage |= ToVulkanImageUsage(currentFlag);
			}
		}
		return usage;
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
			case PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
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

	void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_NONE;
		VkAccessFlags srcAccess = VK_ACCESS_NONE;

		VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_NONE;
		VkAccessFlags dstAccess = VK_ACCESS_NONE;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		{
			srcAccess = 0u;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			srcAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}

		if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			dstAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			dstAccess = VK_ACCESS_SHADER_READ_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext               = nullptr;
		imageMemoryBarrier.srcAccessMask       = srcAccess;
		imageMemoryBarrier.dstAccessMask       = dstAccess;
		imageMemoryBarrier.oldLayout           = oldLayout;
		imageMemoryBarrier.newLayout           = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image               = image;
		imageMemoryBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		imageMemoryBarrier.subresourceRange.baseMipLevel   = 0u;
		imageMemoryBarrier.subresourceRange.levelCount     = 1u;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
		imageMemoryBarrier.subresourceRange.layerCount     = 1u;

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage, 0u,
			0u, nullptr,            // memory barrier
			0u, nullptr,            // buffer memory barrier
			1u, &imageMemoryBarrier // image memory barrier
		);
	}
}