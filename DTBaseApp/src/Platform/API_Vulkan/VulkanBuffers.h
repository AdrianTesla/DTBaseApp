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
		VulkanIndexBuffer(const void* data, uint64 size, VkIndexType indexType = VK_INDEX_TYPE_UINT32);
		~VulkanIndexBuffer();
		
		VkIndexType GetVulkanIndexType() const { return m_IndexType; }
		uint32 GetIndexCount() const { return m_IndexCount; }
		VkBuffer& GetVulkanBuffer() { return m_Buffer; }
	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		uint32 m_IndexCount = 0u;
		VkIndexType m_IndexType;
	};
}