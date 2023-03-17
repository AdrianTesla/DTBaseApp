#include "VulkanRenderer.h"
#include "VulkanContext.h"

namespace DT
{
	void VulkanRenderer::Init()
	{
		s_Instance = this;

		m_Swapchain.Init();
		CreateSyncronizationObjects();
	}

	void VulkanRenderer::CreateSyncronizationObjects()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
        
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			VK_CALL(vkCreateFence(device, &fenceCreateInfo, nullptr, &m_PreviousFrameFinishedFences[i]));
        
		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = 0u;
        
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			VK_CALL(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_RenderCompleteSemaphores[i]));
	}

	void VulkanRenderer::Shutdown()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

        for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CALL(vkWaitForFences(device, 1u, &m_PreviousFrameFinishedFences[i], VK_TRUE, UINT64_MAX));
            vkDestroyFence(device, m_PreviousFrameFinishedFences[i], nullptr);
            vkDestroySemaphore(device, m_RenderCompleteSemaphores[i], nullptr);
        }

		m_Swapchain.Shutdown();
	}

	void VulkanRenderer::BeginFrame()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VK_CALL(vkWaitForFences(device, 1u, &m_PreviousFrameFinishedFences[m_CurrentFrame], VK_TRUE, UINT64_MAX));

        m_AquireNextImageFailed = !m_Swapchain.AquireNextImage();
		if (m_AquireNextImageFailed)
			return;

		VK_CALL(vkResetFences(device, 1u, &m_PreviousFrameFinishedFences[m_CurrentFrame]));
	}

	void VulkanRenderer::EndFrame()
	{
		if (m_AquireNextImageFailed)
			return;

		m_Swapchain.Present(m_RenderCompleteSemaphores[m_CurrentFrame]);

		m_CurrentFrame++;
		if (m_CurrentFrame == MAX_FRAMES_IN_FLIGHT)
            m_CurrentFrame = 0u;
	}

	void VulkanRenderer::OnWindowResize()
	{
		m_Swapchain.OnWindowResize();
	}

	void VulkanRenderer::SetVerticalSync(bool enabled)
	{
		m_Swapchain.SetVerticalSync(enabled);
	}
}