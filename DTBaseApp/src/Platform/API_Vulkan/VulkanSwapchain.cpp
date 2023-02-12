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

		uint32 surfacePresentModesCount;
		VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModesCount, nullptr));
		m_SupportDetails.PresentModes.resize(surfacePresentModesCount);
		VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModesCount, m_SupportDetails.PresentModes.data()));

		LOG_INFO("Swapchain image count: {} to {}", m_SupportDetails.SurfaceCapabilities.minImageCount, m_SupportDetails.SurfaceCapabilities.maxImageCount);
	}

	void VulkanSwapchain::SelectSurfaceFormat()
	{
		bool found = false;
		for (VkSurfaceFormatKHR& availableFormat : m_SupportDetails.SurfaceFormats)
		{
			// prefer the standard BGRA8 unorm
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				m_SurfaceFormat = availableFormat;
				found = true;
				break;
			}

		}
		ASSERT(found);

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
		uint32 minWidth  = m_SupportDetails.SurfaceCapabilities.minImageExtent.width;
		uint32 minHeight = m_SupportDetails.SurfaceCapabilities.minImageExtent.height;
		uint32 maxWidth  = m_SupportDetails.SurfaceCapabilities.maxImageExtent.width;
		uint32 maxHeight = m_SupportDetails.SurfaceCapabilities.maxImageExtent.height;
		//LOG_INFO("Swapchain image size range: {} to {}", m_SupportDetails.SurfaceCapabilities.minImageExtent.width);
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
