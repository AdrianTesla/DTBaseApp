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
		imageCreateInfo.imageType			  = Convert::ToVulkanImageType(m_Specification.Type);
		imageCreateInfo.format				  = Convert::ToVulkanFormat(m_Specification.Format);
		imageCreateInfo.extent.width		  = m_Specification.Width;
		imageCreateInfo.extent.height		  = m_Specification.Height;
		imageCreateInfo.extent.depth		  = m_Specification.Depth;
		imageCreateInfo.mipLevels			  = m_Specification.MipLevels;
		imageCreateInfo.arrayLayers			  = m_Specification.ArrayLayers;
		imageCreateInfo.samples				  = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling				  = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage				  = Convert::ToVulkanImageUsageFlags(m_Specification.UsageFlags);
		imageCreateInfo.sharingMode			  = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0u;
		imageCreateInfo.pQueueFamilyIndices	  = nullptr;
		imageCreateInfo.initialLayout		  = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		VK_CALL(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &m_Image, &m_ImageAllocation, &m_ImageAllocationInfo));
		
		m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
		VulkanDevice vulkanDevice = VulkanContext::GetCurrentDevice();
		
		VkCommandBuffer commandBuffer = vulkanDevice.BeginCommandBuffer(QueueType::Graphics);
		Vulkan::TransitionImageLayout(commandBuffer, m_Image, m_CurrentLayout, newLayout);
		vulkanDevice.EndCommandBuffer();

		m_CurrentLayout = newLayout;
	}

	void VulkanImage::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VulkanDevice vulkanDevice = VulkanContext::GetCurrentDevice();

		VkCommandBuffer commandBuffer = vulkanDevice.BeginCommandBuffer(QueueType::Graphics);
		Vulkan::TransitionImageLayout(commandBuffer, m_Image, oldLayout, newLayout);
		vulkanDevice.EndCommandBuffer();
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

		ImageData imageData;
		LoadImageFile(&imageData);

		// create the device local image
		ImageSpecification specification{};
		specification.Width = imageData.Width;
		specification.Height = imageData.Height;
		specification.Format = imageData.Format;
		specification.MipLevels = Utils::CalculateMipLevels(specification.Width, specification.Height, specification.Depth);
		specification.UsageFlags = ImageUsage::Texture | ImageUsage::TransferDst;
		m_Image = Ref<VulkanImage>::Create(specification);

		// upload the pixels to the staging buffer
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		VkBuffer stagingBuffer = VK_NULL_HANDLE;		   
		VmaAllocation stagingAllocation = VK_NULL_HANDLE;  
		VmaAllocationInfo stagingAllocationInfo{};		   

		VmaAllocationCreateInfo stagingAllocationCreateInfo{};
		stagingAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		stagingAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		Vulkan::CreateBuffer(imageData.Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingAllocationCreateInfo, &stagingBuffer, &stagingAllocation, &stagingAllocationInfo);
		
		ASSERT(stagingAllocationInfo.pMappedData != nullptr)
		memcpy(stagingAllocationInfo.pMappedData, imageData.Data, imageData.Size);
		stbi_image_free(imageData.Data);

		Timer timer;

		VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();
		VkCommandBuffer commandBuffer = vulkanDevice.BeginCommandBuffer(QueueType::Graphics);
		{
			Vulkan::TransitionImageLayout(commandBuffer, m_Image->GetVulkanImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			Vulkan::CopyBufferToImage(commandBuffer, stagingBuffer, m_Image->GetVulkanImage(), imageData.Width, imageData.Height);
			Vulkan::TransitionImageLayout(commandBuffer, m_Image->GetVulkanImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		vulkanDevice.EndCommandBuffer();

		LOG_WARN("Transition, copy, transition: {} us", timer.ElapsedMicroseconds());

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
		CreateImageView();
	}

	void VulkanTexture2D::Destroy()
	{
		m_Image.Reset();
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		vkDestroyImageView(device, m_ImageView, nullptr);
		m_ImageView = VK_NULL_HANDLE;
	}
	
	void VulkanTexture2D::LoadImageFile(ImageData* imageData)
	{
		std::string pathString = m_Specification.AssetPath.string();
		const char* filepath = pathString.c_str();
		int32 width, height;
		if (stbi_is_hdr(filepath)) {
			imageData->Data = (uint8*)stbi_loadf(filepath, &width, &height, nullptr, STBI_rgb_alpha);
			imageData->Size = (uint64)(width * height * sizeof(float) * 4u);
			imageData->Format = ImageFormat::RGBA32F;
		} else {
			imageData->Data = (uint8*)stbi_load(filepath, &width, &height, nullptr, STBI_rgb_alpha);
			imageData->Size = (uint64)(width * height * sizeof(uint8) * 4u);
			imageData->Format = ImageFormat::RGBA8;
		}
		imageData->Width = (uint32)width;
		imageData->Height = (uint32)height;
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
		imageViewCreateInfo.format       = m_Image->GetVulkanFormat();
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