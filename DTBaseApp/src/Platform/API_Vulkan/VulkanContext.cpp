#include "VulkanContext.h"
#include <GLFW/glfw3.h>

namespace DT
{
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
		instanceCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext                   = nullptr;
		instanceCreateInfo.flags                   = 0u;
		instanceCreateInfo.pApplicationInfo        = &applicationInfo;
		instanceCreateInfo.enabledLayerCount       = (uint32)requestedLayers.size();
		instanceCreateInfo.ppEnabledLayerNames     = requestedLayers.data();
		instanceCreateInfo.enabledExtensionCount   = (uint32)requestedExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = requestedExtensions.data();
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

		// extensions required by GLFW
		uint32 glfwExtensionCount;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		for (size_t i = 0u; i < glfwExtensionCount; i++)
			extensions.emplace_back(glfwExtensions[i]);

		return extensions;
	}

	std::vector<const char*> VulkanContext::BuildRequestedLayers()
	{
		std::vector<const char*> layers;

		return layers;
	}

	VulkanContext::~VulkanContext()
	{
		vkDestroyInstance(s_Instance, nullptr);
		s_Instance = VK_NULL_HANDLE;
	}

	void VulkanContext::Present()
	{
	}
}