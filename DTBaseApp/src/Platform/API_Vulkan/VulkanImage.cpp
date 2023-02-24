#include "VulkanImage.h"
#include "VulkanContext.h"

namespace DT
{
	VulkanImage::VulkanImage(const ImageSpecification& specification)
		: m_Specification(specification)
	{
		Invalidate();
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

		VmaAllocationCreateInfo allocationCreateInfo{};

		VK_CALL(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &m_Image, &m_Allocation, &m_AllocationInfo));
	}

	void VulkanImage::Destroy()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		vmaDestroyImage(allocator, m_Image, m_Allocation);
		m_Image = VK_NULL_HANDLE;
		m_Allocation = VK_NULL_HANDLE;
	}
}