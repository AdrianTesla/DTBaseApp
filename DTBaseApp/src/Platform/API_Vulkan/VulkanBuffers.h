#pragma once
#include "Vulkan.h"

namespace DT
{
	class VulkanVertexBuffer : public RefCounted
	{
	public:
		VulkanVertexBuffer(const void* data, uint64 size);
		~VulkanVertexBuffer();

		VkBuffer& GetVulkanBuffer() { return m_Buffer; }
	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
	};

	class VulkanIndexBuffer : public RefCounted
	{
	public:
		VulkanIndexBuffer(const void* data, uint64 size);
		~VulkanIndexBuffer();
		
		VkBuffer& GetVulkanBuffer() { return m_Buffer; }
	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
	};
}