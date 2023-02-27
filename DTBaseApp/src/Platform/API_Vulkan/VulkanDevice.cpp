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
		VK_CALL(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_StagingGraphicsCommandPool));
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.TransferIndex.value();
		VK_CALL(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_StagingTransferCommandPool));
		commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.ComputeIndex.value();
		VK_CALL(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_StagingComputeCommandPool));
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

	VkCommandPool VulkanDevice::GetCommandPool(QueueType queueType, bool stagingPool)
	{
		switch (queueType)
		{
			case QueueType::Graphics: return (stagingPool ? m_StagingGraphicsCommandPool : m_GraphicsCommandPool);
			case QueueType::Transfer: return (stagingPool ? m_StagingTransferCommandPool : m_TransferCommandPool);
			case QueueType::Compute:  return (stagingPool ? m_StagingComputeCommandPool : m_ComputeCommandPool);
		}
		ASSERT(false);
		return m_GraphicsCommandPool;
	}

	//VkCommandBuffer VulkanDevice::AllocateCommandBuffer(QueueType queueType, bool stagingPool, VkCommandBufferLevel level)
	//{
	//	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	//	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	//	commandBufferAllocateInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	//	commandBufferAllocateInfo.pNext				 = nullptr;
	//	commandBufferAllocateInfo.commandPool		 = GetCommandPool(queueType, stagingPool);
	//	commandBufferAllocateInfo.level				 = level;
	//	commandBufferAllocateInfo.commandBufferCount = 1u;
	//	VK_CALL(vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, &commandBuffer));

	//	return commandBuffer;
	//}

	void VulkanDevice::Shutdown()
	{
		vkDestroyFence(m_Device, m_WaitFence, nullptr);
		vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_TransferCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_ComputeCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_StagingGraphicsCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_StagingTransferCommandPool, nullptr);
		vkDestroyCommandPool(m_Device, m_StagingComputeCommandPool, nullptr);
		vkDestroyDevice(m_Device, nullptr);

		m_WaitFence = VK_NULL_HANDLE;
		m_GraphicsCommandPool = VK_NULL_HANDLE;
		m_TransferCommandPool = VK_NULL_HANDLE;
		m_ComputeCommandPool = VK_NULL_HANDLE;
		m_StagingGraphicsCommandPool = VK_NULL_HANDLE;
		m_StagingTransferCommandPool = VK_NULL_HANDLE;
		m_StagingComputeCommandPool = VK_NULL_HANDLE;
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