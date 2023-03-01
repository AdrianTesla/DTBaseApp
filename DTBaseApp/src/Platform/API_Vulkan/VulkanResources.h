#pragma once
#include "Vulkan.h"

namespace DT
{
	class VulkanVertexBuffer : public RefCounted
	{
	public:
		VulkanVertexBuffer(const void* data, uint64 size);
		~VulkanVertexBuffer();

		VkBuffer& GetVulkanBuffer() { return m_Buffer.Buffer; }
	private:
		VulkanBuffer m_Buffer;
	};

	class VulkanIndexBuffer : public RefCounted
	{
	public:
		VulkanIndexBuffer(const void* data, uint64 size);
		~VulkanIndexBuffer();
		
		uint32 GetIndexCount() const { return m_IndexCount; }
		VkBuffer& GetVulkanBuffer() { return m_Buffer.Buffer; }
	private:
		VulkanBuffer m_Buffer;
		uint32 m_IndexCount = 0u;
	};

	class VulkanUniformBuffer : public RefCounted
	{
	public:
		VulkanUniformBuffer(uint64 size);
		~VulkanUniformBuffer();

		void SetData(const void* data, uint64 size);
		VkBuffer& GetVulkanBuffer() { return m_Buffer.Buffer; }
		const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const { return m_DescriptorBufferInfo; }
	private:
		VulkanBuffer m_Buffer;
		VkDescriptorBufferInfo m_DescriptorBufferInfo{};
	};

	class VulkanStorageBuffer : public RefCounted
	{
	public:
		VulkanStorageBuffer(uint64 size);
		~VulkanStorageBuffer();

		void SetData(const void* data, uint64 size);
		VkBuffer& GetVulkanBuffer() { return m_Buffer.Buffer; }
		const VkDescriptorBufferInfo& GetDescriptorBufferInfo() const { return m_DescriptorBufferInfo; }
	private:
		VulkanBuffer m_Buffer;
		VkDescriptorBufferInfo m_DescriptorBufferInfo{};
	};
}