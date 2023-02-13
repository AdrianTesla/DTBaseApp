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
		std::optional<uint32> PresentIndex;
	};

	class VulkanPhysicalDevice
	{
	public:
		void Init(VkPhysicalDevice physicalDevice);
		bool IsExtensionSupported(const char* deviceExtensionName);

		VkPhysicalDevice GetVulkanPhysicalDevice() const { return m_PhysicalDevice; }
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

	class VulkanDevice
	{
	public:
		void Init();
		void Shutdown();

		VkDevice GetVulkanDevice() const { return m_Device; }
	private:
		std::vector<const char*> BuildRequestedDeviceExtensions();
		void BuildEnabledFeatures(VkPhysicalDeviceFeatures* features);
	private:
		VkDevice m_Device = VK_NULL_HANDLE;

		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_TransferQueue = VK_NULL_HANDLE;
		VkQueue m_ComputeQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
	};
}