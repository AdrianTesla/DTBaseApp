#include "VulkanResources.h"
#include "VulkanContext.h"

namespace DT
{
	VulkanVertexBuffer::VulkanVertexBuffer(const void* data, uint64 size)
	{
		Vulkan::CreateBufferStaging(data, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, &m_Buffer, &m_Allocation);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyBuffer(allocator, m_Buffer, m_Allocation);
		m_Buffer = VK_NULL_HANDLE;
		m_Allocation = VK_NULL_HANDLE;
	}
	
	VulkanIndexBuffer::VulkanIndexBuffer(const void* data, uint64 size)
		: m_IndexCount((uint32)(size / sizeof(uint32)))
	{
		Vulkan::CreateBufferStaging(data, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &m_Buffer, &m_Allocation);
	}
	 
	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyBuffer(allocator, m_Buffer, m_Allocation);
		m_Buffer = VK_NULL_HANDLE;
		m_Allocation = VK_NULL_HANDLE;
	}

	VulkanUniformBuffer::VulkanUniformBuffer(uint64 size)
		: m_Size(size)
	{
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		Vulkan::CreateBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &allocationCreateInfo, &m_Buffer, &m_Allocation, &m_AllocationInfo);
		
		m_DescriptorBufferInfo.buffer = m_Buffer;
		m_DescriptorBufferInfo.offset = 0u;
		m_DescriptorBufferInfo.range  = m_Size;
	}

	void VulkanUniformBuffer::SetData(const void* data, uint64 size)
	{
		ASSERT(m_AllocationInfo.pMappedData != nullptr);
		memcpy(m_AllocationInfo.pMappedData, data, size);
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyBuffer(allocator, m_Buffer, m_Allocation);
		m_Buffer = VK_NULL_HANDLE;
		m_Allocation = VK_NULL_HANDLE;
	}
}