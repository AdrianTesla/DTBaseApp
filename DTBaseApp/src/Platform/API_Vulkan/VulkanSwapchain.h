#pragma once
#include "Vulkan.h"
#include "Core/Window.h"

namespace DT
{
	template<typename T, uint32 MaxSize = 10u>
	struct FixedArray
	{
		FixedArray() = default;
		FixedArray(uint32 size)
			: Size(size)
		{}

		T& operator[](uint32 index) { ASSERT(index < Size); return Data[index]; }
		const T& operator[](uint32 index) const { ASSERT(index < Size); return Data[index]; }

		T Data[MaxSize];
		uint32 Size = 0u;
	};

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

		void AquireNextImage();
		void Present(VkSemaphore waitSemaphore);

		uint32 GetImageCount() const { return m_ImageCount; }
		uint32 GetCurrentImageIndex() const { return m_CurrentImageIndex; }

		uint32 GetWidth() const { return (uint32)m_Width; }
		uint32 GetHeight() const { return (uint32)m_Height; }

		VkRenderPass GetSwapchainRenderPass() const { return m_SwapchainRenderPass; }
		VkSemaphore& GetImageAvailableSemaphore();

		VkFramebuffer GetFramebuffer(uint32 imageIndex) const { return m_SwapchainFramebuffers[imageIndex]; }
		VkImageView GetImageView(uint32 imageIndex) const { return m_SwapchainImageViews[imageIndex]; }
	private:
		void RecreateSwapchain();
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

		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		FixedArray<VkImage> m_SwapchainImages;
		FixedArray<VkImageView> m_SwapchainImageViews;
		FixedArray<VkFramebuffer> m_SwapchainFramebuffers;
		uint32 m_CurrentImageIndex = 0u;

		VkRenderPass m_SwapchainRenderPass = VK_NULL_HANDLE;

		InFlight<VkSemaphore> m_ImageAvailableSemaphores;

		bool m_VerticalSync;
		bool m_LogCreation = true;
	};
}