#include "Vulkan.h"
#include "VulkanContext.h"
#include "VulkanDevice.h"

namespace DT::Vulkan
{
	void QueueSubmit(VkQueue queue, VkCommandBuffer commandBuffer, VkFence fence)
	{
		VkSubmitInfo submitInfo{};
		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext                = nullptr;
		submitInfo.waitSemaphoreCount   = 0u;
		submitInfo.pWaitSemaphores      = nullptr;
		submitInfo.pWaitDstStageMask    = nullptr;
		submitInfo.commandBufferCount   = 1u;
		submitInfo.pCommandBuffers      = &commandBuffer;
		submitInfo.signalSemaphoreCount = 0u;
		submitInfo.pSignalSemaphores    = nullptr;
		VK_CALL(vkQueueSubmit(queue, 1u, &submitInfo, fence));
	}

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaAllocationCreateInfo* pAllocationCreateInfo, VkBuffer* pBuffer, VmaAllocation* pAllocation, VmaAllocationInfo* pAllocationInfo)
	{
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext                 = nullptr;
		bufferCreateInfo.flags                 = 0u;
		bufferCreateInfo.size                  = size;
		bufferCreateInfo.usage                 = usage;
		bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.queueFamilyIndexCount = 0u;
		bufferCreateInfo.pQueueFamilyIndices   = nullptr;
		VK_CALL(vmaCreateBuffer(allocator, &bufferCreateInfo, pAllocationCreateInfo, pBuffer, pAllocation, pAllocationInfo));
	}

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();

		VkDevice device = vulkanDevice.GetVulkanDevice();
		VkCommandPool transferPool = vulkanDevice.GetTransferCommandPool(true);
		VkQueue transferQueue = vulkanDevice.GetTransferQueue();

		VkCommandBuffer copyCommandBuffer;
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext              = nullptr;
		commandBufferAllocateInfo.commandPool        = transferPool;
		commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1u;
		VK_CALL(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &copyCommandBuffer));

		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext            = nullptr;
		commandBufferBeginInfo.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		VkBufferCopy bufferCopyRegion{};
		bufferCopyRegion.srcOffset = 0u;
		bufferCopyRegion.dstOffset = 0u; 
		bufferCopyRegion.size      = size;

		VK_CALL(vkBeginCommandBuffer(copyCommandBuffer, &commandBufferBeginInfo));
		vkCmdCopyBuffer(copyCommandBuffer, srcBuffer, dstBuffer, 1u, &bufferCopyRegion);
		VK_CALL(vkEndCommandBuffer(copyCommandBuffer));

		VkSubmitInfo submitInfo{};
		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext                = nullptr;
		submitInfo.waitSemaphoreCount   = 0u;
		submitInfo.pWaitSemaphores      = nullptr;
		submitInfo.pWaitDstStageMask    = nullptr;
		submitInfo.commandBufferCount   = 1u;
		submitInfo.pCommandBuffers      = &copyCommandBuffer;
		submitInfo.signalSemaphoreCount = 0u;
		submitInfo.pSignalSemaphores    = nullptr;
		VK_CALL(vkQueueSubmit(transferQueue, 1u, &submitInfo, VK_NULL_HANDLE));
		VK_CALL(vkQueueWaitIdle(transferQueue));

		vkFreeCommandBuffers(device, transferPool, 1u, &copyCommandBuffer);
	}

	void CreateBufferStaging(const void* data, uint64 size, VkBufferUsageFlags usage, VkBuffer* pBuffer, VmaAllocation* pAllocation)
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		// create staging buffer and upload data to it
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingAllocation = VK_NULL_HANDLE;
		VmaAllocationInfo stagingAllocationInfo{};
		
		VmaAllocationCreateInfo stagingAllocationCreateInfo{};
		stagingAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		stagingAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &stagingAllocationCreateInfo, &stagingBuffer, &stagingAllocation, &stagingAllocationInfo);
		ASSERT(stagingAllocationInfo.pMappedData != nullptr)
		memcpy(stagingAllocationInfo.pMappedData, data, size);

		// create device local buffer
		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		CreateBuffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, &allocationCreateInfo, pBuffer, pAllocation);
		CopyBuffer(stagingBuffer, *pBuffer, size);

		// destroy staging buffer and free staging allocation
		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
	}
}