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
		void Init(bool verticalSync = false);
		void Shutdown();

		void Resize(int32 width, int32 height);
		void AquireNextImage(VkSemaphore imageAvailableSemaphore);
		void Present(VkSemaphore renderCompleteSemaphore);

		VkFormat GetImageFormat() const { return m_SurfaceFormat.format; }
		uint32 GetImageCount() const { return m_ImageCount; }
		uint32 GetCurrentImageIndex() const { return m_CurrentImageIndex; }

		uint32 GetWidth() const { return (uint32)m_Width; }
		uint32 GetHeight() const { return (uint32)m_Height; }

		const std::vector<VkImageView>& GetImageViews() const { return m_SwapchainImageViews; }
	private:
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
	private:
		SwapchainSupportDetails m_SupportDetails;

		VkSurfaceFormatKHR m_SurfaceFormat;
		VkPresentModeKHR m_PresentMode;
		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;

		uint32 m_ImageCount;
		VkImageUsageFlags m_SwapImageUsage;
		VkCompositeAlphaFlagBitsKHR m_CompositeAlpha;
		VkSurfaceTransformFlagBitsKHR m_SurfaceTransform;

		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		uint32 m_CurrentImageIndex = 0u;

		int32 m_Width;
		int32 m_Height;

		bool m_VerticalSync;
	};
}