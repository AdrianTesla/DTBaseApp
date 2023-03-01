#pragma once
#include "Vulkan.h"

namespace DT
{
	struct ImageSpecification
	{
		uint32 Width = 1u;
		uint32 Height = 1u;
		uint32 Depth = 1u;
		uint32 MipLevels = 1u;
		uint32 ArrayLayers = 1u;
		ImageFormat Format = ImageFormat::RGBA8;
		ImageUsageFlags UsageFlags = ImageUsage::Texture;
	};

	class VulkanImage2D : public RefCounted
	{
	public:
		VulkanImage2D(const ImageSpecification& specification);
		~VulkanImage2D();

		void Invalidate();
		void Destroy();

		const VmaAllocationInfo& GetAllocationInfo() const { return m_ImageAllocationInfo; }
		VkImage GetVulkanImage() const { return m_Image; }
		VkFormat GetVulkanFormat() const { return Convert::ToVulkanFormat(m_Specification.Format); }

		uint32 GetWidth() const { return m_Specification.Width; }
		uint32 GetHeight() const { return m_Specification.Height; }

		void TransitionImageLayout(VkImageLayout newLayout);
		void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

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
	};

	class VulkanTexture2D : public RefCounted
	{
	public:
		VulkanTexture2D(const TextureSpecification& specification);
		~VulkanTexture2D();

		void Invalidate();
		void Destroy();

		VkImageView GetVulkanImageView() const { return m_ImageView; }
		Ref<VulkanImage2D> GetImage() const { return m_Image; }
	private:
		struct ImageData
		{
			void* Data = nullptr;
			uint32 Size = 0u;
			uint32 Width = 0u;
			uint32 Height = 0u;
			ImageFormat Format = ImageFormat::None;
		};
		void LoadImageFile(ImageData* imageData);
		void CreateImageView();
	private:
		Ref<VulkanImage2D> m_Image;
		VkImageView m_ImageView = VK_NULL_HANDLE;
		TextureSpecification m_Specification;
	};
}