#include "VulkanBuffers.h"
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
	
	VulkanIndexBuffer::VulkanIndexBuffer(const void* data, uint64 size, VkIndexType indexType)
		: m_IndexType(indexType)
	{
		switch (m_IndexType)
		{
			case VK_INDEX_TYPE_UINT16: m_IndexCount = (uint32)(size / sizeof(uint16)); break;
			case VK_INDEX_TYPE_UINT32: m_IndexCount = (uint32)(size / sizeof(uint32)); break;
			default: ASSERT(false);
		}

		Vulkan::CreateBufferStaging(data, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, &m_Buffer, &m_Allocation);
	}
	 
	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyBuffer(allocator, m_Buffer, m_Allocation);
		m_Buffer = VK_NULL_HANDLE;
		m_Allocation = VK_NULL_HANDLE;
	}
}