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

	enum class ImageUsage
	{

	};

	enum class ImageFormat
	{
		None,
		RGBA8,
		RGBA32F
	};

	namespace Convert
	{
		VkFormat ToVulkanFormat(ImageFormat format);
	}

	namespace Vulkan
	{
		void QueueSubmit(VkQueue queue, VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE);
		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateInfo* pAllocationCreateInfo, VkBuffer* pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo = nullptr);
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void CreateBufferStaging(const void* data, uint64 size, VkBufferUsageFlags usage, VkBuffer* pBuffer, VmaAllocation* pAllocation);
		void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	}
}