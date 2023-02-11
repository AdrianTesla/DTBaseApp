#pragma once
#include "Vulkan.h"
#include <optional>

namespace DT
{
	struct QueueFamilyIndices
	{
		std::optional<uint32> GraphicsIndex;
		std::optional<uint32> TransferIndex;
		std::optional<uint32> ComputeIndex;
	};

	class VulkanPhysicalDevice
	{
	public:
		void Init(VkPhysicalDevice physicalDevice);
		bool IsExtensionSupported(const char* deviceExtensionName);

		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }
		const VkPhysicalDeviceFeatures& GetSupportedFeatures() const { return m_PhysicalDeviceFeatures; }
	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		QueueFamilyIndices m_QueueFamilyIndices;
		VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures{};
		VkPhysicalDeviceProperties m_PhysicalDeviceProperties{};
		std::vector<VkExtensionProperties> m_SupportedDeviceExtensions;
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
	};

	struct VulkanDevice
	{
	public:
		void Init(VulkanPhysicalDevice& physicalDevice);
		void Shutdown();
	private:
		std::vector<const char*> BuildRequestedDeviceExtensions()
		{
			std::vector<const char*> deviceExtensions;

			return deviceExtensions;
		}
		void BuildEnabledFeatures(VkPhysicalDeviceFeatures* features)
		{
			const VkPhysicalDeviceFeatures& supportedFeatures = m_PhysicalDevice->GetSupportedFeatures();

			ASSERT(supportedFeatures.wideLines);
			features->wideLines = VK_TRUE;
		}
	private:
		VulkanPhysicalDevice* m_PhysicalDevice = nullptr;
		VkDevice m_Device = VK_NULL_HANDLE;
	};
}