#include "VulkanSwapchain.h"
#include <GLFW/glfw3.h>
#include "VulkanContext.h"

namespace DT
{
	void VulkanSwapchain::Init(bool verticalSync)
	{
		m_VerticalSync = verticalSync;
		m_PresentQueue = VulkanContext::GetCurrentDevice().GetPresentQueue();

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
		CreateSwapchainRenderPass();
		CreateSwapchainFramebuffers();
		CreateSyncronizationObjects();
	}

	VkSemaphore& VulkanSwapchain::GetImageAvailableSemaphore() 
	{ 
		return m_ImageAvailableSemaphores[VulkanContext::GetCurrentFrame()]; 
	}

	void VulkanSwapchain::RecreateSwapchain()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VK_CALL(vkDeviceWaitIdle(device));

		for (uint32 i = 0u; i < m_ImageCount; i++)
			vkDestroyFramebuffer(device, m_SwapchainFramebuffers[i], nullptr);

		for (uint32 i = 0u; i < m_ImageCount; i++)
			vkDestroyImageView(device, m_SwapchainImageViews[i], nullptr);

		GetSupportDetails(false);
		SelectSurfaceFormat(false);
		SelectPresentMode(false);
		SelectSwapExtent(false);
		SelectImageCount(false);
		SelectImageUsage(false);
		SelectCompositeAlpha(false);
		SelectSurfaceTransform(false);
		CreateSwapchain(false);
		CreateSwapchainImageViews();
		CreateSwapchainFramebuffers();

