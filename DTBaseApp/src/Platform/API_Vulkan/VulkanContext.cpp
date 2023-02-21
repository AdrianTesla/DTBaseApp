#include "VulkanContext.h"
#include <GLFW/glfw3.h>
#include "Platform/PlatformUtils.h"

namespace DT
{
	static struct UniformBufferData
	{
		float ScreenWidth;
		float ScreenHeight;
		float AspectRatio;
		float Time;
	} s_UniformBufferData;

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

	bool VulkanContext::IsInstanceExtensionSupported(const char* extensionName)
	{
		for (const VkExtensionProperties& extension : s_Context->m_AvailableInstanceExtensions)
		{
			if (strcmp(extension.extensionName, extensionName) == 0)
				return true;
		}
		return false;
	}

	bool VulkanContext::IsInstanceLayerSupported(const char* layerName)
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
		CreateBuffers();
		CreateCommandBuffers();
		CreateDescriptorPools();
		CreateDescriptorSets();
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

	void VulkanContext::CreateBuffers()
	{
		struct Vertex
		{
			struct
			{
				float x;
				float y;
			} Position;
			struct
			{
				float r;
				float g;
				float b;
			} Color;
		};

		Vertex vertices[4];
		vertices[0].Position = { -0.5f,-0.5f };
		vertices[1].Position = { -0.5f,+0.5f };
		vertices[2].Position = { +0.5f,+0.5f };
		vertices[3].Position = { +0.5f,-0.5f };

		vertices[0].Color = { 1.0f,1.0f,0.0f };
		vertices[1].Color = { 1.0f,0.4f,0.0f };
		vertices[2].Color = { 1.0f,1.0f,0.0f };
		vertices[3].Color = { 1.0f,0.4f,0.0f };

		uint32 indices[6] = { 0u,1u,2u, 0u,2u,3u };

		m_VertexBuffer = Ref<VulkanVertexBuffer>::Create(vertices, sizeof(vertices));
		m_IndexBuffer = Ref<VulkanIndexBuffer>::Create(indices, sizeof(indices));

		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			m_UniformBuffers[i] = Ref<VulkanUniformBuffer>::Create(sizeof(UniformBufferData));
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

	void VulkanContext::CreateDescriptorPools()
	{
		VkDevice device = m_Device.GetVulkanDevice();

		VkDescriptorPoolSize descriptorPoolSize{};
		descriptorPoolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSize.descriptorCount = 1u;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
		descriptorPoolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.pNext         = nullptr;
		descriptorPoolCreateInfo.flags         = 0u;
		descriptorPoolCreateInfo.maxSets       = MAX_FRAMES_IN_FLIGHT;
		descriptorPoolCreateInfo.poolSizeCount = 1u;
		descriptorPoolCreateInfo.pPoolSizes    = &descriptorPoolSize;
		VK_CALL(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &m_DescriptorPool));
	}

	void VulkanContext::CreateDescriptorSets()
	{
		VkDevice device = m_Device.GetVulkanDevice();

		VkDescriptorSetLayout descriptorSetLayout = m_PipelineFill->GetDescriptorSetLayout();
		VkDescriptorSetLayout descriptorSetLayouts[MAX_FRAMES_IN_FLIGHT];
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			descriptorSetLayouts[i] = descriptorSetLayout;

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext              = nullptr;
		descriptorSetAllocateInfo.descriptorPool     = m_DescriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
		descriptorSetAllocateInfo.pSetLayouts        = descriptorSetLayouts;
		VK_CALL(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, m_DescriptorSets.Data()));

