#include "VulkanImage.h"
#include "VulkanContext.h"
#include <stb_image.h>

namespace DT
{
	namespace Utils
	{
		uint32 CalculateMipLevels(uint32 width, uint32 height, uint32 depth = 1u)
		{
			return 1u + (uint32)std::floorf(std::log2f((float)std::max({ width,height,depth })));
		}
	}

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
		imageCreateInfo.format				  = Convert::ToVulkanFormat(m_Specification.Format);
		imageCreateInfo.extent.width		  = m_Specification.Width;
		imageCreateInfo.extent.height		  = m_Specification.Height;
		imageCreateInfo.extent.depth		  = 1u;
		imageCreateInfo.mipLevels			  = m_Specification.Dynamic ? 1u : m_Specification.MipLevels;
		imageCreateInfo.arrayLayers			  = m_Specification.ArrayLayers;
		imageCreateInfo.samples				  = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling				  = (m_Specification.Dynamic ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL);
		imageCreateInfo.usage				  = VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode			  = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0u;
		imageCreateInfo.pQueueFamilyIndices	  = nullptr;
		imageCreateInfo.initialLayout		  = VK_IMAGE_LAYOUT_UNDEFINED;

		if (!m_Specification.Dynamic)
			imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		
		if (m_Specification.Dynamic) 
			allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VK_CALL(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &m_Image, &m_ImageAllocation, &m_ImageAllocationInfo));
	}

	void VulkanImage::Destroy()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyImage(allocator, m_Image, m_ImageAllocation);
		m_Image = VK_NULL_HANDLE;
		m_ImageAllocation = VK_NULL_HANDLE;
	}

	void VulkanImage::TransitionImageLayout(VkImageLayout newLayout)
	{
		Vulkan::TransitionImageLayout(m_Image, m_CurrentLayout, newLayout);
		m_CurrentLayout = newLayout;
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
		Destroy();

		// load the image file from disk
		std::string pathString = m_Specification.AssetPath.string();
		const char* filepath = pathString.c_str();

		uint8* pixels = nullptr;
		uint64 imageSize = 0u;
		int32 width = 0, height = 0;
		ImageFormat format = ImageFormat::None;

		if (stbi_is_hdr(filepath)) {
			pixels = (uint8*)stbi_loadf(filepath, &width, &height, nullptr, STBI_rgb_alpha);
			imageSize = (uint64)(width * height * sizeof(float) * 4u);
			format = ImageFormat::RGBA32F;
		} else {
			pixels = (uint8*)stbi_load(filepath, &width, &height, nullptr, STBI_rgb_alpha);
			imageSize = (uint64)(width * height * sizeof(uint8) * 4u);
			format = ImageFormat::RGBA8;
		}

		// create the device local image
		ImageSpecification specification{};
		specification.Width = (uint32)width;
		specification.Height = (uint32)height;
		specification.Dynamic = m_Specification.Dynamic;
		specification.MipLevels = Utils::CalculateMipLevels(specification.Width, specification.Height);
		m_Image = Ref<VulkanImage>::Create(specification);

		if (m_Specification.Dynamic)
		{
			void* mappedMemory = m_Image->GetAllocationInfo().pMappedData;
			ASSERT(mappedMemory != nullptr)
			memcpy(mappedMemory, pixels, imageSize);
		}
		else
		{
			m_Image->TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			// upload the pixels to the staging buffer
			VkDevice device = VulkanContext::GetCurrentVulkanDevice();
			VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

			VkBuffer stagingBuffer = VK_NULL_HANDLE;		   
			VmaAllocation stagingAllocation = VK_NULL_HANDLE;  
			VmaAllocationInfo stagingAllocationInfo{};		   

			VmaAllocationCreateInfo stagingAllocationCreateInfo{};
			stagingAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
			stagingAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
			Vulkan::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingAllocationCreateInfo, &stagingBuffer, &stagingAllocation, &stagingAllocationInfo);
		
			ASSERT(stagingAllocationInfo.pMappedData != nullptr)
			memcpy(stagingAllocationInfo.pMappedData, pixels, imageSize);

			// now transfer the staging buffer to the GPU
			VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();

			VkCommandBuffer commandBuffer = vulkanDevice.BeginCommandBuffer(QueueType::Transfer);
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
				copyRegion.imageExtent.width               = (uint32)width;
				copyRegion.imageExtent.height              = (uint32)height;
				copyRegion.imageExtent.depth               = 1u;
				vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, m_Image->GetVulkanImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &copyRegion);
			}
			vulkanDevice.EndCommandBuffer();

			vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
		}
		m_Image->TransitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		stbi_image_free(pixels);

		CreateImageView();
	}

	void VulkanTexture2D::Destroy()
	{
		m_Image.Reset();
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
		imageViewCreateInfo.format       = Convert::ToVulkanFormat(m_Image->GetSpecification().Format);
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