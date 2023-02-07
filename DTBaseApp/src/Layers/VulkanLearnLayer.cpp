#include "VulkanLearnLayer.h"
#include "Core/Application.h"

#include <vulkan/vulkan.h>

#define VK_CALL(call) { VkResult result = (call); if (result != VK_SUCCESS) { ASSERT(false); } }
static VkInstance s_VulkanInstance = VK_NULL_HANDLE;

namespace DT
{
	void VulkanLearnLayer::OnAttach()
	{
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
		instanceCreateInfo.enabledLayerCount       = 0u;
		instanceCreateInfo.ppEnabledLayerNames     = nullptr;
		instanceCreateInfo.enabledExtensionCount   = 0u;
		instanceCreateInfo.ppEnabledExtensionNames = nullptr;
		VK_CALL(vkCreateInstance(&instanceCreateInfo, nullptr, &s_VulkanInstance));
	}

	void VulkanLearnLayer::OnUpdate(float dt)
	{
	}

	void VulkanLearnLayer::OnEvent(Event& event)
	{
	}

	void VulkanLearnLayer::OnDetach()
	{
		LOG_TRACE("Detached!");
	}
}