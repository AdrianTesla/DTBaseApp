#pragma once
#include "Vulkan.h"

namespace DT
{
	struct ImageSpecification
	{
		uint32 Width;
		uint32 Height;
	};

	class VulkanImage
	{
	public:
		VulkanImage(const ImageSpecification& specification);
		~VulkanImage();

		void Invalidate();
		void Destroy();
	private:
		ImageSpecification m_Specification;
		VkImage m_Image = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_AllocationInfo;
	};
}