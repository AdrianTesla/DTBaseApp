#pragma once
#include "Vulkan.h"
#include "Core/Window.h"

namespace DT
{
	struct SwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		std::vector<VkSurfaceFormatKHR> SurfaceFormats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class VulkanSwapchain
	{
	public:
		void Init(bool verticalSyncEnabled = true);
		void Shutdown();

		void OnWindowResize();

		bool AquireNextImage();
		void Present(VkSemaphore waitSemaphore);
		void SetVerticalSync(bool enabled);

		uint32 GetImageCount() const { return m_ImageCount; }
		uint32 GetCurrentImageIndex() const { return m_CurrentImageIndex; }

		uint32 GetWidth() const { return (uint32)m_Width; }
		uint32 GetHeight() const { return (uint32)m_Height; }

		VkSemaphore& GetImageAvailableSemaphore(uint32 frameIndex);

		VkRenderPass GetSwapchainRenderPass() const { return m_SwapchainRenderPass; }
		VkSampleCountFlagBits GetSwapchainSampleCount() const { return m_MSAACount; }
		VkFramebuffer GetActiveFramebuffer() const { return m_SwapchainFramebuffers[m_CurrentImageIndex]; }
		VkImageView GetActiveImageView() const { return m_SwapchainImageViews[m_CurrentImageIndex]; }
	private:
		void Invalidate();
		void DestroyDynamicResources();

		void GetSupportDetails();
		void SelectSurfaceFormat();
		void SelectPresentMode();
		void SelectSwapExtent();
		void SelectImageCount();
		void SelectImageUsage();
		void SelectCompositeAlpha();
		void SelectSurfaceTransform();
		void CreateSwapchain();
		void CreateSwapchainImageViews();
		void CreateSwapchainFramebuffers();
		void CreateSwapchainRenderPass();
		void CreateSyncronizationObjects();

		// temp
		void CreateMultisampledImages();
		void CreateDepthImage();
	private:
		SwapchainSupportDetails m_SupportDetails;

		VkSurfaceFormatKHR m_SurfaceFormat;
		VkPresentModeKHR m_PresentMode;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

		int32 m_Width;
		int32 m_Height;

		uint32 m_ImageCount = 0u;
		VkImageUsageFlags m_SwapImageUsage;
		VkCompositeAlphaFlagBitsKHR m_CompositeAlpha;
		VkSurfaceTransformFlagBitsKHR m_SurfaceTransform;


		// temp
		VkImage m_MSAAColorImage = VK_NULL_HANDLE;
		VmaAllocation m_MSAAColorImageAllocation = VK_NULL_HANDLE;
		VkImageView m_MSAAColorImageView = VK_NULL_HANDLE;

		VkImage m_DepthImage = VK_NULL_HANDLE;
		VmaAllocation m_DepthImageAllocation = VK_NULL_HANDLE;
		VkImageView m_DepthImageView = VK_NULL_HANDLE;
		VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;
		VkSampleCountFlagBits m_MSAACount = VK_SAMPLE_COUNT_1_BIT;

		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		FixedArray<VkImage> m_SwapchainImages;
		FixedArray<VkImageView> m_SwapchainImageViews;
		FixedArray<VkFramebuffer> m_SwapchainFramebuffers;
		uint32 m_CurrentImageIndex = 0u;

		VkRenderPass m_SwapchainRenderPass = VK_NULL_HANDLE;

		InFlight<VkSemaphore> m_ImageAvailableSemaphores{};

		bool m_OldVerticalSync = true;
		bool m_VerticalSync = true;
		bool m_LogCreation = true;
		bool m_SurfaceResized = false;
	};
}