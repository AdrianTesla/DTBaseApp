#pragma once
#include "Renderer/RendererBackend.h"
#include "VulkanSwapchain.h"

namespace DT
{
	class VulkanRenderer : public RendererBackend
	{
	public:
		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual void OnWindowResize() override;
		virtual void SetVerticalSync(bool enabled) override;

		virtual uint32 CurrentFrame() const override { return m_CurrentFrame; }

		static VulkanSwapchain& GetSwapchain() { return s_Instance->m_Swapchain; }
		static VkSemaphore& GetActiveRenderCompleteSemaphore() { return s_Instance->m_RenderCompleteSemaphores[s_Instance->m_CurrentFrame]; }
		static VkFence& GetActivePreviousFrameFence() { return s_Instance->m_PreviousFrameFinishedFences[s_Instance->m_CurrentFrame]; }
	private:
		void CreateSyncronizationObjects();
	private:
		inline static VulkanRenderer* s_Instance = nullptr;

		VulkanSwapchain m_Swapchain;

		uint32 m_CurrentFrame = 0u;
		bool m_AquireNextImageFailed = false;

		InFlight<VkFence> m_PreviousFrameFinishedFences{};
		InFlight<VkSemaphore> m_RenderCompleteSemaphores{};
	};
}