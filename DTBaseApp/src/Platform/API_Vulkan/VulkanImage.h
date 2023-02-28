#pragma once
#include "Vulkan.h"

namespace DT
{
	struct ImageSpecification
	{
		uint32 Width = 1u;
		uint32 Height = 1u;
		uint32 MipLevels = 1u;
		uint32 ArrayLayers = 1u;
		ImageFormat Format = ImageFormat::RGBA8;

		bool Dynamic = false;
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
		VkFormat GetVulkanFormat() const { return Convert::ToVulkanFormat(m_Specification.Format); }

		uint32 GetWidth() const { return m_Specification.Width; }
		uint32 GetHeight() const { return m_Specification.Height; }

		void TransitionImageLayout(VkImageLayout newLayout);

		const ImageSpecification& GetSpecification() const { return m_Specification; }
	private:
		VkImage m_Image = VK_NULL_HANDLE;
		VmaAllocation m_ImageAllocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_ImageAllocationInfo{};
		VkImageLayout m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		ImageSpecification m_Specification;
	};

	struct TextureSpecification
	{
		std::filesystem::path AssetPath;
		bool Dynamic = false;
	};

	class VulkanTexture2D : public RefCounted
	{
	public:
		VulkanTexture2D(const TextureSpecification& specification);
		~VulkanTexture2D();

		void Invalidate();
		void Destroy();

		uint32 GetWidth() const { return m_Image->GetWidth(); }
		uint32 GetHeight() const { return m_Image->GetHeight(); }

		VkImageView GetVulkanImageView() const { return m_ImageView; }
		Ref<VulkanImage> GetImage() const { return m_Image; }
	private:
		void CreateImageView();
	private:
		Ref<VulkanImage> m_Image;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		TextureSpecification m_Specification;
	};
}