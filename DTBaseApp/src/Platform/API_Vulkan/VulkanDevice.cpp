#include "VulkanDevice.h"
#include "Platform/PlatformUtils.h"
#include "VulkanContext.h"

namespace DT
{
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

		if (VulkanContext::Get().GetAvailablePhysicalDevices().size() > 1u)
			LOG_WARN("Selected GPU: {}", m_PhysicalDeviceProperties.deviceName);

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

	bool VulkanPhysicalDevice::IsExtensionSupported(const char* deviceExtensionName)
	{
		for (VkExtensionProperties& extensionProperty : m_SupportedDeviceExtensions)
		{
			if (strcmp(extensionProperty.extensionName, deviceExtensionName) == 0)
				return true;
		}
		return false;
	}

	void VulkanDevice::Init(VulkanPhysicalDevice& physicalDevice)
	{
		m_PhysicalDevice = &physicalDevice;

		VkDeviceQueueCreateInfo deviceQueueCreateInfos[3];
		for (uint8 i = 0u; i < 3u; i++)
		{
			constexpr float defaultQueuePriority = 1.0f;
			deviceQueueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCreateInfos[i].pNext = nullptr;
			deviceQueueCreateInfos[i].flags = 0u;
			deviceQueueCreateInfos[i].queueCount = 1u;
			deviceQueueCreateInfos[i].pQueuePriorities = &defaultQueuePriority;
		}

		const QueueFamilyIndices& queueFamilyIndices = m_PhysicalDevice->GetQueueFamilyIndices();
		deviceQueueCreateInfos[0].queueFamilyIndex = queueFamilyIndices.GraphicsIndex.value();
		deviceQueueCreateInfos[1].queueFamilyIndex = queueFamilyIndices.TransferIndex.value();
		deviceQueueCreateInfos[2].queueFamilyIndex = queueFamilyIndices.ComputeIndex.value();

		// requested extension and availability check
		std::vector<const char*> deviceExtensions = BuildRequestedDeviceExtensions();
		for (const char* extensionName : deviceExtensions)
		{
			if (!m_PhysicalDevice->IsExtensionSupported(extensionName))
				MessageBoxes::ShowError(std::format("Extension {} is not supported on the selected GPU", extensionName));
		}
		VkPhysicalDeviceFeatures enabledFeatures{};
		BuildEnabledFeatures(&enabledFeatures);
		
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = nullptr;
		deviceCreateInfo.flags = 0u;
		deviceCreateInfo.queueCreateInfoCount = (uint32)std::size(deviceQueueCreateInfos);
		deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
		deviceCreateInfo.enabledLayerCount = 0u;
		deviceCreateInfo.ppEnabledLayerNames = nullptr;
		deviceCreateInfo.enabledExtensionCount = (uint32)deviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
		VK_CALL(vkCreateDevice(m_PhysicalDevice->GetPhysicalDevice(), &deviceCreateInfo, nullptr, &m_Device));
	
		vkGetDeviceQueue(m_Device, queueFamilyIndices.GraphicsIndex.value(), 0u, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, queueFamilyIndices.TransferIndex.value(), 0u, &m_TransferQueue);
		vkGetDeviceQueue(m_Device, queueFamilyIndices.ComputeIndex.value(), 0u, &m_ComputeQueue);
	}

	void VulkanDevice::Shutdown()
	{
		vkDestroyDevice(m_Device, nullptr);
		m_Device = VK_NULL_HANDLE;
		m_PhysicalDevice = nullptr;
	}
	
	std::vector<const char*> VulkanDevice::BuildRequestedDeviceExtensions()
	{
		std::vector<const char*> deviceExtensions;
		//deviceExtensions.emplace_back("VK_KHR_swapchain");
		return deviceExtensions;
	}
	
	void VulkanDevice::BuildEnabledFeatures(VkPhysicalDeviceFeatures* features)
	{
		const VkPhysicalDeviceFeatures& supportedFeatures = m_PhysicalDevice->GetSupportedFeatures();

		ASSERT(supportedFeatures.wideLines);
		features->wideLines = VK_TRUE;
	}
}