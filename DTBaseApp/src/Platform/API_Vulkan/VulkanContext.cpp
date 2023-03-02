#include "VulkanContext.h"
#include <GLFW/glfw3.h>
#include "Platform/PlatformUtils.h"
#include <set>

namespace DT
{

	#pragma region VulkanContext

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
				__debugbreak();
				break;
			default: ASSERT(false); 
				break;
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
				LOG_WARN("  driver ver: {}", physicalDeviceProperties.driverVersion);
				LOG_WARN("  vendorID: {}", physicalDeviceProperties.vendorID);
				LOG_WARN("  deviceID: {}", physicalDeviceProperties.deviceID);
			} else {
				LOG_TRACE("  name: {}", physicalDeviceProperties.deviceName);
				LOG_TRACE("  type: {}", string_VkPhysicalDeviceType(physicalDeviceProperties.deviceType));
				LOG_TRACE("  driver ver: {}", physicalDeviceProperties.driverVersion);
				LOG_TRACE("  vendorID: {}", physicalDeviceProperties.vendorID);
				LOG_TRACE("  deviceID: {}", physicalDeviceProperties.deviceID);
			}
		}

		m_PhysicalDevice.Init(physicalDevice);
	}
	
	void VulkanContext::CreateLogicalDevice()
	{
		m_Device.Init();
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

	void VulkanContext::Shutdown()
	{
		VkDevice device = m_Device.GetVulkanDevice();

		VK_CALL(vkDeviceWaitIdle(device));

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

	#pragma endregion

	#pragma region VulkanDevice

	void VulkanPhysicalDevice::Init(VkPhysicalDevice physicalDevice)
	{
		ASSERT(physicalDevice != VK_NULL_HANDLE);
		m_PhysicalDevice = physicalDevice;

		GetSupportDetails();
		SelectQueueFamilyIndices();
	}

	bool VulkanPhysicalDevice::IsFormatFeatureSupported(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags formatFeatures) const
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProperties);
		if (tiling == VK_IMAGE_TILING_OPTIMAL) {
			for (uint8 bit = 0u; bit < 32; bit++) {
				VkFormatFeatureFlagBits currentFeature = VkFormatFeatureFlagBits(1u << bit);
				if (currentFeature & formatFeatures) {
					if (!(formatProperties.optimalTilingFeatures & currentFeature)) {
						return false;
					}
				}
			}
		} else {
			for (uint8 bit = 0u; bit < 32; bit++) {
				VkFormatFeatureFlagBits currentFeature = VkFormatFeatureFlagBits(1u << bit);
				if (currentFeature & formatFeatures) {
					if (!(formatProperties.linearTilingFeatures & currentFeature)) {
						return false;
					}
				}
			}
		}
		return true;
	}

	VkFormat VulkanPhysicalDevice::GetBestDepthOnlyFormat(VkImageTiling tiling) const
	{
		constexpr VkFormat preferenceDepthOnlyFormats[] = {
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM
		};
		for (size_t i = 0u; i < std::size(preferenceDepthOnlyFormats); i++) {
			if (IsFormatFeatureSupported(preferenceDepthOnlyFormats[i], tiling, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
				return preferenceDepthOnlyFormats[i];
			}
		}
		ASSERT(false);
		return VK_FORMAT_UNDEFINED;
	}

	VkFormat VulkanPhysicalDevice::GetBestDepthStencilFormat(VkImageTiling tiling) const
	{
		constexpr VkFormat preferenceDepthStencilFormats[] = {
			VK_FORMAT_D24_UNORM_S8_UINT,  // higher performance
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT
		};
		for (size_t i = 0u; i < std::size(preferenceDepthStencilFormats); i++) {
			if (IsFormatFeatureSupported(preferenceDepthStencilFormats[i], tiling, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
				return preferenceDepthStencilFormats[i];
			}
		}
		ASSERT(false);
		return VK_FORMAT_UNDEFINED;
	}

	void VulkanPhysicalDevice::GetSupportDetails()
	{
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_SupportDetails.Properties);
		vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_SupportDetails.Features);
		
		uint32 deviceExtensionCount = 0u;
		VK_CALL(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &deviceExtensionCount, nullptr));
		m_SupportDetails.Extensions.resize(deviceExtensionCount);
		VK_CALL(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &deviceExtensionCount, m_SupportDetails.Extensions.data()));
		m_SupportDetails.Properties.deviceID;
		uint32 queueFamilyPropertyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyPropertyCount, nullptr);
		m_SupportDetails.QueueFamilyProperties.resize(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyPropertyCount, m_SupportDetails.QueueFamilyProperties.data());
	}

	void VulkanPhysicalDevice::SelectQueueFamilyIndices()
	{
		uint32 queueFamilyPropertyCount = (uint32)m_SupportDetails.QueueFamilyProperties.size();

		LOG_INFO("Found {} queue families:", queueFamilyPropertyCount);
		VkSurfaceKHR surface = VulkanContext::GetSurface();
		for (uint32 i = 0u; i < queueFamilyPropertyCount; i++)
		{
			VkBool32 presentSupported;
			VK_CALL(vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, surface, &presentSupported));

			bool hasGraphics = (m_SupportDetails.QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT);
			bool hasTransfer = (m_SupportDetails.QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT);
			bool hasCompute  = (m_SupportDetails.QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT);

			// try find (graphics + transfer + compute) which is the all-in-one queue
			if (hasGraphics && hasTransfer && hasCompute)
				m_QueueFamilyIndices.GraphicsIndex = i;

			// try find a transfer only queue (DMA unit usually)
			if (!hasGraphics && hasTransfer && !hasCompute)
				m_QueueFamilyIndices.TransferIndex = i;

			// try find a (compute + transfer) only queue
			if (!hasGraphics && hasCompute && hasTransfer)
				m_QueueFamilyIndices.ComputeIndex = i;

			// try use the present queue that supports graphics
			if (hasGraphics && (bool)presentSupported)
				m_QueueFamilyIndices.PresentIndex = i;

			// otherwise, if not present use any queue that supports present (compute or transfer)
			if (!hasGraphics && (bool)presentSupported && !m_QueueFamilyIndices.PresentIndex.has_value())
				m_QueueFamilyIndices.PresentIndex = i;
		}

		// if no all-in-one queue found
		if (!m_QueueFamilyIndices.GraphicsIndex.has_value())
			MessageBoxes::ShowError("No GPU queue with (graphics + transfer + compute) capabilities was found!");

		// if no dedicated transfer queue found
		if (!m_QueueFamilyIndices.TransferIndex.has_value())
		{
			LOG_WARN("No transfer-only queue found, fallback to the all-in-one graphics queue");
			m_QueueFamilyIndices.TransferIndex = m_QueueFamilyIndices.GraphicsIndex;
		}

		// if no dedicated (compute + transfer) queue found
		if (!m_QueueFamilyIndices.ComputeIndex.has_value())
		{
			LOG_WARN("No (compute + transfer) only queue found, fallback to the all-in-one graphics queue");
			m_QueueFamilyIndices.ComputeIndex = m_QueueFamilyIndices.GraphicsIndex;
		}

		if (!m_QueueFamilyIndices.PresentIndex.has_value())
			MessageBoxes::ShowError("No GPU queue with present capabilities found!");

		for (uint32 i = 0u; i < queueFamilyPropertyCount; i++)
		{
			VkBool32 presentSupported;
			VK_CALL(vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, surface, &presentSupported));

			std::string currentFamilyNames;

			if (i == m_QueueFamilyIndices.GraphicsIndex.value())
				currentFamilyNames += "[GraphicsIndex]";

			if (i == m_QueueFamilyIndices.TransferIndex.value())
				currentFamilyNames += "[TransferIndex]";

			if (i == m_QueueFamilyIndices.ComputeIndex.value())
				currentFamilyNames += "[ComputeIndex]";

			if (i == m_QueueFamilyIndices.PresentIndex.value())
				currentFamilyNames += "[PresentIndex]";

			ASSERT(!currentFamilyNames.empty());
			LOG_WARN("  index {} {}:", i, currentFamilyNames);
			LOG_TRACE("    flags: {}", string_VkQueueFlags(m_SupportDetails.QueueFamilyProperties[i].queueFlags));
			LOG_TRACE("    presentSupport: {}", (bool)presentSupported);
		}
	}

	bool VulkanPhysicalDevice::IsExtensionSupported(const char* deviceExtensionName)
	{
		for (VkExtensionProperties& extensionProperty : m_SupportDetails.Extensions)
		{
			if (strcmp(extensionProperty.extensionName, deviceExtensionName) == 0)
				return true;
		}
		return false;
	}

	void VulkanDevice::Init()
	{
		CreateDevice();
		CreateCommandPools();
		CreateSyncronizationObjects();
	}

	VkCommandBuffer VulkanDevice::BeginCommandBuffer(QueueType queueType, bool oneTimeSubmit)
	{
		ASSERT(m_ActiveCommandBuffer == VK_NULL_HANDLE);
		ASSERT(m_ActiveCommandPool == VK_NULL_HANDLE);
		ASSERT(m_ActiveQueue == VK_NULL_HANDLE);

		m_ActiveQueue = GetQueue(queueType);
		m_ActiveCommandPool = GetCommandPool(queueType, oneTimeSubmit);

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext              = nullptr;
		commandBufferAllocateInfo.commandPool        = m_ActiveCommandPool;
		commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1u;
		VK_CALL(vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, &m_ActiveCommandBuffer));
		
		VkCommandBufferBeginInfo commandBufferBeginInfo{};
		commandBufferBeginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext            = nullptr;
		commandBufferBeginInfo.flags            = (oneTimeSubmit ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0u);
		commandBufferBeginInfo.pInheritanceInfo = nullptr;
		VK_CALL(vkBeginCommandBuffer(m_ActiveCommandBuffer, &commandBufferBeginInfo));

		return m_ActiveCommandBuffer;
	}

	void VulkanDevice::EndCommandBuffer()
	{
		ASSERT(m_ActiveCommandBuffer);
		ASSERT(m_ActiveCommandPool);
		ASSERT(m_ActiveQueue);

		VK_CALL(vkEndCommandBuffer(m_ActiveCommandBuffer));

		VkSubmitInfo submitInfo{};
		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext                = nullptr;
		submitInfo.waitSemaphoreCount   = 0u;
		submitInfo.pWaitSemaphores      = nullptr;
		submitInfo.pWaitDstStageMask    = nullptr;
		submitInfo.commandBufferCount   = 1u;
		submitInfo.pCommandBuffers      = &m_ActiveCommandBuffer;
		submitInfo.signalSemaphoreCount = 0u;
		submitInfo.pSignalSemaphores    = nullptr;
		VK_CALL(vkQueueSubmit(m_ActiveQueue, 1u, &submitInfo, m_WaitFence));
		
		VK_CALL(vkWaitForFences(m_Device, 1u, &m_WaitFence, VK_TRUE, UINT64_MAX));
		VK_CALL(vkResetFences(m_Device, 1u, &m_WaitFence));

		vkFreeCommandBuffers(m_Device, m_ActiveCommandPool, 1u, &m_ActiveCommandBuffer);

		m_ActiveCommandBuffer = VK_NULL_HANDLE;
		m_ActiveCommandPool = VK_NULL_HANDLE;
		m_ActiveQueue = VK_NULL_HANDLE;
	}

	void VulkanDevice::CreateDevice()
	{
		VulkanPhysicalDevice& physicalDevice = VulkanContext::GetCurrentPhysicalDevice();
		const QueueFamilyIndices& queueFamilyIndices = physicalDevice.GetQueueFamilyIndices();
		
		std::set<uint32> uniqueQueueIndices = {
			queueFamilyIndices.GraphicsIndex.value(),
			queueFamilyIndices.TransferIndex.value(),
			queueFamilyIndices.ComputeIndex.value(),
			queueFamilyIndices.PresentIndex.value()
		};

		constexpr float defaultQueuePriority = 1.0f;
		
		uint32 i = 0u;
		VkDeviceQueueCreateInfo deviceQueueCreateInfos[3u];
		ASSERT(std::size(deviceQueueCreateInfos) >= uniqueQueueIndices.size());
		for (uint32 queueIndex : uniqueQueueIndices)
		{
			VkDeviceQueueCreateInfo& deviceQueueCreateInfo = deviceQueueCreateInfos[i++];
			deviceQueueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCreateInfo.pNext            = nullptr;
			deviceQueueCreateInfo.flags            = 0u;
			deviceQueueCreateInfo.queueFamilyIndex = queueIndex;
			deviceQueueCreateInfo.queueCount       = 1u;
			deviceQueueCreateInfo.pQueuePriorities = &defaultQueuePriority;
		}

		// requested extension and availability check
		std::vector<const char*> deviceExtensions = BuildRequestedDeviceExtensions();
		for (const char* extensionName : deviceExtensions)
		{
			if (!physicalDevice.IsExtensionSupported(extensionName))
				MessageBoxes::ShowError(std::format("Extension {} is not supported on the selected GPU", extensionName));
		}
		VkPhysicalDeviceFeatures enabledFeatures{};
		BuildEnabledFeatures(&enabledFeatures);

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext                   = nullptr;
		deviceCreateInfo.flags                   = 0u;
		deviceCreateInfo.queueCreateInfoCount    = (uint32)uniqueQueueIndices.size();
		deviceCreateInfo.pQueueCreateInfos       = deviceQueueCreateInfos;
		deviceCreateInfo.enabledLayerCount       = 0u;
		deviceCreateInfo.ppEnabledLayerNames     = nullptr;
		deviceCreateInfo.enabledExtensionCount   = (uint32)deviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		deviceCreateInfo.pEnabledFeatures        = &enabledFeatures;
		VK_CALL(vkCreateDevice(physicalDevice.GetVulkanPhysicalDevice(), &deviceCreateInfo, nullptr, &m_Device));

		vkGetDeviceQueue(m_Device, queueFamilyIndices.GraphicsIndex.value(), 0u, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, queueFamilyIndices.TransferIndex.value(), 0u, &m_TransferQueue);
		vkGetDeviceQueue(m_Device, queueFamilyIndices.ComputeIndex.value(), 0u, &m_ComputeQueue);
		vkGetDeviceQueue(m_Device, queueFamilyIndices.PresentIndex.value(), 0u, &m_PresentQueue);
	}

	void VulkanDevice::CreateCommandPools()
	{
		VulkanPhysicalDevice& physicalDevice = VulkanContext::GetCurrentPhysicalDevice();
		const QueueFamilyIndices& queueFamilyIndices = physicalDevice.GetQueueFamilyIndices();

		VkCommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext            = nullptr;

		commandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.GraphicsIndex.value();
		VK_CALL(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_GraphicsCommandPool));
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.TransferIndex.value();
		VK_CALL(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_TransferCommandPool));
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.ComputeIndex.value();
		VK_CALL(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_ComputeCommandPool));

		commandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.GraphicsIndex.value();
		VK_CALL(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_TransientGraphicsCommandPool));
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.TransferIndex.value();
		VK_CALL(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_TransientTransferCommandPool));
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.ComputeIndex.value();
		VK_CALL(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_TransientComputeCommandPool));
	}

	void VulkanDevice::CreateSyncronizationObjects()
	{
		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = 0u;
		VK_CALL(vkCreateFence(m_Device, &fenceCreateInfo, nullptr, &m_WaitFence));
	}

	VkQueue VulkanDevice::GetQueue(QueueType queueType)
	{
		switch (queueType)
		{
			case QueueType::Graphics: return m_GraphicsQueue;
			case QueueType::Transfer: return m_TransferQueue;
			case QueueType::Compute:  return m_ComputeQueue;
			case QueueType::Present:  return m_PresentQueue;
		}
		ASSERT(false);
		return m_GraphicsQueue;
	}

	VkCommandPool VulkanDevice::GetCommandPool(QueueType queueType, bool transientPool)
	{
		switch (queueType)
		{
			case QueueType::Graphics: return (transientPool ? m_TransientGraphicsCommandPool : m_GraphicsCommandPool);
			case QueueType::Transfer: return (transientPool ? m_TransientTransferCommandPool : m_TransferCommandPool);
			case QueueType::Compute:  return (transientPool ? m_TransientComputeCommandPool : m_ComputeCommandPool);
		}
		ASSERT(false);
		return m_GraphicsCommandPool;
	}

	void VulkanDevice::Shutdown()
	{
		vkDestroyFence(m_Device, m_WaitFence, nullptr);
		vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_TransferCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_ComputeCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_TransientGraphicsCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_TransientTransferCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_TransientComputeCommandPool, nullptr);
		vkDestroyDevice(m_Device, nullptr);

		m_WaitFence = VK_NULL_HANDLE;
		m_GraphicsCommandPool = VK_NULL_HANDLE;
		m_TransferCommandPool = VK_NULL_HANDLE;
		m_ComputeCommandPool = VK_NULL_HANDLE;
		m_TransientGraphicsCommandPool = VK_NULL_HANDLE;
		m_TransientTransferCommandPool = VK_NULL_HANDLE;
		m_TransientComputeCommandPool = VK_NULL_HANDLE;
		m_Device = VK_NULL_HANDLE;
	}
	
	std::vector<const char*> VulkanDevice::BuildRequestedDeviceExtensions()
	{
		std::vector<const char*> deviceExtensions;
		deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		return deviceExtensions;
	}
	
	void VulkanDevice::BuildEnabledFeatures(VkPhysicalDeviceFeatures* features)
	{
		features->wideLines = VK_TRUE;
		features->fillModeNonSolid = VK_TRUE;
		features->samplerAnisotropy = VK_TRUE;

		VulkanPhysicalDevice& physicalDevice = VulkanContext::GetCurrentPhysicalDevice();
		const VkPhysicalDeviceFeatures& supportedFeatures = physicalDevice.GetSupportedFeatures();
		const VkBool32* supported = (VkBool32*)&supportedFeatures;
		const VkBool32* requested = (VkBool32*)features;
		constexpr uint32 featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
		for (uint32 i = 0u; i < featureCount; i++, requested++, supported++)
		{
			if (*requested && !(*supported))
				MessageBoxes::ShowError(std::format("Requested device feature (number {}) is not supported!", i + 1u));
		}
	}

	#pragma endregion

}