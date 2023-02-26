#include "VulkanImage.h"
#include "VulkanContext.h"
#include <stb_image.h>

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
		VK_CALL(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &m_Image, &m_ImageAllocation, &m_ImageAllocationInfo));
	}

	void VulkanImage::Destroy()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyImage(allocator, m_Image, m_ImageAllocation);
		m_Image = VK_NULL_HANDLE;
		m_ImageAllocation = VK_NULL_HANDLE;
	}


	VulkanTexture2D::VulkanTexture2D(const TextureSpecification& specification)
		: m_Specification(specification)
	{
		Invalidate();
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		Destroy();
	}

	void VulkanTexture2D::Invalidate()
	{
		int32 width = 0, height = 0;
		uint8* pixels = (uint8*)stbi_load(m_Specification.AssetPath.string().c_str(), &width, &height, nullptr, STBI_rgb_alpha);

		ImageSpecification specification{};
		specification.Width = (uint32)width;
		specification.Height = (uint32)height;
		m_Image = Ref<VulkanImage>::Create(specification);
		
		const VmaAllocationInfo& allocationInfo = m_Image->GetAllocationInfo();
		memcpy(allocationInfo.pMappedData, pixels, allocationInfo.size);
		stbi_image_free(pixels);

		CreateImageView();

		Vulkan::TransitionImageLayout(
			m_Image->GetVulkanImage(), 
			VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}

	void VulkanTexture2D::Destroy()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		vkDestroyImageView(device, m_ImageView, nullptr);
		m_ImageView = VK_NULL_HANDLE;
	}
	
	void VulkanTexture2D::CreateImageView()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext        = nullptr;
		imageViewCreateInfo.flags        = 0u;
		imageViewCreateInfo.image        = m_Image->GetVulkanImage();
		imageViewCreateInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format       = VK_FORMAT_R8G8B8A8_UNORM;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel   = 0u;
		imageViewCreateInfo.subresourceRange.levelCount     = 1u;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0u;
		imageViewCreateInfo.subresourceRange.layerCount     = 1u;
		VK_CALL(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_ImageView));
	}
}