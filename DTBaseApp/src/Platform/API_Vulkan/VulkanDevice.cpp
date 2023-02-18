#include "VulkanDevice.h"
#include "Platform/PlatformUtils.h"
#include "VulkanContext.h"
#include <set>

namespace DT
{
	void VulkanPhysicalDevice::Init(VkPhysicalDevice physicalDevice)
	{
		ASSERT(physicalDevice != VK_NULL_HANDLE);
		m_PhysicalDevice = physicalDevice;

		GetSupportDetails();
		SelectQueueFamilyIndices();
	}

	void VulkanPhysicalDevice::GetSupportDetails()
	{
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_SupportDetails.Properties);
		vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_SupportDetails.Features);
		
		uint32 deviceExtensionCount = 0u;
		VK_CALL(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &deviceExtensionCount, nullptr));
		m_SupportDetails.Extensions.resize(deviceExtensionCount);
		VK_CALL(vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &deviceExtensionCount, m_SupportDetails.Extensions.data()));

		uint32 queueFamilyPropertyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyPropertyCount, nullptr);
		m_SupportDetails.QueueFamilyProperties.resize(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyPropertyCount, m_SupportDetails.QueueFamilyProperties.data());
	}

	void VulkanPhysicalDevice::SelectQueueFamilyIndices()
	{
		uint32 queueFamilyPropertyCount = (uint32)m_SupportDetails.QueueFamilyProperties.size();

		LOG_INFO("Found {} queue families:", queueFamilyPropertyCount);
		VkSurfaceKHR surface = VulkanContext::Get().GetSurface();
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
			if (!hasGraphics && hasTransfer && hasCompute)
				m_QueueFamilyIndices.ComputeIndex = i;

			// try use the present queue that supports graphics
			if (hasGraphics && (bool)presentSupported)
				m_QueueFamilyIndices.PresentIndex = i;

			// otherwise, if not present use queue that supports (compute + transfer)
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

		std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
		deviceQueueCreateInfos.reserve(uniqueQueueIndices.size());
		for (uint32 queueIndex : uniqueQueueIndices)
		{
			constexpr float defaultQueuePriority = 1.0f;
			
			VkDeviceQueueCreateInfo& deviceQueueCreateInfo = deviceQueueCreateInfos.emplace_back();
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
		deviceCreateInfo.queueCreateInfoCount    = (uint32)deviceQueueCreateInfos.size();
		deviceCreateInfo.pQueueCreateInfos       = deviceQueueCreateInfos.data();
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

		VkCommandPoolCreateInfo commandPoolCreateInfo{};
		commandPoolCreateInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext            = nullptr;
		commandPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = physicalDevice.GetQueueFamilyIndices().GraphicsIndex.value();
		VK_CALL(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_GraphicsCommandPool));
	}

	VkCommandBuffer VulkanDevice::AllocateGraphicsCommandBuffer()
	{
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

		VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
		commandBufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext              = nullptr;
		commandBufferAllocateInfo.commandPool        = m_GraphicsCommandPool;
		commandBufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1u;
		VK_CALL(vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, &commandBuffer));

		return commandBuffer;
	}

	void VulkanDevice::Shutdown()
	{
		vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);
		vkDestroyDevice(m_Device, nullptr);
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
}