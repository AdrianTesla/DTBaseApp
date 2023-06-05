#include "VulkanResources.h"
#include "VulkanContext.h"

namespace DT
{

#pragma region VulkanVertexBuffer

	VulkanVertexBuffer::VulkanVertexBuffer(uint64 size)
		: m_Dynamic(true)
	{
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		Vulkan::CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &allocationCreateInfo, &m_Buffer);
	}

	VulkanVertexBuffer::VulkanVertexBuffer(const void* data, uint64 size)
		: m_Dynamic(false)
	{
		Vulkan::CreateBufferStaging(data, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &m_Buffer);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		Vulkan::DestroyBuffer(m_Buffer);
	}

	void VulkanVertexBuffer::SetData(const void* data, uint64 size)
	{
		ASSERT(m_Dynamic);
		ASSERT(m_Buffer.AllocationInfo.pMappedData != nullptr);
		memcpy(m_Buffer.AllocationInfo.pMappedData, data, size);
	}

	void VulkanVertexBuffer::ReadData(void* dst, uint64 size, uint64 offset)
	{
		memcpy(dst, offset + (uint8*)m_Buffer.AllocationInfo.pMappedData, size);
	}

#pragma endregion
	
#pragma region VulkanIndexBuffer

	VulkanIndexBuffer::VulkanIndexBuffer(uint64 size)
		: m_Dynamic(true)
	{
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		Vulkan::CreateBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &allocationCreateInfo, &m_Buffer);
	}

	VulkanIndexBuffer::VulkanIndexBuffer(const void* data, uint64 size)
		: m_Dynamic(false), m_IndexCount((uint32)(size / sizeof(uint32)))
	{
		Vulkan::CreateBufferStaging(data, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &m_Buffer);
	}
	 
	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyBuffer(allocator, m_Buffer.Buffer, m_Buffer.Allocation);
		m_Buffer.Buffer = VK_NULL_HANDLE;
		m_Buffer.Allocation = VK_NULL_HANDLE;
	}

	void VulkanIndexBuffer::SetData(const void* data, uint64 size)
	{
		ASSERT(m_Dynamic);
		ASSERT(m_Buffer.AllocationInfo.pMappedData != nullptr);
		memcpy(m_Buffer.AllocationInfo.pMappedData, data, size);
	}

#pragma endregion

#pragma region VulkanUniformBuffer

	VulkanUniformBuffer::VulkanUniformBuffer(uint64 size)
	{
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		Vulkan::CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &allocationCreateInfo, &m_Buffer);
		
		m_DescriptorBufferInfo.buffer = m_Buffer.Buffer;
		m_DescriptorBufferInfo.offset = 0u;
		m_DescriptorBufferInfo.range  = size;
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyBuffer(allocator, m_Buffer.Buffer, m_Buffer.Allocation);
		m_Buffer.Buffer = VK_NULL_HANDLE;
		m_Buffer.Allocation = VK_NULL_HANDLE;
	}

	void VulkanUniformBuffer::SetData(const void* data, uint64 size)
	{
		ASSERT(m_Buffer.AllocationInfo.pMappedData != nullptr);
		memcpy(m_Buffer.AllocationInfo.pMappedData, data, size);
	}

#pragma endregion

#pragma region VulkanStorageBuffer

	VulkanStorageBuffer::VulkanStorageBuffer(uint64 size)
	{
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		Vulkan::CreateBuffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, &allocationCreateInfo, &m_Buffer);
		
		m_DescriptorBufferInfo.buffer = m_Buffer.Buffer;
		m_DescriptorBufferInfo.offset = 0u;
		m_DescriptorBufferInfo.range  = size;
	}

	VulkanStorageBuffer::~VulkanStorageBuffer()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyBuffer(allocator, m_Buffer.Buffer, m_Buffer.Allocation);
		m_Buffer.Buffer = VK_NULL_HANDLE;
		m_Buffer.Allocation = VK_NULL_HANDLE;
	}

	void VulkanStorageBuffer::SetData(const void* data, uint64 size)
	{
		ASSERT(m_Buffer.AllocationInfo.pMappedData != nullptr);
		memcpy(m_Buffer.AllocationInfo.pMappedData, data, size);
	}

#pragma endregion

}