#include "Renderer.h"
#include "Platform/API_Vulkan/VulkanContext.h"

namespace DT
{
    struct RendererData
    {
        uint32 CurrentFrame = 0u;
        bool AquireNextImageFailed = false;

        InFlight<VkFence> PreviousFrameFinishedFences{};
        InFlight<VkSemaphore> RenderCompleteSemaphores{};
    };

    static RendererData* s_Data = nullptr;

    void Renderer::Init()
    {
        s_Data = new RendererData();
        
        VkDevice device = VulkanContext::GetCurrentVulkanDevice();
        
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			VK_CALL(vkCreateFence(device, &fenceCreateInfo, nullptr, &s_Data->PreviousFrameFinishedFences[i]));
        
		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = 0u;
        
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			VK_CALL(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &s_Data->RenderCompleteSemaphores[i]));
    }

    void Renderer::Shutdown()
    {
        VkDevice device = VulkanContext::GetCurrentVulkanDevice();

        for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyFence(device, s_Data->PreviousFrameFinishedFences[i], nullptr);
            vkDestroySemaphore(device, s_Data->RenderCompleteSemaphores[i], nullptr);
        }

        delete s_Data;
    }

    void Renderer::BeginFrame()
    {
        VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VK_CALL(vkWaitForFences(device, 1u, &s_Data->PreviousFrameFinishedFences[s_Data->CurrentFrame], VK_TRUE, UINT64_MAX));

        s_Data->AquireNextImageFailed = !VulkanContext::GetSwapchain().AquireNextImage();
		if (s_Data->AquireNextImageFailed)
			return;

		VK_CALL(vkResetFences(device, 1u, &s_Data->PreviousFrameFinishedFences[s_Data->CurrentFrame]));
    }

    void Renderer::EndFrame()
    {
        if (s_Data->AquireNextImageFailed)
			return;

        VulkanContext::GetSwapchain().Present(s_Data->RenderCompleteSemaphores[s_Data->CurrentFrame]);

		s_Data->CurrentFrame++;
		if (s_Data->CurrentFrame == MAX_FRAMES_IN_FLIGHT)
            s_Data->CurrentFrame = 0u;
    }

    void Renderer::SetVerticalSync(bool enabled)
    {
        VulkanContext::GetSwapchain().SetVerticalSync(enabled);
    }

    uint32 Renderer::CurrentFrame()
    {
        return s_Data->CurrentFrame;
    }

    VkSemaphore& Renderer::GetActiveRenderCompleteSemaphore()
    {
        return s_Data->RenderCompleteSemaphores[s_Data->CurrentFrame];
    }

    VkFence& Renderer::GetActivePreviousFrameFence()
    {
        return s_Data->PreviousFrameFinishedFences[s_Data->CurrentFrame];
    }
}