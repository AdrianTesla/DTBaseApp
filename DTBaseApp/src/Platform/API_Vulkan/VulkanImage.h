#pragma once
#include "Vulkan.h"

namespace DT
{
	struct ImageSpecification
	{
		uint32 Width = 1u;
		uint32 Height = 1u;
	};

	class VulkanImage : public RefCounted
	{
	public:
		VulkanImage(const ImageSpecification& specification);
		~VulkanImage();

		void Invalidate();
		void Destroy();

		VkImage GetVulkanImage() const { return m_Image; }
	private:
		VkImage m_Image = VK_NULL_HANDLE;
		VmaAllocation m_ImageAllocation = VK_NULL_HANDLE;

		ImageSpecification m_Specification;
	};

	class VulkanDynamicImage : public RefCounted
	{
	public:
		VulkanDynamicImage(const ImageSpecification& specification);
		~VulkanDynamicImage();

		void Invalidate();
		void Destroy();

		uint32 GetWidth() const { return m_Specification.Width; }
		uint32 GetHeight() const { return m_Specification.Height; }

		void* GetBuffer() const { return m_AllocationInfo.pMappedData; }
		uint64 GetSize() const { return m_AllocationInfo.size; }

		VkImage& GetVulkanImage() { return m_Image; }
	private:
		ImageSpecification m_Specification;
		VkImage m_Image = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_AllocationInfo;
		VkSubresourceLayout m_SubresourceLayout{};
	};
}