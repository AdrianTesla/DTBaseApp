#include "VulkanContext.h"
#include <GLFW/glfw3.h>
#include "Platform/PlatformUtils.h"

namespace DT
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
	{
		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				LOG_TRACE(pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				LOG_INFO(pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				LOG_WARN(pCallbackData->pMessage);
				MessageBoxes::ShowWarning(pCallbackData->pMessage, "Vulkan Validation Warning!");
				__debugbreak();
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				LOG_ERROR(pCallbackData->pMessage);
				MessageBoxes::ShowError(pCallbackData->pMessage, "Vulkan Validation Error!");
				__debugbreak();
				break;
			default: ASSERT(false);
		}
		return VK_FALSE;
	}

	std::vector<const char*> VulkanContext::BuildRequestedInstanceExtensions()
	{
		std::vector<const char*> extensions;

		uint32 glfwExtensionCount;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// required by GLFW
		for (uint32 i = 0u; i < glfwExtensionCount; i++)
			extensions.emplace_back(glfwExtensions[i]);

		// debug messenger
		if (s_ValidationLayerEnabled)
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		
		return extensions;
	}

	std::vector<const char*> VulkanContext::BuildRequestedInstanceLayers()
	{
		std::vector<const char*> instanceLayers;

		if (s_ValidationLayerEnabled)
			instanceLayers.emplace_back(VK_VALIDATION_LAYER_NAME);

		return instanceLayers;
	}

	bool VulkanContext::IsInstanceExtensionSupported(const char* extensionName) const
	{
		for (const VkExtensionProperties& extension : m_AvailableInstanceExtensions)
		{
			if (strcmp(extension.extensionName, extensionName) == 0)
				return true;
		}
		return false;
	}

	bool VulkanContext::IsInstanceLayerSupported(const char* layerName) const
	{
		for (const VkLayerProperties& layer : s_Context->m_AvailableInstanceLayers)
		{
			if (strcmp(layer.layerName, layerName) == 0)
				return true;
		}
		return false;
	}

	VulkanContext::VulkanContext(const Ref<Window>& window)
		: m_Window(window)
	{
		s_Context = this;
	}

	void VulkanContext::Init()
	{
		if (!glfwVulkanSupported())
			MessageBoxes::ShowError("Vulkan is not supported on the system!", "Error");

		CreateVulkanInstance();
		CreateWindowSurface();
		SelectPhysicalDevice();
		CreateLogicalDevice();
		CreateMemoryAllocator();
		CreateSwapchain();
		CreateGraphicsPipeline();
		CreateCommandBuffers();
		CreateSyncObjects();
	}

	void VulkanContext::CreateVulkanInstance()
	{
		// enumerate available instance extensions
		uint32 extensionCount;
		VK_CALL(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
		m_AvailableInstanceExtensions.resize(extensionCount);
		VK_CALL(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_AvailableInstanceExtensions.data()));
		
		// enumerate available instance layers
		uint32 layerCount;
		VK_CALL(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
		m_AvailableInstanceLayers.resize(layerCount);
		VK_CALL(vkEnumerateInstanceLayerProperties(&layerCount, m_AvailableInstanceLayers.data()));

		// requested instance extensions
		std::vector<const char*> requestedExtensions = BuildRequestedInstanceExtensions();
		for (const char* extension : requestedExtensions)
			ASSERT(IsInstanceExtensionSupported(extension));
		
		// requested instance layers
		std::vector<const char*> requestedLayers = BuildRequestedInstanceLayers();
		for (const char* layer : requestedLayers)
			ASSERT(IsInstanceLayerSupported(layer));
		
		VkApplicationInfo applicationInfo{};
		applicationInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext              = nullptr;
		applicationInfo.pApplicationName   = nullptr;
		applicationInfo.applicationVersion = 0u;
		applicationInfo.pEngineName        = nullptr;
		applicationInfo.engineVersion      = 0u;
		applicationInfo.apiVersion         = VK_API_VERSION_1_3;
		
		VkInstanceCreateInfo instanceCreateInfo{};
		instanceCreateInfo.pNext                   = nullptr;
		instanceCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.flags                   = 0u;
		instanceCreateInfo.pApplicationInfo        = &applicationInfo;
		instanceCreateInfo.enabledLayerCount       = (uint32)requestedLayers.size();
		instanceCreateInfo.ppEnabledLayerNames     = requestedLayers.data();
		instanceCreateInfo.enabledExtensionCount   = (uint32)requestedExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = requestedExtensions.data();
		
		// inject a debug messenger for the instance creation
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
		if (s_ValidationLayerEnabled)
		{
			debugUtilsMessengerCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsMessengerCreateInfo.pNext           = nullptr;
			debugUtilsMessengerCreateInfo.flags           = 0u;
			debugUtilsMessengerCreateInfo.messageSeverity =
			//	VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT	|
			//	VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT	|
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT	|
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugUtilsMessengerCreateInfo.messageType     =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     | 
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  | 
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugUtilsMessengerCreateInfo.pfnUserCallback = VulkanMessageCallback;
			debugUtilsMessengerCreateInfo.pUserData       = nullptr;
			instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
		}
		
		VK_CALL(vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance));

		// create the actual debug messenger
		if (s_ValidationLayerEnabled)
			VK_CALL(GET_INSTANCE_FUNC(vkCreateDebugUtilsMessengerEXT)(m_Instance, &debugUtilsMessengerCreateInfo, nullptr, &m_DebugMessenger));
	}

	void VulkanContext::CreateWindowSurface()
	{
		GLFWwindow* glfwWindow = (GLFWwindow*)m_Window->GetPlatformWindow();
		VK_CALL(glfwCreateWindowSurface(m_Instance, glfwWindow, nullptr, &m_Surface));
	}

	void VulkanContext::SelectPhysicalDevice()
	{
		uint32 physicalDeviceCount;
		VK_CALL(vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr));
		m_AvailablePhysicalDevices.resize(physicalDeviceCount);
		VK_CALL(vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, m_AvailablePhysicalDevices.data()));

		if (physicalDeviceCount == 0u)
			MessageBoxes::ShowError("Error", "No GPU's found on the system!");

		// prefer dedicated GPU's
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		for (uint32 i = 0u; i < physicalDeviceCount; i++)
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(m_AvailablePhysicalDevices[i], &physicalDeviceProperties);
			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				physicalDevice = m_AvailablePhysicalDevices[i];
				break;
			}
		}

		// if no dedicated GPU found, use the first enumerated one 
		if (physicalDevice == VK_NULL_HANDLE)
			physicalDevice = m_AvailablePhysicalDevices[0];

		// GPU selection override for debug
	    //physicalDevice = m_AvailablePhysicalDevices[1];

		LOG_INFO("Found {} physical device(s):", physicalDeviceCount);
		for (uint32 i = 0u; i < physicalDeviceCount; i++)
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(m_AvailablePhysicalDevices[i], &physicalDeviceProperties);

			if (physicalDevice == m_AvailablePhysicalDevices[i]) {
				LOG_WARN("  name: {}", physicalDeviceProperties.deviceName);
				LOG_WARN("  type: {}", string_VkPhysicalDeviceType(physicalDeviceProperties.deviceType));
			} else {
				LOG_TRACE("  name: {}", physicalDeviceProperties.deviceName);
				LOG_TRACE("  type: {}", string_VkPhysicalDeviceType(physicalDeviceProperties.deviceType));
			}
		}

		m_PhysicalDevice.Init(physicalDevice);
	}
	
	void VulkanContext::CreateLogicalDevice()
	{
		m_Device.Init();
	}

	void VulkanContext::CreateSwapchain()
	{
		m_Swapchain.Init(false);
	}

	void VulkanContext::CreateSyncObjects()
	{
		VkDevice device = m_Device.GetVulkanDevice();

		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			vkCreateFence(device, &fenceCreateInfo, nullptr, &m_PreviousFrameFinishedFences[i]);

		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.pNext = nullptr;
		semaphoreCreateInfo.flags = 0u;
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			VK_CALL(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_RenderCompleteSemaphores[i]));
	}

	void VulkanContext::CreateGraphicsPipeline()
	{
		m_Shader = Ref<VulkanShader>::Create();
		{
			PipelineSpecification specification;
			specification.Shader = m_Shader;
			specification.PolygonMode = PolygonMode::Fill;
			m_PipelineFill = Ref<VulkanPipeline>::Create(specification);
		}
		{
			PipelineSpecification specification;
			specification.Shader = m_Shader;
			specification.PolygonMode = PolygonMode::Wireframe;
			m_PipelineWireframe = Ref<VulkanPipeline>::Create(specification);
		}
	}

	void VulkanContext::CreateCommandBuffers()
	{
		VkDevice device = m_Device.GetVulkanDevice();

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext              = nullptr;
		commandBufferAllocateInfo.commandPool        = m_Device.GetGraphicsCommandPool();
		commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		VK_CALL(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, m_GraphicsCommandBuffers.Data()));
	}

	void VulkanContext::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext            = nullptr;
		commandBufferBeginInfo.flags            = 0u;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		VkClearValue clearValue = {{{ 0.0f,0.0f,0.0f,1.0f }}};

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext             = nullptr;
		renderPassBeginInfo.renderPass        = m_Swapchain.GetSwapchainRenderPass();
		renderPassBeginInfo.framebuffer       = m_Swapchain.GetFramebuffer(imageIndex);
		renderPassBeginInfo.renderArea.offset = { 0u,0u };
		renderPassBeginInfo.renderArea.extent = { (uint32)m_Swapchain.GetWidth(),(uint32)m_Swapchain.GetHeight() };
		renderPassBeginInfo.clearValueCount   = 1u;
		renderPassBeginInfo.pClearValues      = &clearValue;
			
		VkViewport viewport{};
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = (float)m_Swapchain.GetWidth();
		viewport.height   = (float)m_Swapchain.GetHeight();
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0u,0u };
		scissor.extent = { (uint32)m_Swapchain.GetWidth(),(uint32)m_Swapchain.GetHeight() };

		uint32 seconds = (uint32)glfwGetTime() / 2;

		Ref<VulkanPipeline> dancingPipeline;
		if (seconds % 2 == 0)
			dancingPipeline = m_PipelineFill;
		else
			dancingPipeline = m_PipelineWireframe;

		struct PushConstant
		{
			float u_AspectRatio;
			float u_Time;
		};

		PushConstant pushConstant;
		pushConstant.u_AspectRatio = viewport.width / viewport.height;
		pushConstant.u_Time = (float)glfwGetTime();

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
		{
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dancingPipeline->GetVulkanPipeline());
			vkCmdSetViewport(commandBuffer, 0u, 1u, &viewport);
			vkCmdSetScissor(commandBuffer, 0u, 1u, &scissor);
			vkCmdPushConstants(commandBuffer, dancingPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0u, 8u, &pushConstant);
			vkCmdDraw(commandBuffer, 3u, 1u, 0u, 0u);
			vkCmdEndRenderPass(commandBuffer);
		}
		VK_CALL(vkEndCommandBuffer(commandBuffer));
	}

	void VulkanContext::ExecuteCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue)
	{
		VkDevice device = m_Device.GetVulkanDevice();

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext                = nullptr;
		submitInfo.waitSemaphoreCount   = 1u;
		submitInfo.pWaitSemaphores      = &m_Swapchain.GetImageAvailableSemaphore();
		submitInfo.pWaitDstStageMask    = &waitStage;
		submitInfo.commandBufferCount   = 1u;
		submitInfo.pCommandBuffers      = &commandBuffer;
		submitInfo.signalSemaphoreCount = 1u;
		submitInfo.pSignalSemaphores    = &m_RenderCompleteSemaphores[m_CurrentFrame];
		VK_CALL(vkQueueSubmit(queue, 1u, &submitInfo, m_PreviousFrameFinishedFences[m_CurrentFrame]));
	}

	void VulkanContext::CreateMemoryAllocator()
	{		
		VmaAllocatorCreateInfo allocatorCreateInfo{};
		allocatorCreateInfo.flags                          = 0u;
		allocatorCreateInfo.physicalDevice                 = m_PhysicalDevice.GetVulkanPhysicalDevice();
		allocatorCreateInfo.device                         = m_Device.GetVulkanDevice();
		allocatorCreateInfo.preferredLargeHeapBlockSize    = 0u; // defaults to 256 MiB
		allocatorCreateInfo.pAllocationCallbacks           = VK_NULL_HANDLE;
		allocatorCreateInfo.pDeviceMemoryCallbacks         = VK_NULL_HANDLE;
		allocatorCreateInfo.pHeapSizeLimit                 = VK_NULL_HANDLE;
		allocatorCreateInfo.pVulkanFunctions               = VK_NULL_HANDLE;
		allocatorCreateInfo.instance                       = m_Instance;
		allocatorCreateInfo.vulkanApiVersion               = VK_API_VERSION_1_3;
		allocatorCreateInfo.pTypeExternalMemoryHandleTypes = VK_NULL_HANDLE;
		VK_CALL(vmaCreateAllocator(&allocatorCreateInfo, &m_MemoryAllocator));
	}

	VulkanContext::~VulkanContext()
	{
		VkDevice device = m_Device.GetVulkanDevice();

		VK_CALL(vkDeviceWaitIdle(device));

		m_Shader.Reset();
		m_PipelineFill.Reset();
		m_PipelineWireframe.Reset();

		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyFence(device, m_PreviousFrameFinishedFences[i], nullptr);
			vkDestroySemaphore(device, m_RenderCompleteSemaphores[i], nullptr);
		}
		
		m_Swapchain.Shutdown();
		vmaDestroyAllocator(m_MemoryAllocator);
		m_Device.Shutdown();
		
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		if (s_ValidationLayerEnabled)
			GET_INSTANCE_FUNC(vkDestroyDebugUtilsMessengerEXT)(m_Instance, m_DebugMessenger, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
		
		m_MemoryAllocator = VK_NULL_HANDLE;
		m_Surface = VK_NULL_HANDLE;
		m_Instance = VK_NULL_HANDLE;
	}

	void VulkanContext::Present()
	{
	}

	void VulkanContext::OnWindowResize()
	{
		m_Swapchain.OnWindowResize();
	}

	void VulkanContext::DrawFrameTest()
	{
		VkDevice device = m_Device.GetVulkanDevice();

		VK_CALL(vkWaitForFences(device, 1u, &m_PreviousFrameFinishedFences[m_CurrentFrame], VK_TRUE, UINT64_MAX));

		if (!m_Swapchain.AquireNextImage())
			return;

		VK_CALL(vkResetFences(device, 1u, &m_PreviousFrameFinishedFences[m_CurrentFrame]));

		vkResetCommandBuffer(m_GraphicsCommandBuffers[m_CurrentFrame], 0u);
		RecordCommandBuffer(m_GraphicsCommandBuffers[m_CurrentFrame], m_Swapchain.GetCurrentImageIndex());
		ExecuteCommandBuffer(m_GraphicsCommandBuffers[m_CurrentFrame], m_Device.GetGraphicsQueue());

		m_Swapchain.Present(m_RenderCompleteSemaphores[m_CurrentFrame]);

		m_CurrentFrame++;
		if (m_CurrentFrame == MAX_FRAMES_IN_FLIGHT)
			m_CurrentFrame = 0u;
	}
}