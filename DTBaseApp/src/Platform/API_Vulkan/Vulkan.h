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
		RGBA32F,
		D32
	};

	enum class ImageUsage
	{
		Texture,
		Attachment,
		Storage
	};

	enum class PolygonMode
	{
		Fill,
		Wireframe,
		Point
	};

	enum class PrimitiveTopology
	{
		TriangleList,
		LineList
	};

	enum class FaceCulling
	{
		Back,
		Front,
		None
	};

	struct VulkanBuffer
	{
		VkBuffer Buffer = VK_NULL_HANDLE;
		VmaAllocation Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo AllocationInfo{};
	};

	struct MemoryDependency
	{
		VkPipelineStageFlags SrcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		VkPipelineStageFlags DstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		VkAccessFlags SrcAccess = VK_ACCESS_NONE;
		VkAccessFlags DstAccess = VK_ACCESS_NONE;
	};

	namespace Convert
	{
		VkFormat ToVulkanFormat(ImageFormat format);
		VkPolygonMode ToVulkanPolygonMode(PolygonMode polygonMode);
		VkPrimitiveTopology ToVulkanPrimitiveTopology(PrimitiveTopology polygonMode);
		VkCullModeFlags ToVulkanCullMode(FaceCulling culling);
	}

	namespace Vulkan
	{
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateInfo* pAllocationCreateInfo, VulkanBuffer* pBuffer);
		void CreateBufferStaging(const void* data, uint64 size, VkBufferUsageFlags usage, VulkanBuffer* pBuffer);
		bool HasDepthComponent(VkFormat format);
		bool HasStencilComponent(VkFormat format);
		bool HasColorComponent(VkFormat format);
		MemoryDependency CalculateMemoryDependency(VkImageLayout oldLayout, VkImageLayout newLayout);
		VkImageAspectFlags GetImageAspectFlags(VkFormat format);
	}

	namespace vkCmd
	{
		void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, uint32 width, uint32 height);
		void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& subresource, const MemoryDependency& dependency);
		void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, const VkImageSubresourceRange& subresource);
		void TransitionImageLayoutAllMips(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkFormat format, uint32 mipLevels);
		void TransitionImageLayoutSingleMip(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkFormat format, uint32 mip);
	}
}