		VkWriteDescriptorSet writeDescriptorSets[MAX_FRAMES_IN_FLIGHT];
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			writeDescriptorSets[i].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[i].pNext            = nullptr;
			writeDescriptorSets[i].dstSet           = m_DescriptorSets[i];
			writeDescriptorSets[i].dstBinding       = 0u;
			writeDescriptorSets[i].dstArrayElement  = 0u;
			writeDescriptorSets[i].descriptorCount  = 1u;
			writeDescriptorSets[i].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[i].pImageInfo       = nullptr;
			writeDescriptorSets[i].pBufferInfo      = &m_UniformBuffers[i]->GetDescriptorBufferInfo();
			writeDescriptorSets[i].pTexelBufferView = nullptr;
		}
		vkUpdateDescriptorSets(device, (uint32)std::size(writeDescriptorSets), writeDescriptorSets, 0u, nullptr);
	}

	void VulkanContext::UpdateUniformBuffers()
	{
		float width = (float)m_Swapchain.GetWidth();
		float height = (float)m_Swapchain.GetHeight();

		s_UniformBufferData.ScreenWidth  = width;
		s_UniformBufferData.ScreenHeight = height;
		s_UniformBufferData.AspectRatio  = width / height;
		s_UniformBufferData.Time         = (float)glfwGetTime();
		
		m_UniformBuffers[m_CurrentFrame]->SetData(&s_UniformBufferData, sizeof(s_UniformBufferData));
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
		scissor.extent = { (uint32)viewport.width,(uint32)viewport.height };

		uint32 seconds = (uint32)glfwGetTime();

		Ref<VulkanPipeline> dancingPipeline;
		if (seconds % 2 == 0)
			dancingPipeline = m_PipelineFill;
		else
			dancingPipeline = m_PipelineWireframe;

		VkDeviceSize offsets = 0u;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
		{
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dancingPipeline->GetVulkanPipeline());
			vkCmdBindVertexBuffers(commandBuffer, 0u, 1u, &m_VertexBuffer->GetVulkanBuffer(), &offsets);
			vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetVulkanBuffer(), 0u, VK_INDEX_TYPE_UINT32);
			vkCmdSetViewport(commandBuffer, 0u, 1u, &viewport);
			vkCmdSetScissor(commandBuffer, 0u, 1u, &scissor);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, dancingPipeline->GetPipelineLayout(), 0u, 1u, &m_DescriptorSets[m_CurrentFrame], 0u, nullptr);
			vkCmdDrawIndexed(commandBuffer, m_IndexBuffer->GetIndexCount(), 1u, 0u, 0u, 0u);
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
		VK_CALL(vmaCreateAllocator(&allocatorCreateInfo, &m_VulkanMemoryAllocator));
	}

	VulkanContext::~VulkanContext()
	{
		VkDevice device = m_Device.GetVulkanDevice();

		VK_CALL(vkDeviceWaitIdle(device));

		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);

		m_Shader.Reset();
		m_VertexBuffer.Reset();
		m_IndexBuffer.Reset();
		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
			m_UniformBuffers[i].Reset();
		m_PipelineFill.Reset();
		m_PipelineWireframe.Reset();

		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyFence(device, m_PreviousFrameFinishedFences[i], nullptr);
			vkDestroySemaphore(device, m_RenderCompleteSemaphores[i], nullptr);
		}
		
		m_Swapchain.Shutdown();
		vmaDestroyAllocator(m_VulkanMemoryAllocator);
		m_Device.Shutdown();
		
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		if (s_ValidationLayerEnabled)
			GET_INSTANCE_FUNC(vkDestroyDebugUtilsMessengerEXT)(m_Instance, m_DebugMessenger, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
		
		m_VulkanMemoryAllocator = VK_NULL_HANDLE;
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

		UpdateUniformBuffers();

		RecordCommandBuffer(m_GraphicsCommandBuffers[m_CurrentFrame], m_Swapchain.GetCurrentImageIndex());
		ExecuteCommandBuffer(m_GraphicsCommandBuffers[m_CurrentFrame], m_Device.GetGraphicsQueue());

		m_Swapchain.Present(m_RenderCompleteSemaphores[m_CurrentFrame]);

		m_CurrentFrame++;
		if (m_CurrentFrame == MAX_FRAMES_IN_FLIGHT)
			m_CurrentFrame = 0u;
	}
}