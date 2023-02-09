#include "VulkanContext.h"
#include <GLFW/glfw3.h>

namespace DT
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanErrorCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
	{
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			LOG_TRACE(pCallbackData->pMessage);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			LOG_INFO(pCallbackData->pMessage);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LOG_WARN(pCallbackData->pMessage);
			ASSERT(false);
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			LOG_ERROR(pCallbackData->pMessage);
			ASSERT(false);
		}
		return VK_FALSE;
	}

	void VulkanContext::Init()
	{
		ASSERT(glfwVulkanSupported());
		
		// enumerate available extensions
		uint32 extensionCount;
		VK_CALL(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
		s_AvailableExtensions.resize(extensionCount);
		VK_CALL(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, s_AvailableExtensions.data()));
		
		// enumerate available layers
		uint32 layerCount;
		VK_CALL(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
		s_AvailableLayers.resize(layerCount);
		VK_CALL(vkEnumerateInstanceLayerProperties(&layerCount, s_AvailableLayers.data()));
		
		// requested extensions
		std::vector<const char*> requestedExtensions = BuildRequestedExtensions();
		for (const char* extension : requestedExtensions)
			ASSERT(IsExtensionAvailable(extension));
		
		// requested layers
		std::vector<const char*> requestedLayers = BuildRequestedLayers();
		for (const char* layer : requestedLayers)
			ASSERT(IsLayerAvailable(layer));
		
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

		if (s_ValidationLayerEnabled)
		{
			PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Instance, "vkCreateDebugUtilsMessengerEXT");
			VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
			debugUtilsMessengerCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsMessengerCreateInfo.pNext           = nullptr;
			debugUtilsMessengerCreateInfo.flags           = 0u;
			debugUtilsMessengerCreateInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT	|
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT	|
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT	|
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugUtilsMessengerCreateInfo.messageType     =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    | 
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugUtilsMessengerCreateInfo.pfnUserCallback = VulkanErrorCallback;
			debugUtilsMessengerCreateInfo.pUserData       = nullptr;
			instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugUtilsMessengerCreateInfo;
		}

		VK_CALL(vkCreateInstance(&instanceCreateInfo, nullptr, &s_Instance));
	}

	bool VulkanContext::IsExtensionAvailable(const char* extensionName)
	{
		for (const VkExtensionProperties& extension : s_AvailableExtensions)
		{
			if (strcmp(extension.extensionName, extensionName) == 0)
				return true;
		}
		return false;
	}

	bool VulkanContext::IsLayerAvailable(const char* layerName)
	{
		for (const VkLayerProperties& layer : s_AvailableLayers)
		{
			if (strcmp(layer.layerName, layerName) == 0)
				return true;
		}
		return false;
	}

	std::vector<const char*> VulkanContext::BuildRequestedExtensions()
	{
		std::vector<const char*> extensions;

		uint32 glfwExtensionCount;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (uint32 i = 0u; i < glfwExtensionCount; i++)
			extensions.emplace_back(glfwExtensions[i]);

		if (s_ValidationLayerEnabled)
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	std::vector<const char*> VulkanContext::BuildRequestedLayers()
	{
		std::vector<const char*> layers;

		if (s_ValidationLayerEnabled)
			layers.emplace_back(VK_VALIDATION_LAYER_NAME);

		return layers;
	}

	VulkanContext::~VulkanContext()
	{
		if (s_ValidationLayerEnabled)
		{
			PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Instance, "vkDestroyDebugUtilsMessengerEXT");
			vkDestroyDebugUtilsMessengerEXT(s_Instance, m_DebugMessenger, nullptr);
		}
		vkDestroyInstance(s_Instance, nullptr);
		s_Instance = VK_NULL_HANDLE;
	}

	void VulkanContext::Present()
	{
	}
}