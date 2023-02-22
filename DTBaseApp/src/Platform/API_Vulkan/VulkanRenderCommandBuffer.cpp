#include "VulkanRenderCommandBuffer.h"
#include "VulkanContext.h"

namespace DT
{
	VulkanRenderCommandBuffer::VulkanRenderCommandBuffer()
	{
		VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();
		VkDevice device = vulkanDevice.GetVulkanDevice();

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext              = nullptr;
		commandBufferAllocateInfo.commandPool        = vulkanDevice.GetGraphicsCommandPool();
		commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		VK_CALL(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, m_CommandBuffers.Data()));
	}

	VulkanRenderCommandBuffer::~VulkanRenderCommandBuffer()
	{
	}

	void VulkanRenderCommandBuffer::Begin()
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext            = nullptr;
		commandBufferBeginInfo.flags            = 0u;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;
		VK_CALL(vkBeginCommandBuffer(m_CommandBuffers[VulkanContext::CurrentFrame()], &commandBufferBeginInfo));
	}

	void VulkanRenderCommandBuffer::End()
	{
		VK_CALL(vkEndCommandBuffer(m_CommandBuffers[VulkanContext::CurrentFrame()]));
	}
	VkCommandBuffer VulkanRenderCommandBuffer::GetActiveCommandBuffer() const
	{
		return m_CommandBuffers[VulkanContext::CurrentFrame()];
	}
}