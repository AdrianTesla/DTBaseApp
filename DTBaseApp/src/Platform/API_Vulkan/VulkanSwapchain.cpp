#include "VulkanSwapchain.h"
#include <GLFW/glfw3.h>
#include "VulkanContext.h"
#include "Renderer/Renderer.h"

namespace DT
{
	void VulkanSwapchain::Init(bool verticalSyncEnabled)
	{
		VulkanPhysicalDevice& physicalDevice = VulkanContext::GetCurrentPhysicalDevice();

		m_VerticalSync = verticalSyncEnabled;
		m_OldVerticalSync = m_VerticalSync;
		m_PresentQueue = VulkanContext::GetCurrentDevice().GetQueue(QueueType::Present);
		m_MSAACount = physicalDevice.GetFramebufferMultisampleCount();

		CreateSyncronizationObjects();
		Invalidate();
	}

	void VulkanSwapchain::Shutdown()
	{
		DestroyDynamicResources();

		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
			m_ImageAvailableSemaphores[i] = VK_NULL_HANDLE;
		}
		vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
		m_Swapchain = VK_NULL_HANDLE;
	}

	void VulkanSwapchain::DestroyDynamicResources()
	{		
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VK_CALL(vkDeviceWaitIdle(device));

		for (uint32 i = 0u; i < m_ImageCount; i++) {
			vkDestroyFramebuffer(device, m_SwapchainFramebuffers[i], nullptr);
			vkDestroyImageView(device, m_SwapchainImageViews[i], nullptr);
			m_SwapchainImageViews[i] = VK_NULL_HANDLE;
			m_SwapchainFramebuffers[i] = VK_NULL_HANDLE;
		}

		// temp
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();
		vmaDestroyImage(allocator, m_DepthImage, m_DepthImageAllocation);
		vkDestroyImageView(device, m_DepthImageView, nullptr);
		vmaDestroyImage(allocator, m_MSAAColorImage, m_MSAAColorImageAllocation);
		vkDestroyImageView(device, m_MSAAColorImageView, nullptr);
		vkDestroyRenderPass(device, m_SwapchainRenderPass, nullptr);
		m_DepthImage               = VK_NULL_HANDLE;
		m_DepthImageAllocation     = VK_NULL_HANDLE;
		m_DepthImageView           = VK_NULL_HANDLE;
		m_MSAAColorImage           = VK_NULL_HANDLE;
		m_MSAAColorImageAllocation = VK_NULL_HANDLE;
		m_MSAAColorImageView       = VK_NULL_HANDLE;
		m_SwapchainRenderPass      = VK_NULL_HANDLE;
	}

	VkSemaphore& VulkanSwapchain::GetImageAvailableSemaphore(uint32 frameIndex) 
	{ 
		return m_ImageAvailableSemaphores[frameIndex];
	}

	void VulkanSwapchain::Invalidate()
	{
		m_SurfaceResized = false;
		m_OldVerticalSync = m_VerticalSync;

		Timer timer;

		DestroyDynamicResources();

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
		CreateMultisampledImages();
		CreateDepthImage();
		CreateSwapchainRenderPass();
		CreateSwapchainFramebuffers();

		LOG_WARN("Created swapchain with size ({}, {}) ({} ms)", m_Width, m_Height, timer.ElapsedMilliseconds());
		m_LogCreation = false;
	}


	void VulkanSwapchain::GetSupportDetails()
	{
		VkPhysicalDevice physicalDevice = VulkanContext::GetCurrentVulkanPhysicalDevice();
		VkSurfaceKHR surface = VulkanContext::GetSurface();

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

		if (m_LogCreation) {
			LOG_INFO("Swapchain image count range: {} to {}", m_SupportDetails.SurfaceCapabilities.minImageCount, m_SupportDetails.SurfaceCapabilities.maxImageCount);
		}
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

		if (m_LogCreation)
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
				if ((m_PresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (supportedMode == VK_PRESENT_MODE_IMMEDIATE_KHR))
					m_PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}

		if (m_LogCreation)
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

	void VulkanSwapchain::SelectSwapExtent()
	{
		VkSurfaceCapabilitiesKHR& surfaceCaps = m_SupportDetails.SurfaceCapabilities;
		int32 minWidth  = (int32)surfaceCaps.minImageExtent.width;
		int32 minHeight = (int32)surfaceCaps.minImageExtent.height;
		int32 maxWidth  = (int32)surfaceCaps.maxImageExtent.width;
		int32 maxHeight = (int32)surfaceCaps.maxImageExtent.height;

		uint32 currentWidth = surfaceCaps.currentExtent.width;
		uint32 currentHeight = surfaceCaps.currentExtent.height;

		if ((currentWidth != UINT32_MAX) && (currentHeight != UINT32_MAX))
		{
			m_Width = (int32)currentWidth;
			m_Height = (int32)currentHeight;

			if (m_LogCreation) {
				LOG_TRACE("Requested size: ({}, {})", m_Width, m_Height);
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

			if (m_LogCreation) {
				LOG_TRACE("Manually setting the swapchain's extent to ({}, {})", m_Width, m_Height);
			}
		}

		ASSERT(minWidth > 0u && minHeight > 0u && maxWidth > 0u && maxHeight > 0u);
		ASSERT(m_Width >= minWidth && m_Height >= minHeight && m_Width <= maxWidth && m_Height <= maxHeight);
	}

	void VulkanSwapchain::SelectImageCount()
	{		
		uint32 minImageCount = m_SupportDetails.SurfaceCapabilities.minImageCount;
		uint32 maxImageCount = m_SupportDetails.SurfaceCapabilities.maxImageCount;

		if (maxImageCount == 0u)
			m_ImageCount = 3u;
		else
		{
			m_ImageCount = minImageCount + 1u;
			if (m_ImageCount > maxImageCount)
				m_ImageCount = maxImageCount;
		}

		if (m_LogCreation) {
			LOG_TRACE("Requested {} swapchain images", m_ImageCount);
		}
	}

	void VulkanSwapchain::SelectImageUsage()
	{
		VkImageUsageFlags supportedUsageFlags = m_SupportDetails.SurfaceCapabilities.supportedUsageFlags;
		
		m_SwapImageUsage = 0u;
		if (supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			m_SwapImageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if (m_LogCreation)
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

	void VulkanSwapchain::SelectCompositeAlpha()
	{
		VkCompositeAlphaFlagsKHR supportedFlags = m_SupportDetails.SurfaceCapabilities.supportedCompositeAlpha;

		m_CompositeAlpha = VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR;
		if (supportedFlags & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
			m_CompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		if (m_LogCreation)
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

	void VulkanSwapchain::SelectSurfaceTransform()
	{
		VkSurfaceTransformFlagsKHR supportedTransforms = m_SupportDetails.SurfaceCapabilities.supportedTransforms;

		m_SurfaceTransform = m_SupportDetails.SurfaceCapabilities.currentTransform;

		if (m_LogCreation)
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

		VkSwapchainKHR oldSwapchain = m_Swapchain;
		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.pNext                 = nullptr;
		swapchainCreateInfo.flags                 = 0u;
		swapchainCreateInfo.surface               = VulkanContext::GetSurface();
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

		VulkanDevice& vulkanDevice = VulkanContext::GetCurrentDevice();
		VkCommandBuffer commandBuffer = vulkanDevice.BeginCommandBuffer(QueueType::Graphics);
		for (uint32 i = 0u; i < m_ImageCount; i++) {
			vkCmd::TransitionImageLayoutSingleMip(
				commandBuffer, 
				m_SwapchainImages[i], 
				VK_IMAGE_LAYOUT_UNDEFINED, 
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 
				m_SurfaceFormat.format, 
				0u
			);
		}
		vulkanDevice.EndCommandBuffer();

		if (m_LogCreation) {
			LOG_INFO("Created swapchain with {} images, each ({}, {})", m_ImageCount, m_Width, m_Height);
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
			VkImageView imageViews[] = {
				m_MSAAColorImageView,
				m_DepthImageView,
				m_SwapchainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferCreateInfo{};
			framebufferCreateInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCreateInfo.pNext           = nullptr;
			framebufferCreateInfo.flags           = 0u;
			framebufferCreateInfo.renderPass      = m_SwapchainRenderPass;
			framebufferCreateInfo.attachmentCount = (uint32)std::size(imageViews);
			framebufferCreateInfo.pAttachments    = imageViews;
			framebufferCreateInfo.width           = (uint32)m_Width;
			framebufferCreateInfo.height          = (uint32)m_Height;
			framebufferCreateInfo.layers          = 1u;
			VK_CALL(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &m_SwapchainFramebuffers[i]));
		}
	}

	void VulkanSwapchain::CreateSwapchainRenderPass()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		std::array<VkAttachmentDescription, 3u> attachmentDescriptions{};

		// multisampled color attachment
		attachmentDescriptions[0].flags          = 0u;
		attachmentDescriptions[0].format         = m_SurfaceFormat.format;
		attachmentDescriptions[0].samples        = m_MSAACount;
		attachmentDescriptions[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[0].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// depth attachment
		attachmentDescriptions[1].flags          = 0u;
		attachmentDescriptions[1].format         = m_DepthFormat;
		attachmentDescriptions[1].samples        = m_MSAACount;
		attachmentDescriptions[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescriptions[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// color resolve target
		attachmentDescriptions[2].flags          = 0u;
		attachmentDescriptions[2].format         = m_SurfaceFormat.format;
		attachmentDescriptions[2].samples        = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescriptions[2].loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[2].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescriptions[2].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescriptions[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescriptions[2].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDescriptions[2].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0u;
		colorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentReference{};
		depthAttachmentReference.attachment = 1u;
		depthAttachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference resolveColorAttachmentReference{};
		resolveColorAttachmentReference.attachment = 2u;
		resolveColorAttachmentReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription{};
		subpassDescription.flags                   = 0u;
		subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.inputAttachmentCount    = 0u;
		subpassDescription.pInputAttachments       = nullptr;
		subpassDescription.colorAttachmentCount    = 1u;
		subpassDescription.pColorAttachments       = &colorAttachmentReference;
		subpassDescription.pResolveAttachments     = &resolveColorAttachmentReference;
		subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
		subpassDescription.preserveAttachmentCount = 0u;
		subpassDescription.pPreserveAttachments    = nullptr;

		VkSubpassDependency subpassDependency{};
		subpassDependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
		subpassDependency.dstSubpass      = 0u;
		subpassDependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;	 
		subpassDependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;	 
		subpassDependency.srcAccessMask   = 0u;																							 
		subpassDependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;		 
		subpassDependency.dependencyFlags = 0u;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.pNext           = nullptr;
		renderPassCreateInfo.flags           = 0u;
		renderPassCreateInfo.attachmentCount = (uint32)attachmentDescriptions.size();
		renderPassCreateInfo.pAttachments    = attachmentDescriptions.data();
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

	void VulkanSwapchain::CreateMultisampledImages()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType				  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext				  = nullptr;
		imageCreateInfo.flags				  = 0u;
		imageCreateInfo.imageType			  = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format				  = m_SurfaceFormat.format;
		imageCreateInfo.extent.width		  = (uint32)m_Width;
		imageCreateInfo.extent.height		  = (uint32)m_Height;
		imageCreateInfo.extent.depth		  = 1u;
		imageCreateInfo.mipLevels			  = 1u;
		imageCreateInfo.arrayLayers			  = 1u;
		imageCreateInfo.samples				  = m_MSAACount;
		imageCreateInfo.tiling				  = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage				  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageCreateInfo.sharingMode			  = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0u;
		imageCreateInfo.pQueueFamilyIndices	  = nullptr;
		imageCreateInfo.initialLayout		  = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		allocationCreateInfo.flags = 0u;
		VK_CALL(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &m_MSAAColorImage, &m_MSAAColorImageAllocation, nullptr));	

		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType		 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext		 = nullptr;
		imageViewCreateInfo.flags		 = 0u;
		imageViewCreateInfo.image		 = m_MSAAColorImage;
		imageViewCreateInfo.viewType	 = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format		 = imageCreateInfo.format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel	= 0u;
		imageViewCreateInfo.subresourceRange.levelCount		= 1u;
		imageViewCreateInfo.subresourceRange.baseArrayLayer	= 0u;
		imageViewCreateInfo.subresourceRange.layerCount		= 1u;
		VK_CALL(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_MSAAColorImageView));
	}

	void VulkanSwapchain::CreateDepthImage()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();
		VmaAllocator allocator = VulkanContext::GetVulkanMemoryAllocator();

		VulkanPhysicalDevice& physicalDevice = VulkanContext::GetCurrentPhysicalDevice();
		m_DepthFormat = physicalDevice.GetBestDepthOnlyFormat();

		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType				  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext				  = nullptr;
		imageCreateInfo.flags				  = 0u;
		imageCreateInfo.imageType			  = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format				  = m_DepthFormat;
		imageCreateInfo.extent.width		  = (uint32)m_Width;
		imageCreateInfo.extent.height		  = (uint32)m_Height;
		imageCreateInfo.extent.depth		  = 1u;
		imageCreateInfo.mipLevels			  = 1u;
		imageCreateInfo.arrayLayers			  = 1u;
		imageCreateInfo.samples				  = m_MSAACount;
		imageCreateInfo.tiling				  = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage				  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageCreateInfo.sharingMode			  = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0u;
		imageCreateInfo.pQueueFamilyIndices	  = nullptr;
		imageCreateInfo.initialLayout		  = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		allocationCreateInfo.flags = 0u;
		VK_CALL(vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &m_DepthImage, &m_DepthImageAllocation, nullptr));	
	
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType		 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext		 = nullptr;
		imageViewCreateInfo.flags		 = 0u;
		imageViewCreateInfo.image		 = m_DepthImage;
		imageViewCreateInfo.viewType	 = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format		 = m_DepthFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_DEPTH_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel	= 0u;
		imageViewCreateInfo.subresourceRange.levelCount		= 1u;
		imageViewCreateInfo.subresourceRange.baseArrayLayer	= 0u;
		imageViewCreateInfo.subresourceRange.layerCount		= 1u;
		VK_CALL(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_DepthImageView));
	}

	bool VulkanSwapchain::AquireNextImage()
	{
		VkDevice device = VulkanContext::GetCurrentVulkanDevice();

		VkResult aquireResult = vkAcquireNextImageKHR(
			device,
			m_Swapchain, 
			UINT64_MAX, 
			m_ImageAvailableSemaphores[Renderer::CurrentFrame()],
			VK_NULL_HANDLE, 
			&m_CurrentImageIndex
		);

		if ((aquireResult == VK_SUCCESS) || (aquireResult == VK_SUBOPTIMAL_KHR))
			return true;

		if (aquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
			Invalidate();
			return false;
		}
		VK_CALL(aquireResult);
		return false;
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

		VkResult presentResult = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
		if ((presentResult != VK_SUCCESS) || (m_VerticalSync != m_OldVerticalSync))
		{
			if (m_SurfaceResized || (presentResult == VK_ERROR_OUT_OF_DATE_KHR) || (presentResult == VK_SUBOPTIMAL_KHR) || (m_VerticalSync != m_OldVerticalSync)) {
				Invalidate(); 
			} else {
				VK_CALL(presentResult);
			}
		}
	}

	void VulkanSwapchain::SetVerticalSync(bool enabled)
	{
		m_OldVerticalSync = m_VerticalSync;
		m_VerticalSync = enabled;
	}

	void VulkanSwapchain::OnWindowResize()
	{
		m_SurfaceResized = true;
	}
}