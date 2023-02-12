#include "VulkanSwapchain.h"
#include <GLFW/glfw3.h>
#include "VulkanContext.h"

namespace DT
{
	void VulkanSwapchain::Init(bool verticalSync)
	{
		m_VerticalSync = verticalSync;

		GetSupportDetails();
		SelectSurfaceFormat();
		SelectPresentMode();
		SelectSwapExtent();
	}

	void VulkanSwapchain::GetSupportDetails()
	{
		VkPhysicalDevice physicalDevice = VulkanContext::Get().GetCurrentPhysicalDevice();
		VkSurfaceKHR surface = VulkanContext::Get().GetSurface();

		VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &m_SupportDetails.SurfaceCapabilities));
		
		uint32 surfaceFormatsCount;
		VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatsCount, nullptr));
		m_SupportDetails.SurfaceFormats.resize(surfaceFormatsCount);
		VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatsCount, m_SupportDetails.SurfaceFormats.data()));
		ASSERT(m_SupportDetails.SurfaceFormats.size() > 0u);

		uint32 surfacePresentModesCount;
		VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModesCount, nullptr));
		m_SupportDetails.PresentModes.resize(surfacePresentModesCount);
		VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModesCount, m_SupportDetails.PresentModes.data()));
		ASSERT(m_SupportDetails.PresentModes.size() > 0u);

		LOG_INFO("Swapchain image count: {} to {}", m_SupportDetails.SurfaceCapabilities.minImageCount, m_SupportDetails.SurfaceCapabilities.maxImageCount);
	}

	void VulkanSwapchain::SelectSurfaceFormat()
	{
		m_SurfaceFormat.format = VK_FORMAT_UNDEFINED;
		m_SurfaceFormat.colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR;

		for (VkSurfaceFormatKHR& availableFormat : m_SupportDetails.SurfaceFormats)
		{
			// prefer the standard BGRA8 unorm
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				m_SurfaceFormat = availableFormat;
				break;
			}
		}
		if (m_SurfaceFormat.format == VK_FORMAT_UNDEFINED)
			m_SurfaceFormat = m_SupportDetails.SurfaceFormats[0];

		LOG_INFO("Found {} swapchain surface formats", m_SupportDetails.SurfaceFormats.size());
		for (VkSurfaceFormatKHR& supportedFormat : m_SupportDetails.SurfaceFormats)
		{
			if ((m_SurfaceFormat.format == supportedFormat.format) && (m_SurfaceFormat.colorSpace == supportedFormat.colorSpace))
				LOG_WARN("  format: {} colorspace: {}", string_VkFormat(supportedFormat.format), string_VkColorSpaceKHR(supportedFormat.colorSpace));
			else
				LOG_TRACE("  format: {} colorspace: {}", string_VkFormat(supportedFormat.format), string_VkColorSpaceKHR(supportedFormat.colorSpace));
		}
	}

	void VulkanSwapchain::SelectPresentMode()
	{
		m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;

		if (!m_VerticalSync)
		{
			// 1) mailbox or 2) immediate, otherwise stick with FIFO (vSync)
			for (VkPresentModeKHR& supportedMode : m_SupportDetails.PresentModes)
			{
				if (supportedMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					m_PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if (m_PresentMode != VK_PRESENT_MODE_MAILBOX_KHR && supportedMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
					m_PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		LOG_INFO("Found {} swapchain present modes", m_SupportDetails.PresentModes.size());
		for (VkPresentModeKHR& supportedMode : m_SupportDetails.PresentModes)
		{
			if (m_PresentMode == supportedMode)
				LOG_WARN("  mode: {}", string_VkPresentModeKHR(supportedMode));
			else
				LOG_TRACE("  mode: {}", string_VkPresentModeKHR(supportedMode));
		}
	}

	void VulkanSwapchain::SelectSwapExtent()
	{
		int32 minWidth  = (int32)m_SupportDetails.SurfaceCapabilities.minImageExtent.width;
		int32 minHeight = (int32)m_SupportDetails.SurfaceCapabilities.minImageExtent.height;
		int32 maxWidth  = (int32)m_SupportDetails.SurfaceCapabilities.maxImageExtent.width;
		int32 maxHeight = (int32)m_SupportDetails.SurfaceCapabilities.maxImageExtent.height;

		uint32 currentWidth = m_SupportDetails.SurfaceCapabilities.currentExtent.width;
		uint32 currentHeight = m_SupportDetails.SurfaceCapabilities.currentExtent.height;

		if ((currentWidth != UINT32_MAX) && (currentHeight != UINT32_MAX))
		{
			m_Width = (int32)currentWidth;
			m_Height = (int32)currentHeight;
		}
		else
		{
			GLFWwindow* glfwWindow = (GLFWwindow*)VulkanContext::Get().GetWindow()->GetPlatformWindow();
			glfwGetFramebufferSize(glfwWindow, &m_Width, &m_Height);

			// clamp to the supported range
			if (m_Width < minWidth) m_Width = minWidth;
			if (m_Width > maxWidth) m_Width = maxWidth;
			if (m_Height < minHeight) m_Height = minHeight;
			if (m_Height > maxHeight) m_Height = maxHeight;

			LOG_TRACE("Manually setting the swapchain's extent to ({}, {})", m_Width, m_Height);
		}

		ASSERT(minWidth > 0u && minHeight > 0u && maxWidth > 0u && maxHeight > 0u);
		ASSERT(m_Width >= minWidth && m_Height >= minHeight && m_Width <= maxWidth && m_Height <= maxHeight);
	}

	void VulkanSwapchain::Shutdown()
	{
	}

	void VulkanSwapchain::Resize(int32 width, int32 height)
	{
	}

	void VulkanSwapchain::Present()
	{
	}
}
