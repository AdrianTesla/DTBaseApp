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

		const VmaAllocationInfo& GetAllocationInfo() const { return m_ImageAllocationInfo; }
		VkImage GetVulkanImage() const { return m_Image; }
	private:
		VkImage m_Image = VK_NULL_HANDLE;
		VmaAllocation m_ImageAllocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_ImageAllocationInfo{};

		ImageSpecification m_Specification;
	};

	struct TextureSpecification
	{
		std::filesystem::path AssetPath;
	};

	class VulkanTexture2D : public RefCounted
	{
	public:
		VulkanTexture2D(const TextureSpecification& specification);
		~VulkanTexture2D();

		void Invalidate();
		void Destroy();

		VkImageView GetVulkanImageView() const { return m_ImageView; }
		Ref<VulkanImage> GetImage() const { return m_Image; }
	private:
		void CreateImageView();
	private:
		Ref<VulkanImage> m_Image;
		VkImageView m_ImageView;
		TextureSpecification m_Specification;
	};
}