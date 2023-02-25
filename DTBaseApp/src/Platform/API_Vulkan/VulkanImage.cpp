#include "VulkanImage.h"
#include "VulkanContext.h"

namespace DT
{
	VulkanImage::VulkanImage(const ImageSpecification& specification)
	{
	}

	VulkanImage::~VulkanImage()
	{
		Destroy();
	}

	void VulkanImage::Invalidate()
	{
		Destroy();

		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType				  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext				  = nullptr;
		imageCreateInfo.flags				  = 0u;
		imageCreateInfo.imageType			  = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format				  = VK_FORMAT_R8G8B8A8_UNORM;
		imageCreateInfo.extent.width		  = m_Specification.Width;
		imageCreateInfo.extent.height		  = m_Specification.Height;
		imageCreateInfo.extent.depth		  = 1u;
		imageCreateInfo.mipLevels			  = 1u;
		imageCreateInfo.arrayLayers			  = 1u;
		imageCreateInfo.samples				  = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling				  = VK_IMAGE_TILING_LINEAR;
		imageCreateInfo.usage				  = VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode			  = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0u;
		imageCreateInfo.pQueueFamilyIndices	  = nullptr;
		imageCreateInfo.initialLayout		  = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		VK_CALL(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &m_Image, &m_ImageAllocation, nullptr));
	}

	void VulkanImage::Destroy()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyImage(allocator, m_Image, m_ImageAllocation);
		m_Image = VK_NULL_HANDLE;
		m_ImageAllocation = VK_NULL_HANDLE;
	}


	VulkanDynamicImage::VulkanDynamicImage(const ImageSpecification& specification)
		: m_Specification(specification)
	{
		Invalidate();
	}

	VulkanDynamicImage::~VulkanDynamicImage()
	{
		Destroy();
	}

	void VulkanDynamicImage::Invalidate()
	{
		Destroy();

		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType				  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext				  = nullptr;
		imageCreateInfo.flags				  = 0u;
		imageCreateInfo.imageType			  = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format				  = VK_FORMAT_R8G8B8A8_UNORM;
		imageCreateInfo.extent.width		  = m_Specification.Width;
		imageCreateInfo.extent.height		  = m_Specification.Height;
		imageCreateInfo.extent.depth		  = 1u;
		imageCreateInfo.mipLevels			  = 1u;
		imageCreateInfo.arrayLayers			  = 1u;
		imageCreateInfo.samples				  = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling				  = VK_IMAGE_TILING_LINEAR;
		imageCreateInfo.usage				  = VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode			  = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0u;
		imageCreateInfo.pQueueFamilyIndices	  = nullptr;
		imageCreateInfo.initialLayout		  = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		VK_CALL(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &m_Image, &m_Allocation, &m_AllocationInfo));

		VkImageSubresource imageSubresource{};
		imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresource.mipLevel   = 0u;
		imageSubresource.arrayLayer = 0u;
		vkGetImageSubresourceLayout(device, m_Image, &imageSubresource, &m_SubresourceLayout);
	}

	void VulkanDynamicImage::Destroy()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		vmaDestroyImage(allocator, m_Image, m_Allocation);
		m_Image = VK_NULL_HANDLE;
		m_Allocation = VK_NULL_HANDLE;
	}
}