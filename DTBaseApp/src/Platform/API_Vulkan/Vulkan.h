#pragma once
#include <Core/Core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#if DT_DEBUG
	#define VK_CALL(call)                          \
	{ 							                   \
		VkResult result = (call);                  \
		if (result != VK_SUCCESS)                  \
		{						                   \
			LOG_CRITICAL(string_VkResult(result)); \
			ASSERT(false); 		                   \
		} 						                   \
	}
#else
	#define VK_CALL(call) (call)
#endif

#ifdef DT_DEBUG
	static constexpr bool s_ValidationLayerEnabled = true;
#else
	static constexpr bool s_ValidationLayerEnabled = false;
#endif

#define VK_VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

#define GET_INSTANCE_FUNC(x) ((PFN_##x)vkGetInstanceProcAddr(m_Instance, #x))

namespace DT
{
	static constexpr uint32 MAX_FRAMES_IN_FLIGHT = 2u;

	template<typename T>
	struct InFlight
	{
		InFlight() = default;

		T& operator[](uint32 index) { return m_Instances[index]; }
		const T& operator[](uint32 index) const { return m_Instances[index]; }

		T* Data() { return m_Instances; }
		const T* Data() const { return m_Instances; }
	private:
		T m_Instances[MAX_FRAMES_IN_FLIGHT];
	};

	enum class ImageFormat
	{
		None,
		RGBA8,
		RGBA32F
	};

	enum class ImageType
	{
		Image1D,
		Image2D,
		Image3D
	};

	enum class PolygonMode
	{
		Fill,
		Wireframe,
		Point
	};

	namespace ImageUsage
	{
		enum Usage
		{
			TransferSrc         = Bit(0),
			TransferDst         = Bit(1),
			Texture             = Bit(2),
			Storage             = Bit(3),
			ColorAttachment     = Bit(4),
			DepthAttachment     = Bit(5),
			TransientAttachment = Bit(6),
			InputAttachment     = Bit(7)
		};
	}
	typedef uint32 ImageUsageFlags;

	struct VulkanBuffer
	{
		VkBuffer Buffer = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo AllocationInfo{};
	};

	struct VulkanImage
	{
		VkImage Image = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo AllocationInfo{};
	};

	namespace Convert
	{
		VkFormat ToVulkanFormat(ImageFormat format);
		VkImageType ToVulkanImageType(ImageType imageType);
		VkImageUsageFlagBits ToVulkanImageUsage(ImageUsage::Usage usage);
		VkImageUsageFlags ToVulkanImageUsageFlags(ImageUsageFlags usageFlags);
		VkPolygonMode ToVulkanPolygonMode(PolygonMode polygonMode);
	}

	namespace Vulkan
	{
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateInfo* pAllocationCreateInfo, VulkanBuffer* pBuffer);
		void CreateBufferStaging(const void* data, uint64 size, VkBufferUsageFlags usage, VulkanBuffer* pBuffer);
	}

	namespace vkCmd
	{
		void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, uint32 width, uint32 height);
		void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	}
}