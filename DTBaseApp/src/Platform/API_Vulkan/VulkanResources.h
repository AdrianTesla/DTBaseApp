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
		
		uint32 GetIndexCount() const { return m_IndexCount; }
		VkBuffer& GetVulkanBuffer() { return m_Buffer; }
	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		uint32 m_IndexCount = 0u;
	};

	class VulkanUniformBuffer : public RefCounted
	{
	public:
		VulkanUniformBuffer(uint64 size);
		~VulkanUniformBuffer();

		void SetData(const void* data, uint64 size);
		VkBuffer& GetVulkanBuffer() { return m_Buffer; }
		const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const { return m_DescriptorBufferInfo; }
	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_AllocationInfo;
		uint64 m_Size = 0u;

		VkDescriptorBufferInfo m_DescriptorBufferInfo{};
	};
}