#include "VulkanContext.h"
#include <GLFW/glfw3.h>
#include "Platform/PlatformUtils.h"

namespace DT
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanErrorCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
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
		for (const VkExtensionProperties& extension : s_AvailableInstanceExtensions)
		{
			if (strcmp(extension.extensionName, extensionName) == 0)
				return true;
		}
		return false;
	}

	bool VulkanContext::IsInstanceLayerSupported(const char* layerName)
	{
		for (const VkLayerProperties& layer : s_AvailableInstanceLayers)
		{
			if (strcmp(layer.layerName, layerName) == 0)
				return true;
		}
		return false;
	}
	
	bool VulkanPhysicalDevice::IsExtensionSupported(const char* deviceExtensionName)
	{
		for (VkExtensionProperties& extensionProperty : m_SupportedDeviceExtensions)
		{
			if (strcmp(extensionProperty.extensionName, deviceExtensionName) == 0)
				return true;
		}
		return false;
	}

	VulkanContext::VulkanContext(const Ref<Window>& window)
		: m_Window(window)
	{}

	void VulkanContext::Init()
	{
		if (!glfwVulkanSupported())
			MessageBoxes::ShowError("Vulkan is not supported on the system!", "Error");
		
		CreateVulkanInstance();
		SelectPhysicalDevice();
	}

	void VulkanContext::CreateVulkanInstance()
	{
		// enumerate available extensions
		uint32 extensionCount;
		VK_CALL(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
		s_AvailableInstanceExtensions.resize(extensionCount);
		VK_CALL(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, s_AvailableInstanceExtensions.data()));
		
		// enumerate available layers
		uint32 layerCount;
		VK_CALL(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
		s_AvailableInstanceLayers.resize(layerCount);
		VK_CALL(vkEnumerateInstanceLayerProperties(&layerCount, s_AvailableInstanceLayers.data()));

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
		
		// inject a debug messenger before instance creation
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
			debugUtilsMessengerCreateInfo.pfnUserCallback = VulkanErrorCallback;
			debugUtilsMessengerCreateInfo.pUserData       = nullptr;
			instanceCreateInfo.pNext = &debugUtilsMessengerCreateInfo;
		}
		
		VK_CALL(vkCreateInstance(&instanceCreateInfo, nullptr, &s_Instance));

		// create the actual debug messenger
		if (s_ValidationLayerEnabled)
			VK_CALL(GET_INSTANCE_FUNC(vkCreateDebugUtilsMessengerEXT)(s_Instance, &debugUtilsMessengerCreateInfo, nullptr, &m_DebugMessenger));
	}

	void VulkanContext::SelectPhysicalDevice()
	{
		// enumerate available physical devices
		uint32 physicalDeviceCount;
		VK_CALL(vkEnumeratePhysicalDevices(s_Instance, &physicalDeviceCount, nullptr));
		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		VK_CALL(vkEnumeratePhysicalDevices(s_Instance, &physicalDeviceCount, physicalDevices.data()));

		if (physicalDeviceCount == 0u)
			MessageBoxes::ShowError("Error", "No GPU's found on the system!");

		LOG_INFO("Found {} physical device(s):", physicalDeviceCount);

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		for (uint32 i = 0u; i < physicalDeviceCount; i++)
		{
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProperties);

			VkPhysicalDeviceFeatures physicalDeviceFeatures{};
			vkGetPhysicalDeviceFeatures(physicalDevices[i], &physicalDeviceFeatures);
			
			LOG_WARN("  Name: {}", physicalDeviceProperties.deviceName);
			LOG_WARN("  Type: {}", string_VkPhysicalDeviceType(physicalDeviceProperties.deviceType));

			if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				physicalDevice = physicalDevices[i];
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE)
			physicalDevice = physicalDevices[0];

		m_PhysicalDevice.Init(physicalDevice);
	}

	void VulkanPhysicalDevice::Init(VkPhysicalDevice physicalDevice)
	{
		ASSERT(physicalDevice != VK_NULL_HANDLE);
		m_PhysicalDevice = physicalDevice;

		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties);
		vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_PhysicalDeviceFeatures);
		
		uint32 deviceExtensionCount = 0u;
		VK_CALL(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &deviceExtensionCount, nullptr));
		m_SupportedDeviceExtensions.resize(deviceExtensionCount);
		VK_CALL(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &deviceExtensionCount, m_SupportedDeviceExtensions.data()));

		uint32 queueFamilyPropertyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyPropertyCount, nullptr);
		m_QueueFamilyProperties.resize(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyPropertyCount, m_QueueFamilyProperties.data());

		for (uint32 i = 0u; i < queueFamilyPropertyCount; i++)
		{
			LOG_TRACE("QueueIndex {}: {}", i, string_VkQueueFlags(m_QueueFamilyProperties[i].queueFlags));
			
			bool hasGraphics = (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT);
			bool hasTransfer = (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT);
			bool hasCompute  = (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT);

			// try find (graphics + transfer + compute) which is the all-in-one queue
			if (hasGraphics && hasTransfer && hasCompute)
				m_QueueFamilyIndices.GraphicsIndex = i;

			// try find a transfer only queue (DMA unit usually)
			if (!hasGraphics && hasTransfer && !hasCompute)
				m_QueueFamilyIndices.TransferIndex = i;

			// try find a (compute + transfer) only queue
			if (!hasGraphics && hasTransfer && hasCompute)
				m_QueueFamilyIndices.ComputeIndex = i;
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
			m_QueueFamilyIndices.TransferIndex = m_QueueFamilyIndices.GraphicsIndex;
		}
	}
	
	void VulkanContext::CreateLogicalDevice()
	{
		//VkDeviceCreateInfo deviceCreateInfo{};
		//deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//deviceCreateInfo.pNext                   = nullptr;
		//deviceCreateInfo.flags                   = 0u;
		//deviceCreateInfo.queueCreateInfoCount    ;
		//deviceCreateInfo.pQueueCreateInfos       ;
		//deviceCreateInfo.enabledLayerCount       ;
		//deviceCreateInfo.ppEnabledLayerNames     ;
		//deviceCreateInfo.enabledExtensionCount   ;
		//deviceCreateInfo.ppEnabledExtensionNames ;
		//deviceCreateInfo.pEnabledFeatures        ;
		//vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device);
	}

	void VulkanContext::CreateMemoryAllocator()
	{		
		// create the vulkan memory allocator
		//VmaAllocatorCreateInfo allocatorCreateInfo{};
		//allocatorCreateInfo.flags                          = 0u;
		//allocatorCreateInfo.physicalDevice                 = m_PhysicalDevice.m_PhysicalDevice;
		//allocatorCreateInfo.device                         = device;
		//allocatorCreateInfo.preferredLargeHeapBlockSize    = 0u; // defaults to 256 MiB
		//allocatorCreateInfo.pAllocationCallbacks           = VK_NULL_HANDLE;
		//allocatorCreateInfo.pDeviceMemoryCallbacks         = VK_NULL_HANDLE;
		//allocatorCreateInfo.pHeapSizeLimit                 = VK_NULL_HANDLE;
		//allocatorCreateInfo.pVulkanFunctions               = VK_NULL_HANDLE;
		//allocatorCreateInfo.instance                       = s_Instance;
		//allocatorCreateInfo.vulkanApiVersion               = VK_API_VERSION_1_3;
		//allocatorCreateInfo.pTypeExternalMemoryHandleTypes = VK_NULL_HANDLE;
		//VK_CALL(vmaCreateAllocator(&allocatorCreateInfo, &s_Allocator));
	}

	VulkanContext::~VulkanContext()
	{
		if (s_ValidationLayerEnabled)
			GET_INSTANCE_FUNC(vkDestroyDebugUtilsMessengerEXT)(s_Instance, m_DebugMessenger, nullptr);

		vkDestroyInstance(s_Instance, nullptr);
		s_Instance = VK_NULL_HANDLE;
	}

	void VulkanContext::Present()
	{
	}
}