		LOG_WARN("Recreated swapchain: ({}, {})", m_Width, m_Height);
	}

	void VulkanSwapchain::GetSupportDetails(bool log)
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

		if (log) {
			LOG_INFO("Swapchain image count range: {} to {}", m_SupportDetails.SurfaceCapabilities.minImageCount, m_SupportDetails.SurfaceCapabilities.maxImageCount);
		}
	}

	void VulkanSwapchain::SelectSurfaceFormat(bool log)
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

		if (log)
		{
			LOG_INFO("Found {} swapchain surface formats", m_SupportDetails.SurfaceFormats.size());
			for (VkSurfaceFormatKHR& supportedFormat : m_SupportDetails.SurfaceFormats)
			{
				if ((m_SurfaceFormat.format == supportedFormat.format) && (m_SurfaceFormat.colorSpace == supportedFormat.colorSpace))
					LOG_WARN("  format: {} colorspace: {}", string_VkFormat(supportedFormat.format), string_VkColorSpaceKHR(supportedFormat.colorSpace));
				else
					LOG_TRACE("  format: {} colorspace: {}", string_VkFormat(supportedFormat.format), string_VkColorSpaceKHR(supportedFormat.colorSpace));
			}
		}
	}

	void VulkanSwapchain::SelectPresentMode(bool log)
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

		if (log)
		{
			LOG_INFO("Found {} swapchain present modes", m_SupportDetails.PresentModes.size());
			for (VkPresentModeKHR& supportedMode : m_SupportDetails.PresentModes)
			{
				if (m_PresentMode == supportedMode)
					LOG_WARN("  mode: {}", string_VkPresentModeKHR(supportedMode));
				else
					LOG_TRACE("  mode: {}", string_VkPresentModeKHR(supportedMode));
			}
		}
	}

	void VulkanSwapchain::SelectSwapExtent(bool log)
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

			if (log) {
				LOG_TRACE("Swapchain size ({}, {})", m_Width, m_Height);
			}
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

			if (log) {
				LOG_TRACE("Manually setting the swapchain's extent to ({}, {})", m_Width, m_Height);
			}
		}

		ASSERT(minWidth > 0u && minHeight > 0u && maxWidth > 0u && maxHeight > 0u);
		ASSERT(m_Width >= minWidth && m_Height >= minHeight && m_Width <= maxWidth && m_Height <= maxHeight);
	}

	void VulkanSwapchain::SelectImageCount(bool log)
	{		
		uint32 minImageCount = m_SupportDetails.SurfaceCapabilities.minImageCount;
		uint32 maxImageCount = m_SupportDetails.SurfaceCapabilities.maxImageCount;

		m_ImageCount = minImageCount + 1u;
		if (maxImageCount != 0 && m_ImageCount > maxImageCount)
			m_ImageCount = maxImageCount;
	}

	void VulkanSwapchain::SelectImageUsage(bool log)
	{
		VkImageUsageFlags supportedUsageFlags = m_SupportDetails.SurfaceCapabilities.supportedUsageFlags;
		
		m_SwapImageUsage = 0u;
		if (supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			m_SwapImageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (log)
		{
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
	}

	void VulkanSwapchain::SelectCompositeAlpha(bool log)
	{
		VkCompositeAlphaFlagsKHR supportedFlags = m_SupportDetails.SurfaceCapabilities.supportedCompositeAlpha;

		m_CompositeAlpha = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
		if (supportedFlags & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
			m_CompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		if (log)
		{
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
	}

	void VulkanSwapchain::SelectSurfaceTransform(bool log)
	{
		VkSurfaceTransformFlagsKHR supportedTransforms = m_SupportDetails.SurfaceCapabilities.supportedTransforms;

		m_SurfaceTransform = m_SupportDetails.SurfaceCapabilities.currentTransform;

		if (log)
		{
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
	}

	void VulkanSwapchain::CreateSwapchain(bool log)
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

		VkSwapchainKHR oldSwapchain = m_Swapchain;
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
		swapchainCreateInfo.preTransform          = m_SurfaceTransform;
		swapchainCreateInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode           = m_PresentMode;
		swapchainCreateInfo.clipped               = VK_TRUE;
		swapchainCreateInfo.oldSwapchain          = m_Swapchain;
		VK_CALL(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &m_Swapchain));

		if (oldSwapchain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(device, oldSwapchain, nullptr);

		// retrieve the automatically created swapchain images
		VK_CALL(vkGetSwapchainImagesKHR(device, m_Swapchain, &m_ImageCount, m_SwapchainImages.Data));
		m_SwapchainImages.Size = m_ImageCount;

		if (log) {
			LOG_INFO("Created swapchain with {} images", m_ImageCount);
		}
	}

	void VulkanSwapchain::CreateSwapchainImageViews()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		m_SwapchainImageViews.Size = m_ImageCount;
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

	void VulkanSwapchain::CreateSwapchainFramebuffers()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		m_SwapchainFramebuffers.Size = m_ImageCount;
		for (uint32 i = 0u; i < m_ImageCount; i++)
		{
			VkFramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.pNext           = nullptr;
			framebufferCreateInfo.flags           = 0u;
			framebufferCreateInfo.renderPass      = m_SwapchainRenderPass;
			framebufferCreateInfo.attachmentCount = 1u;
			framebufferCreateInfo.pAttachments    = &m_SwapchainImageViews[i];
			framebufferCreateInfo.width           = (uint32)m_Width;
			framebufferCreateInfo.height          = (uint32)m_Height;
			framebufferCreateInfo.layers          = 1u;
			VK_CALL(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &m_SwapchainFramebuffers[i]));
		}
	}

	void VulkanSwapchain::CreateSwapchainRenderPass()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.flags          = 0u;
		attachmentDescription.format         = m_SurfaceFormat.format;
		attachmentDescription.samples        = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescription.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescription.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0u;
		colorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription{};
		subpassDescription.flags                   = 0u;
		subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.inputAttachmentCount    = 0u;
		subpassDescription.pInputAttachments       = nullptr;
		subpassDescription.colorAttachmentCount    = 1u;
		subpassDescription.pColorAttachments       = &colorAttachmentReference;
		subpassDescription.pResolveAttachments     = nullptr;
		subpassDescription.pDepthStencilAttachment = nullptr;
		subpassDescription.preserveAttachmentCount = 0u;
		subpassDescription.pPreserveAttachments    = nullptr;

		VkSubpassDependency subpassDependency{};
		subpassDependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
		subpassDependency.dstSubpass      = 0u;
		subpassDependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDependency.srcAccessMask   = 0u;
		subpassDependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpassDependency.dependencyFlags = 0u;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.pNext           = nullptr;
		renderPassCreateInfo.flags           = 0u;
		renderPassCreateInfo.attachmentCount = 1u;
		renderPassCreateInfo.pAttachments    = &attachmentDescription;
		renderPassCreateInfo.subpassCount    = 1u;
		renderPassCreateInfo.pSubpasses      = &subpassDescription;
		renderPassCreateInfo.dependencyCount = 1u;	
		renderPassCreateInfo.pDependencies   = &subpassDependency;
		VK_CALL(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &m_SwapchainRenderPass));
	}

	void VulkanSwapchain::CreateSyncronizationObjects()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = 0u;
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			VK_CALL(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_ImageAvailableSemaphores[i]));
	}

	void VulkanSwapchain::AquireNextImage()
	{
		VK_CALL(vkAcquireNextImageKHR(
			VulkanContext::GetCurrentVulkanDevice(),
			m_Swapchain, 
			UINT64_MAX, 
			m_ImageAvailableSemaphores[VulkanContext::GetCurrentFrame()],
			VK_NULL_HANDLE, 
			&m_CurrentImageIndex
		));
	}

	void VulkanSwapchain::Present(VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext              = nullptr;
		presentInfo.waitSemaphoreCount = 1u;
		presentInfo.pWaitSemaphores    = &waitSemaphore;
		presentInfo.swapchainCount     = 1u;
		presentInfo.pSwapchains        = &m_Swapchain;
		presentInfo.pImageIndices      = &m_CurrentImageIndex;
		presentInfo.pResults           = nullptr;
		VkResult result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
		if (result != VK_SUCCESS)
		{
			RecreateSwapchain();
		}
	}

	void VulkanSwapchain::Shutdown()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
		
		for (uint32 i = 0u; i < m_ImageCount; i++)
		{
			vkDestroyImageView(device, m_SwapchainImageViews[i], nullptr);
			vkDestroyFramebuffer(device, m_SwapchainFramebuffers[i], nullptr);
		}

		vkDestroyRenderPass(device, m_SwapchainRenderPass, nullptr);

		vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
		
		m_Swapchain = VK_NULL_HANDLE;
		m_SwapchainRenderPass = VK_NULL_HANDLE;
	}
}