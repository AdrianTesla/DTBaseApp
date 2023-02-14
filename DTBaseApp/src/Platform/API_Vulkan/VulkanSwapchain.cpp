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
		SelectImageCount();
		SelectImageUsage();
		SelectCompositeAlpha();
		SelectSurfaceTransform();
		CreateSwapchain();
		CreateSwapchainImageViews();
	}

	void VulkanSwapchain::GetSupportDetails()
	{
		VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentVulkanPhysicalDevice();
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

		LOG_INFO("Swapchain image count range: {} to {}", m_SupportDetails.SurfaceCapabilities.minImageCount, m_SupportDetails.SurfaceCapabilities.maxImageCount);
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

	void VulkanSwapchain::SelectImageCount()
	{		
		uint32 minImageCount = m_SupportDetails.SurfaceCapabilities.minImageCount;
		uint32 maxImageCount = m_SupportDetails.SurfaceCapabilities.maxImageCount;

		m_ImageCount = minImageCount + 1u;
		if (maxImageCount != 0 && m_ImageCount > maxImageCount)
			m_ImageCount = maxImageCount;
	}

	void VulkanSwapchain::SelectImageUsage()
	{
		VkImageUsageFlags supportedUsageFlags = m_SupportDetails.SurfaceCapabilities.supportedUsageFlags;
		
		m_SwapImageUsage = 0u;
		if (supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			m_SwapImageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		LOG_INFO("Supported swapchain image usage flags:");
		uint32 index = 0u;
		while (supportedUsageFlags) 
		{
			VkImageUsageFlagBits currentFlag = (VkImageUsageFlagBits)(1u << index);
			if (supportedUsageFlags & 1u) 
			{
				if (m_SwapImageUsage & currentFlag)
					LOG_WARN("  {}", string_VkImageUsageFlagBits(currentFlag));
				else
					LOG_TRACE("  {}", string_VkImageUsageFlagBits(currentFlag));
			}
			index++;
			supportedUsageFlags >>= 1u;
		}
	}

	void VulkanSwapchain::SelectCompositeAlpha()
	{
		VkCompositeAlphaFlagsKHR supportedFlags = m_SupportDetails.SurfaceCapabilities.supportedCompositeAlpha;

		m_CompositeAlpha = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
		if (supportedFlags & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
			m_CompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		LOG_INFO("Supported swapchain composite alpha flags:");
		uint32 index = 0u;
		while (supportedFlags)
		{
			VkCompositeAlphaFlagBitsKHR currentFlag = (VkCompositeAlphaFlagBitsKHR)(1u << index);
			if (supportedFlags & 1u)
			{
				if (m_CompositeAlpha & currentFlag)
					LOG_WARN("  {}", string_VkCompositeAlphaFlagBitsKHR(currentFlag));
				else
					LOG_TRACE("  {}", string_VkCompositeAlphaFlagBitsKHR(currentFlag));
			}
			index++;
			supportedFlags >>= 1u;
		}
	}

	void VulkanSwapchain::SelectSurfaceTransform()
	{
		VkSurfaceTransformFlagsKHR supportedTransforms = m_SupportDetails.SurfaceCapabilities.supportedTransforms;

		m_SurfaceTransform = m_SupportDetails.SurfaceCapabilities.currentTransform;

		LOG_INFO("Supported swapchain surface transforms:");
		uint32 index = 0u;
		while (supportedTransforms)
		{
			VkSurfaceTransformFlagBitsKHR currentFlag = (VkSurfaceTransformFlagBitsKHR)(1u << index);
			if (supportedTransforms & 1u)
			{
				if (m_SurfaceTransform & currentFlag)
					LOG_WARN("  {}", string_VkSurfaceTransformFlagBitsKHR(currentFlag));
				else
					LOG_TRACE("  {}", string_VkSurfaceTransformFlagBitsKHR(currentFlag));
			}
			index++;
			supportedTransforms >>= 1u;
		}
	}

	void VulkanSwapchain::CreateSwapchain()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VulkanPhysicalDevice& physicalDevice = VulkanContext::GetCurrentPhysicalDevice();

		const QueueFamilyIndices& queueFamilyIndices = physicalDevice.GetQueueFamilyIndices();

		uint32 graphicsAndPresentIndices[] = {
			queueFamilyIndices.GraphicsIndex.value(),
			queueFamilyIndices.PresentIndex.value()
		};

		// if the present and graphics queues are different, we need to share the swapchain images across the queues
		VkSharingMode imageSharingMode;
		if (queueFamilyIndices.GraphicsIndex != queueFamilyIndices.PresentIndex)
			imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		else
			imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.pNext                 = nullptr;
		swapchainCreateInfo.flags                 = 0u;
		swapchainCreateInfo.surface               = VulkanContext::Get().GetSurface();
		swapchainCreateInfo.minImageCount         = m_ImageCount;
		swapchainCreateInfo.imageFormat           = m_SurfaceFormat.format;
		swapchainCreateInfo.imageColorSpace       = m_SurfaceFormat.colorSpace;
		swapchainCreateInfo.imageExtent.width     = (uint32)m_Width;
		swapchainCreateInfo.imageExtent.height    = (uint32)m_Height;
		swapchainCreateInfo.imageArrayLayers      = 1u;
		swapchainCreateInfo.imageUsage            = m_SwapImageUsage;
		swapchainCreateInfo.imageSharingMode      = imageSharingMode;
		swapchainCreateInfo.queueFamilyIndexCount = ((imageSharingMode == VK_SHARING_MODE_CONCURRENT) ? 2u : 0u);
		swapchainCreateInfo.pQueueFamilyIndices   = graphicsAndPresentIndices;
		swapchainCreateInfo.preTransform          = m_SupportDetails.SurfaceCapabilities.currentTransform;
		swapchainCreateInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode           = m_PresentMode;
		swapchainCreateInfo.clipped               = VK_TRUE;
		swapchainCreateInfo.oldSwapchain          = m_Swapchain;
		VK_CALL(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &m_Swapchain));

		// retrieve the automatically created swapchain images
		VK_CALL(vkGetSwapchainImagesKHR(device, m_Swapchain, &m_ImageCount, nullptr));
		m_SwapchainImages.resize(m_ImageCount);
		VK_CALL(vkGetSwapchainImagesKHR(device, m_Swapchain, &m_ImageCount, m_SwapchainImages.data()));

		LOG_INFO("Created swapchain with {} images", m_ImageCount);
	}

	void VulkanSwapchain::CreateSwapchainImageViews()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		m_SwapchainImageViews.resize(m_ImageCount);
		for (uint32 i = 0u; i < m_ImageCount; i++)
		{
			VkImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.pNext      = nullptr;
			imageViewCreateInfo.flags      = 0u;
			imageViewCreateInfo.image      = m_SwapchainImages[i];
			imageViewCreateInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format     = m_SurfaceFormat.format;
			imageViewCreateInfo.components = {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			};
			imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel   = 0u;
			imageViewCreateInfo.subresourceRange.levelCount     = 1u;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0u;
			imageViewCreateInfo.subresourceRange.layerCount     = 1u;
			VK_CALL(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_SwapchainImageViews[i]));
		}
	}

	void VulkanSwapchain::Shutdown()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		for (uint32 i = 0u; i < m_ImageCount; i++)
			vkDestroyImageView(device, m_SwapchainImageViews[i], nullptr);

		vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
		m_Swapchain = VK_NULL_HANDLE;
	}

	void VulkanSwapchain::Resize(int32 width, int32 height)
	{
	}

	void VulkanSwapchain::AquireNextImage(VkSemaphore imageAvailableSemaphore)
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VK_CALL(vkAcquireNextImageKHR(device, m_Swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &m_CurrentImageIndex));
	}

	void VulkanSwapchain::Present(VkSemaphore renderCompleteSemaphore)
	{
		VkQueue presentQueue = VulkanContext::GetCurrentDevice().GetPresentQueue();
		
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext              = nullptr;
		presentInfo.waitSemaphoreCount = 1u;
		presentInfo.pWaitSemaphores    = &renderCompleteSemaphore;
		presentInfo.swapchainCount     = 1u;
		presentInfo.pSwapchains        = &m_Swapchain;
		presentInfo.pImageIndices      = &m_CurrentImageIndex;
		presentInfo.pResults           = nullptr;
		VK_CALL(vkQueuePresentKHR(presentQueue, &presentInfo));
	}
}