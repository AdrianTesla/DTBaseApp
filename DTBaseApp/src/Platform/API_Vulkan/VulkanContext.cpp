#include "VulkanContext.h"
#include <GLFW/glfw3.h>
#include "Platform/PlatformUtils.h"

namespace DT
{
	struct PushConstant
	{
		float AspectRatio;
		float Time;
	};

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
		VkDevice device = GetCurrentVulkanDevice();

		Buffer vertSPIRV = FileSystem::ReadFileBinary("assets/shaders/vert.spv");
		Buffer fragSPIRV = FileSystem::ReadFileBinary("assets/shaders/frag.spv");

		// vertex shader
		VkShaderModule vertShaderModule = VK_NULL_HANDLE;
		VkShaderModuleCreateInfo vertShaderModuleCreateInfo{};
		vertShaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertShaderModuleCreateInfo.pNext    = nullptr;
		vertShaderModuleCreateInfo.flags    = 0u;
		vertShaderModuleCreateInfo.codeSize = vertSPIRV.Size;
		vertShaderModuleCreateInfo.pCode    = (uint32*)vertSPIRV.Data;
		VK_CALL(vkCreateShaderModule(device, &vertShaderModuleCreateInfo, nullptr, &vertShaderModule));

		// fragment shader
		VkShaderModule fragShaderModule = VK_NULL_HANDLE;
		VkShaderModuleCreateInfo fragShaderModuleCreateInfo{};
		fragShaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fragShaderModuleCreateInfo.pNext    = nullptr;
		fragShaderModuleCreateInfo.flags    = 0u;
		fragShaderModuleCreateInfo.codeSize = fragSPIRV.Size;
		fragShaderModuleCreateInfo.pCode    = (uint32*)fragSPIRV.Data;
		VK_CALL(vkCreateShaderModule(device, &fragShaderModuleCreateInfo, nullptr, &fragShaderModule));

		VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2];

		pipelineShaderStageCreateInfos[0].sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipelineShaderStageCreateInfos[0].pNext               = nullptr;
		pipelineShaderStageCreateInfos[0].flags               = 0u;
		pipelineShaderStageCreateInfos[0].stage               = VK_SHADER_STAGE_VERTEX_BIT;
		pipelineShaderStageCreateInfos[0].module              = vertShaderModule;
		pipelineShaderStageCreateInfos[0].pName               = "main";
		pipelineShaderStageCreateInfos[0].pSpecializationInfo = nullptr;

		pipelineShaderStageCreateInfos[1].sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pipelineShaderStageCreateInfos[1].pNext               = nullptr;
		pipelineShaderStageCreateInfos[1].flags               = 0u;
		pipelineShaderStageCreateInfos[1].stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
		pipelineShaderStageCreateInfos[1].module              = fragShaderModule;
		pipelineShaderStageCreateInfos[1].pName               = "main";
		pipelineShaderStageCreateInfos[1].pSpecializationInfo = nullptr;

		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
		pipelineVertexInputStateCreateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipelineVertexInputStateCreateInfo.pNext                           = nullptr;
		pipelineVertexInputStateCreateInfo.flags                           = 0u;
		pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount   = 0u;
		pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions      = nullptr;
		pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0u;
		pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions    = nullptr;

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
		pipelineInputAssemblyStateCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		pipelineInputAssemblyStateCreateInfo.pNext                  = nullptr;
		pipelineInputAssemblyStateCreateInfo.flags                  = 0u;
		pipelineInputAssemblyStateCreateInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo{};
		pipelineTessellationStateCreateInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		pipelineTessellationStateCreateInfo.pNext              = nullptr;
		pipelineTessellationStateCreateInfo.flags              = 0u;
		pipelineTessellationStateCreateInfo.patchControlPoints = 1u;

		VkViewport viewport{};
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = (float)m_Swapchain.GetWidth();
		viewport.height   = (float)m_Swapchain.GetHeight();
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0u,0u };
		scissor.extent = { m_Swapchain.GetWidth(),m_Swapchain.GetHeight() };

		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
		pipelineViewportStateCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipelineViewportStateCreateInfo.pNext         = nullptr;
		pipelineViewportStateCreateInfo.flags         = 0u;
		pipelineViewportStateCreateInfo.viewportCount = 1u;
		pipelineViewportStateCreateInfo.pViewports    = &viewport;
		pipelineViewportStateCreateInfo.scissorCount  = 1u;
		pipelineViewportStateCreateInfo.pScissors     = &scissor;

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
		pipelineRasterizationStateCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipelineRasterizationStateCreateInfo.pNext                   = nullptr;
		pipelineRasterizationStateCreateInfo.flags                   = 0u;
		pipelineRasterizationStateCreateInfo.depthClampEnable        = VK_FALSE;
		pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		pipelineRasterizationStateCreateInfo.polygonMode             = VK_POLYGON_MODE_FILL;
		pipelineRasterizationStateCreateInfo.cullMode                = VK_CULL_MODE_NONE;
		pipelineRasterizationStateCreateInfo.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		pipelineRasterizationStateCreateInfo.depthBiasEnable         = VK_FALSE;
		pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
		pipelineRasterizationStateCreateInfo.depthBiasClamp          = 0.0f;
		pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor    = 0.0f;
		pipelineRasterizationStateCreateInfo.lineWidth               = 1.0f;

		VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
		pipelineMultisampleStateCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		pipelineMultisampleStateCreateInfo.pNext                 = nullptr;
		pipelineMultisampleStateCreateInfo.flags                 = 0u;
		pipelineMultisampleStateCreateInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
		pipelineMultisampleStateCreateInfo.sampleShadingEnable   = VK_FALSE;
		pipelineMultisampleStateCreateInfo.minSampleShading      = 0.0f;
		pipelineMultisampleStateCreateInfo.pSampleMask           = nullptr;
		pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
		pipelineMultisampleStateCreateInfo.alphaToOneEnable      = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
		pipelineDepthStencilStateCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		pipelineDepthStencilStateCreateInfo.pNext                 = nullptr;
		pipelineDepthStencilStateCreateInfo.flags                 = 0u;
		pipelineDepthStencilStateCreateInfo.depthTestEnable       = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.depthWriteEnable      = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.depthCompareOp        = VK_COMPARE_OP_EQUAL;
		pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.stencilTestEnable     = VK_FALSE;
		pipelineDepthStencilStateCreateInfo.front.failOp          = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.front.passOp          = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.front.depthFailOp     = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.front.compareOp       = VK_COMPARE_OP_EQUAL;
		pipelineDepthStencilStateCreateInfo.front.compareMask     = 0u;
		pipelineDepthStencilStateCreateInfo.front.writeMask       = 0u;
		pipelineDepthStencilStateCreateInfo.front.reference       = 0u;
		pipelineDepthStencilStateCreateInfo.back.failOp           = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.back.passOp           = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.back.depthFailOp      = VK_STENCIL_OP_KEEP;
		pipelineDepthStencilStateCreateInfo.back.compareOp        = VK_COMPARE_OP_EQUAL;
		pipelineDepthStencilStateCreateInfo.back.compareMask      = 0u;
		pipelineDepthStencilStateCreateInfo.back.writeMask        = 0u;
		pipelineDepthStencilStateCreateInfo.back.reference        = 0u;
		pipelineDepthStencilStateCreateInfo.minDepthBounds        = 0.0f;
		pipelineDepthStencilStateCreateInfo.maxDepthBounds        = 1.0f;

		VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentStates[1];
		for (size_t i = 0u; i < std::size(pipelineColorBlendAttachmentStates); i++)
		{
			pipelineColorBlendAttachmentStates[i].blendEnable         = VK_FALSE;
			pipelineColorBlendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			pipelineColorBlendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			pipelineColorBlendAttachmentStates[i].colorBlendOp        = VK_BLEND_OP_ADD;
			pipelineColorBlendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			pipelineColorBlendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			pipelineColorBlendAttachmentStates[i].alphaBlendOp        = VK_BLEND_OP_ADD;
			pipelineColorBlendAttachmentStates[i].colorWriteMask      = 
				VK_COLOR_COMPONENT_R_BIT | 
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;
		}

		VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
		pipelineColorBlendStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		pipelineColorBlendStateCreateInfo.pNext             = nullptr;
		pipelineColorBlendStateCreateInfo.flags             = 0u;
		pipelineColorBlendStateCreateInfo.logicOpEnable     = VK_FALSE;
		pipelineColorBlendStateCreateInfo.logicOp           = VK_LOGIC_OP_NO_OP;
		pipelineColorBlendStateCreateInfo.attachmentCount   = (uint32)std::size(pipelineColorBlendAttachmentStates);
		pipelineColorBlendStateCreateInfo.pAttachments      = pipelineColorBlendAttachmentStates;
		pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
		pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
		pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
		pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
		pipelineDynamicStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		pipelineDynamicStateCreateInfo.pNext             = nullptr;
		pipelineDynamicStateCreateInfo.flags             = 0u;
		pipelineDynamicStateCreateInfo.dynamicStateCount = (uint32)std::size(dynamicStates);
		pipelineDynamicStateCreateInfo.pDynamicStates    = dynamicStates;

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset     = 0u;
		pushConstantRange.size       = sizeof(PushConstant);

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext                  = nullptr;
		pipelineLayoutCreateInfo.flags                  = 0u;
		pipelineLayoutCreateInfo.setLayoutCount         = 0u;
		pipelineLayoutCreateInfo.pSetLayouts            = nullptr;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1u;
		pipelineLayoutCreateInfo.pPushConstantRanges    = &pushConstantRange;
		VK_CALL(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout));

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
		graphicsPipelineCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.pNext               = nullptr;
		graphicsPipelineCreateInfo.flags               = 0u;
		graphicsPipelineCreateInfo.stageCount          = (uint32)std::size(pipelineShaderStageCreateInfos);
		graphicsPipelineCreateInfo.pStages             = pipelineShaderStageCreateInfos;
		graphicsPipelineCreateInfo.pVertexInputState   = &pipelineVertexInputStateCreateInfo;
		graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
		graphicsPipelineCreateInfo.pTessellationState  = &pipelineTessellationStateCreateInfo;
		graphicsPipelineCreateInfo.pViewportState      = &pipelineViewportStateCreateInfo;
		graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
		graphicsPipelineCreateInfo.pMultisampleState   = &pipelineMultisampleStateCreateInfo;
		graphicsPipelineCreateInfo.pDepthStencilState  = &pipelineDepthStencilStateCreateInfo;
		graphicsPipelineCreateInfo.pColorBlendState    = &pipelineColorBlendStateCreateInfo;
		graphicsPipelineCreateInfo.pDynamicState       = &pipelineDynamicStateCreateInfo;
		graphicsPipelineCreateInfo.layout              = m_PipelineLayout;
		graphicsPipelineCreateInfo.renderPass          = m_Swapchain.GetSwapchainRenderPass();
		graphicsPipelineCreateInfo.subpass             = 0u;
		graphicsPipelineCreateInfo.basePipelineHandle  = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex   = 0;
		VK_CALL(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1u, &graphicsPipelineCreateInfo, nullptr, &m_Pipeline));

		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
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

		PushConstant pushConstant;
		pushConstant.AspectRatio = viewport.width / viewport.height;
		pushConstant.Time = (float)glfwGetTime();

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
		{
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
			vkCmdSetViewport(commandBuffer, 0u, 1u, &viewport);
			vkCmdSetScissor(commandBuffer, 0u, 1u, &scissor);
			vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(PushConstant), &pushConstant);
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

		for (uint32 i = 0u; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyFence(device, m_PreviousFrameFinishedFences[i], nullptr);
			vkDestroySemaphore(device, m_RenderCompleteSemaphores[i], nullptr);
		}
		
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
		vkDestroyPipeline(device, m_Pipeline, nullptr);
		
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